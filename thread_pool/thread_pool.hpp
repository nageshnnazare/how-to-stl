#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>

/**
 * @brief Thread Pool - Task-based Parallelism
 * 
 * A production-quality thread pool implementation with:
 * - Worker threads that process tasks from a queue
 * - Future-based result retrieval
 * - Graceful shutdown
 * - Exception handling
 * - Work stealing (optional)
 * 
 * Key Features:
 * - Submit tasks and get futures
 * - Automatic thread management
 * - Thread-safe task queue
 * - Configurable number of workers
 * - Statistics tracking
 */

class ThreadPool {
private:
    // Worker threads
    std::vector<std::thread> workers_;
    
    // Task queue
    std::queue<std::function<void()>> tasks_;
    
    // Synchronization
    std::mutex mutex_;
    std::condition_variable condition_;
    
    // State
    std::atomic<bool> stop_;
    std::atomic<size_t> active_workers_;
    std::atomic<size_t> completed_tasks_;
    
    /**
     * Worker thread function
     */
    void worker_thread() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(mutex_);
                
                // Wait for task or stop signal
                condition_.wait(lock, [this] {
                    return stop_.load() || !tasks_.empty();
                });
                
                // Exit if stopping and no tasks left
                if (stop_.load() && tasks_.empty()) {
                    return;
                }
                
                // Get next task
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }
            
            // Execute task outside lock
            if (task) {
                active_workers_.fetch_add(1, std::memory_order_relaxed);
                
                try {
                    task();
                } catch (...) {
                    // Swallow exceptions to keep worker alive
                }
                
                active_workers_.fetch_sub(1, std::memory_order_relaxed);
                completed_tasks_.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }
    
public:
    /**
     * Create thread pool with specified number of workers
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
     * Destructor - waits for all tasks to complete
     */
    ~ThreadPool() {
        shutdown();
    }
    
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    /**
     * Submit a task and get a future for the result
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        
        // Create packaged task
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
     * Submit a task without return value
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
     * Wait for all queued tasks to complete
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
     * Shutdown thread pool
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
    
    /**
     * Get number of worker threads
     */
    size_t num_workers() const {
        return workers_.size();
    }
    
    /**
     * Get number of queued tasks
     */
    size_t queued_tasks() const {
        std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return tasks_.size();
    }
    
    /**
     * Get number of active workers
     */
    size_t active_workers() const {
        return active_workers_.load(std::memory_order_relaxed);
    }
    
    /**
     * Get total completed tasks
     */
    size_t completed_tasks() const {
        return completed_tasks_.load(std::memory_order_relaxed);
    }
    
    /**
     * Check if stopped
     */
    bool is_stopped() const {
        return stop_.load();
    }
};

// ============================================================================
// PARALLEL FOR - Parallel loop execution
// ============================================================================
class ParallelFor {
private:
    ThreadPool& pool_;
    
public:
    explicit ParallelFor(ThreadPool& pool) : pool_(pool) {}
    
    /**
     * Execute loop in parallel
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
        
        // Wait for all chunks
        for (auto& future : futures) {
            future.wait();
        }
    }
};

// ============================================================================
// TASK QUEUE - Priority task queue
// ============================================================================
template<typename T>
class TaskQueue {
private:
    struct Task {
        T value;
        int priority;
        
        bool operator<(const Task& other) const {
            return priority < other.priority;  // Higher priority first
        }
    };
    
    std::priority_queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
    
public:
    TaskQueue() : stop_(false) {}
    
    void push(T value, int priority = 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push({std::move(value), priority});
        cv_.notify_one();
    }
    
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
// WORK STEALING THREAD POOL - Advanced version
// ============================================================================
class WorkStealingThreadPool {
private:
    struct Worker {
        std::queue<std::function<void()>> tasks;
        std::mutex mutex;
        std::thread thread;
    };
    
    std::vector<std::unique_ptr<Worker>> workers_;
    std::atomic<bool> stop_;
    std::atomic<size_t> next_worker_;
    
    void worker_thread(size_t worker_id) {
        Worker& worker = *workers_[worker_id];
        
        while (!stop_.load()) {
            std::function<void()> task;
            
            // Try to get task from own queue
            {
                std::unique_lock<std::mutex> lock(worker.mutex);
                if (!worker.tasks.empty()) {
                    task = std::move(worker.tasks.front());
                    worker.tasks.pop();
                }
            }
            
            // If no task, try to steal from others
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
    
    ~WorkStealingThreadPool() {
        stop_.store(true);
        
        for (auto& worker : workers_) {
            if (worker->thread.joinable()) {
                worker->thread.join();
            }
        }
    }
    
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

