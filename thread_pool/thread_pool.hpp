#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>               // for worker thread storage
#include <queue>                // for FIFO task queue
#include <thread>               // for std::thread workers
#include <mutex>                // for queue protection
#include <condition_variable>   // for worker sleep/wake
#include <functional>           // for std::function tasks
#include <future>               // for submit() return type
#include <atomic>               // for stop_, active_workers_, counters
#include <memory>               // for shared_ptr packaged_task wrapper

// ============================================================================
//  ThreadPool — fixed worker threads + shared FIFO task queue
// ============================================================================
//
// WHAT IT IS
// ----------
// N persistent worker threads block on a condition_variable until tasks arrive.
// Producers call submit() or execute() to enqueue std::function<void()> work;
// workers pop tasks, run them outside the queue mutex, and track statistics.
// submit() wraps callables in std::packaged_task and returns std::future results.
//
// THE SEVEN FIELDS
// ----------------
//
//     workers_          -> std::thread objects (one per worker)
//     tasks_            -> FIFO queue of std::function<void()> work items
//     mutex_            -> protects tasks_ and coordinates with cv_
//     condition_        -> workers sleep until stop_ || !tasks_.empty()
//     stop_             -> graceful shutdown flag (atomic)
//     active_workers_   -> tasks currently executing (outside mutex)
//     completed_tasks_  -> total finished tasks since construction
//
// PRODUCER / CONSUMER FLOW
// ------------------------
//
//   Producer thread                Shared queue                 Worker thread
//        │                              │                            │
//        ├─ lock(mutex_)                 │                            │
//        ├─ tasks_.push(task)           │                            │
//        ├─ unlock; notify_one() ────────┼── wake ───────────────────▶│
//        │                              │                     lock(mutex_)
//        │                              │◀── pop front ──────────────┤
//        │                              │                     unlock
//        │                              │                     run task()
//        │                              │                     active_workers_++
//        │                              │                     completed_tasks_++
//
// WHY run tasks OUTSIDE the mutex?
// --------------------------------
// Holding mutex_ during task execution would serialize all work and deadlock
// if a task calls submit() on the same pool. Pop under lock, execute after unlock.
//
// SHUTDOWN
// --------
//     stop_ = true; notify_all()
//     each worker: wake → if stop && queue empty → return from thread
//     destructor joins all workers (waits for in-flight tasks)
// ============================================================================
class ThreadPool {
private:
    std::vector<std::thread> workers_;              // OS threads running worker_thread()
    std::queue<std::function<void()>> tasks_;       // FIFO pending work (mutex-protected)
    std::mutex mutex_;                              // guards tasks_ + predicate waits
    std::condition_variable condition_;             // park workers until work or stop
    std::atomic<bool> stop_;                        // true → workers exit when queue drains
    std::atomic<size_t> active_workers_;            // tasks executing right now
    std::atomic<size_t> completed_tasks_;           // lifetime task completion counter

    /**
     * @brief Main loop for each worker thread.
     *
     *     loop forever:
     *         lock
     *         wait until stop_ || !tasks_.empty()
     *         if stop_ && tasks_.empty(): return   // graceful exit
     *         pop task into local std::function
     *         unlock
     *         if task: active_workers_++; run(); completed_tasks_++
     *
     *     ┌──────── wait on cv ────────┐
     *     │  stop? && empty? → exit   │
     *     │  else pop task             │
     *     └──────── execute task ──────┘
     */
    void worker_thread() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);

                condition_.wait(lock, [this] {
                    return stop_.load() || !tasks_.empty();
                });

                if (stop_.load() && tasks_.empty()) {
                    return;
                }

                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }

            if (task) {
                active_workers_.fetch_add(1, std::memory_order_relaxed);

                try {
                    task();
                } catch (...) {
                    // keep worker alive; packaged_task stores exception in future
                }

                active_workers_.fetch_sub(1, std::memory_order_relaxed);
                completed_tasks_.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

public:
    /**
     * @brief Spawn num_threads workers (default: hardware_concurrency()).
     *
     *     for i in 0..num_threads:
     *         workers_.emplace_back(worker_thread)
     */
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false)
        , active_workers_(0)
        , completed_tasks_(0)
    {
        workers_.reserve(num_threads);

        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_thread(); });
        }
    }

    /**
     * @brief Shutdown: drain policy via shutdown(), join all workers.
     */
    ~ThreadPool() {
        shutdown();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Enqueue callable; return std::future of its return value.
     *
     * Steps:
     *     (1) packaged_task = bind(f, args...)
     *     (2) future = task.get_future()
     *     (3) lock; if stop_ throw; push lambda that runs (*task)()
     *     (4) notify_one worker
     *     (5) return future to caller
     *
     *     caller ──▶ [queue] ──▶ worker runs task ──▶ future.set_value/exception
     *
     * shared_ptr keeps packaged_task alive until worker executes the lambda.
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (stop_.load()) {
                throw std::runtime_error("ThreadPool is stopped");
            }

            tasks_.emplace([task]() { (*task)(); });
        }

        condition_.notify_one();
        return result;
    }

    /**
     * @brief Fire-and-forget enqueue (no future).
     *
     * Same queue path as submit() but caller does not wait for a result.
     */
    template<typename F>
    void execute(F&& f) {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (stop_.load()) {
                throw std::runtime_error("ThreadPool is stopped");
            }

            tasks_.emplace(std::forward<F>(f));
        }

        condition_.notify_one();
    }

    /**
     * @brief Spin-yield until queue empty AND no active workers.
     *
     *     loop:
     *         lock; if tasks_.empty() && active_workers_==0: break
     *         unlock; yield
     *
     * Does not prevent new submit() from other threads unless externally stopped.
     */
    void wait() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (tasks_.empty() && active_workers_.load() == 0) {
                break;
            }
            lock.unlock();
            std::this_thread::yield();
        }
    }

    /**
     * @brief Graceful shutdown: stop flag, wake all, join workers.
     *
     *     stop_ = true
     *     notify_all()  ──▶ each worker eventually sees stop && empty → exit
     *     join every thread in workers_
     */
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_.store(true);
        }

        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        workers_.clear();
    }

    /** @brief Number of worker threads (0 after shutdown). */
    size_t num_workers() const {
        return workers_.size();
    }

    /** @brief Tasks waiting in FIFO queue (snapshot under mutex). */
    size_t queued_tasks() const {
        std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return tasks_.size();
    }

    /** @brief Workers currently executing a popped task. */
    size_t active_workers() const {
        return active_workers_.load(std::memory_order_relaxed);
    }

    /** @brief Total tasks finished since pool creation. */
    size_t completed_tasks() const {
        return completed_tasks_.load(std::memory_order_relaxed);
    }

    /** @brief Whether shutdown() has been requested. */
    bool is_stopped() const {
        return stop_.load();
    }
};

// ============================================================================
//  ParallelFor — split [start, end) across pool workers
// ============================================================================
//
// WHAT IT IS
// ----------
// Splits an index range into ~num_workers chunks, submit()s one task per chunk,
// waits on all futures. Convenience wrapper over ThreadPool::submit.
//
// CHUNKING (N=1000, 4 workers → chunk_size=250)
// -----------------------------------------------
//
//     W0: [0,250)   W1: [250,500)   W2: [500,750)   W3: [750,1000)
// ============================================================================
class ParallelFor {
private:
    ThreadPool& pool_;      // non-owning reference to shared worker pool

public:
    explicit ParallelFor(ThreadPool& pool) : pool_(pool) {}

    /**
     * @brief Invoke func(i) for each i in [start, end) in parallel.
     *
     *     chunk_size = ceil((end-start) / num_workers)
     *     for each worker i: submit loop over [chunk_start, chunk_end)
     *     wait on all futures
     */
    template<typename F>
    void execute(size_t start, size_t end, F&& func) {
        size_t num_workers = pool_.num_workers();
        size_t range = end - start;
        size_t chunk_size = (range + num_workers - 1) / num_workers;

        std::vector<std::future<void>> futures;

        for (size_t i = 0; i < num_workers; ++i) {
            size_t chunk_start = start + i * chunk_size;
            size_t chunk_end = std::min(chunk_start + chunk_size, end);

            if (chunk_start >= end) break;

            futures.push_back(pool_.submit([chunk_start, chunk_end, &func]() {
                for (size_t j = chunk_start; j < chunk_end; ++j) {
                    func(j);
                }
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }
};

// ============================================================================
//  TaskQueue<T> — mutex + priority_queue + condition_variable
// ============================================================================
//
// WHAT IT IS
// ----------
// Thread-safe priority queue: higher `priority` int pops first. Used to teach
// ordered task scheduling separate from the FIFO ThreadPool queue.
//
// THE FOUR FIELDS
// ---------------
//
//     queue_  -> std::priority_queue<Task{value, priority}>
//     mutex_  -> protects queue_
//     cv_     -> wait until stop_ || !queue_.empty()
//     stop_   -> wake waiters so pop() can return false
// ============================================================================
template<typename T>
class TaskQueue {
private:
    struct Task {
        T value;
        int priority;

        bool operator<(const Task& other) const {
            return priority < other.priority;  // higher priority first
        }
    };

    std::priority_queue<Task> queue_;   // max-heap by priority
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;

public:
    TaskQueue() : stop_(false) {}

    /**
     * @brief Push value with priority; notify one waiter.
     */
    void push(T value, int priority = 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push({std::move(value), priority});
        cv_.notify_one();
    }

    /**
     * @brief Block until item available or stopped; return false on stop+empty.
     */
    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });

        if (stop_ && queue_.empty()) {
            return false;
        }

        value = std::move(queue_.top().value);
        queue_.pop();
        return true;
    }

    /** @brief Set stop flag and wake all blocked pop() calls. */
    void stop() {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
        cv_.notify_all();
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

// ============================================================================
//  WorkStealingThreadPool — per-worker queues + steal from neighbors
// ============================================================================
//
// WHAT IT IS
// ----------
// Each worker owns a private FIFO queue. submit() round-robins new tasks onto
// worker queues. Idle workers try_to_lock neighbors and steal from the front of
// another queue — reduces contention vs one global mutex queue.
//
// STRUCTURE
// ---------
//
//   Worker 0          Worker 1          Worker 2
//   [tasks]           [tasks]           [tasks]
//      │                 │                 │
//      └──── steal if empty ────────────────┘
//
// THE THREE TOP-LEVEL FIELDS
// --------------------------
//
//     workers_     -> vector<unique_ptr<Worker{queue, mutex, thread}>>
//     stop_        -> atomic shutdown flag
//     next_worker_ -> round-robin index for submit()
//
// WORKER LOOP
// -----------
//     (1) try pop own queue (lock own mutex)
//     (2) if empty, try_to_lock other workers' mutexes, steal front task
//     (3) if task: run(); else yield()
// ============================================================================
class WorkStealingThreadPool {
private:
    struct Worker {
        std::queue<std::function<void()>> tasks;  // private FIFO for this worker
        std::mutex mutex;                          // protects this worker's tasks
        std::thread thread;                        // runs worker_thread(id)
    };

    std::vector<std::unique_ptr<Worker>> workers_;  // one Worker per OS thread
    std::atomic<bool> stop_;                        // set true in destructor
    std::atomic<size_t> next_worker_;               // round-robin submit target

    /**
     * @brief Per-worker loop: pop local queue, else steal, else yield.
     *
     *     while !stop_:
     *         try own queue
     *         if empty: for each other worker: try_lock + steal front
     *         if task: run()
     *         else: yield()
     */
    void worker_thread(size_t worker_id) {
        Worker& worker = *workers_[worker_id];

        while (!stop_.load()) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(worker.mutex);
                if (!worker.tasks.empty()) {
                    task = std::move(worker.tasks.front());
                    worker.tasks.pop();
                }
            }

            if (!task) {
                for (size_t i = 0; i < workers_.size(); ++i) {
                    if (i == worker_id) continue;

                    std::unique_lock<std::mutex> lock(workers_[i]->mutex, std::try_to_lock);
                    if (lock.owns_lock() && !workers_[i]->tasks.empty()) {
                        task = std::move(workers_[i]->tasks.front());
                        workers_[i]->tasks.pop();
                        break;
                    }
                }
            }

            if (task) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    /**
     * @brief Create workers and start threads (no global task queue).
     */
    explicit WorkStealingThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false)
        , next_worker_(0)
    {
        workers_.reserve(num_threads);

        for (size_t i = 0; i < num_threads; ++i) {
            workers_.push_back(std::make_unique<Worker>());
        }

        for (size_t i = 0; i < num_threads; ++i) {
            workers_[i]->thread = std::thread([this, i] { worker_thread(i); });
        }
    }

    /**
     * @brief Set stop_ and join workers (does not drain pending tasks).
     */
    ~WorkStealingThreadPool() {
        stop_.store(true);

        for (auto& worker : workers_) {
            if (worker->thread.joinable()) {
                worker->thread.join();
            }
        }
    }

    /**
     * @brief Round-robin enqueue onto worker queues.
     *
     *     id = next_worker_.fetch_add(1) % num_workers
     *     lock(workers_[id]); push task
     */
    template<typename F>
    void submit(F&& f) {
        size_t worker_id = next_worker_.fetch_add(1, std::memory_order_relaxed) % workers_.size();

        std::unique_lock<std::mutex> lock(workers_[worker_id]->mutex);
        workers_[worker_id]->tasks.emplace(std::forward<F>(f));
    }

    size_t num_workers() const {
        return workers_.size();
    }
};

#endif // THREAD_POOL_HPP
