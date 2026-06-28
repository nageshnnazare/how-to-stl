#include "thread_pool/thread_pool.hpp"
#include <iostream>
#include <cassert>
#include <numeric>
#include <chrono>

int g_tests_passed = 0;
int g_tests_failed = 0;

void test(const char* name, bool condition) {
    if (condition) {
        std::cout << "✓ " << name << "\n";
        g_tests_passed++;
    } else {
        std::cout << "✗ " << name << "\n";
        g_tests_failed++;
    }
}

// Test 1: Basic Thread Pool Creation
void test_creation() {
    ThreadPool pool(4);
    test("Thread pool created", pool.num_workers() == 4);
    test("Thread pool not stopped initially", !pool.is_stopped());
    test("No active workers initially", pool.active_workers() == 0);
    test("No queued tasks initially", pool.queued_tasks() == 0);
}

// Test 2: Submit Task with Return Value
void test_submit_with_return() {
    ThreadPool pool(2);
    
    auto future = pool.submit([]() { return 42; });
    int result = future.get();
    
    test("Submit returns correct value", result == 42);
}

// Test 3: Submit Task with Parameters
void test_submit_with_params() {
    ThreadPool pool(2);
    
    auto future = pool.submit([](int a, int b) { return a + b; }, 10, 20);
    int result = future.get();
    
    test("Submit with params returns correct value", result == 30);
}

// Test 4: Multiple Tasks
void test_multiple_tasks() {
    ThreadPool pool(4);
    
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool.submit([i]() { return i * 2; }));
    }
    
    bool all_correct = true;
    for (int i = 0; i < 100; ++i) {
        if (futures[i].get() != i * 2) {
            all_correct = false;
            break;
        }
    }
    
    test("Multiple tasks all execute correctly", all_correct);
    test("Completed tasks counter", pool.completed_tasks() >= 100);
}

// Test 5: Execute (Fire and Forget)
void test_execute() {
    ThreadPool pool(2);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 50; ++i) {
        pool.execute([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    pool.wait();
    
    test("Execute fire-and-forget", counter.load() == 50);
}

// Test 6: Wait for Completion
void test_wait() {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 100; ++i) {
        pool.execute([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    pool.wait();
    
    test("Wait completes all tasks", counter.load() == 100);
    test("No queued tasks after wait", pool.queued_tasks() == 0);
    test("No active workers after wait", pool.active_workers() == 0);
}

// Test 7: Parallel Computation
void test_parallel_computation() {
    ThreadPool pool(4);
    
    const int N = 10000;
    std::vector<int> numbers(N);
    std::iota(numbers.begin(), numbers.end(), 1);
    
    size_t chunk_size = N / pool.num_workers();
    std::vector<std::future<long long>> futures;
    
    for (size_t i = 0; i < pool.num_workers(); ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == pool.num_workers() - 1) ? N : (i + 1) * chunk_size;
        
        futures.push_back(pool.submit([&numbers, start, end]() {
            long long sum = 0;
            for (size_t j = start; j < end; ++j) {
                sum += numbers[j];
            }
            return sum;
        }));
    }
    
    long long total = 0;
    for (auto& future : futures) {
        total += future.get();
    }
    
    long long expected = (long long)N * (N + 1) / 2;
    test("Parallel computation correct", total == expected);
}

// Test 8: Parallel For
void test_parallel_for() {
    ThreadPool pool(4);
    ParallelFor pfor(pool);
    
    std::vector<int> data(1000);
    
    pfor.execute(0, data.size(), [&data](size_t i) {
        data[i] = i * i;
    });
    
    bool all_correct = true;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] != (int)(i * i)) {
            all_correct = false;
            break;
        }
    }
    
    test("Parallel for executes correctly", all_correct);
}

// Test 9: Exception Handling
void test_exception_handling() {
    ThreadPool pool(2);
    
    // Submit task that throws
    auto future = pool.submit([]() -> int {
        throw std::runtime_error("Test exception");
        return 0;
    });
    
    bool exception_caught = false;
    try {
        future.get();
    } catch (const std::runtime_error& e) {
        exception_caught = true;
    }
    
    test("Exception propagated through future", exception_caught);
    
    // Pool should still work after exception
    auto future2 = pool.submit([]() { return 100; });
    test("Pool still works after exception", future2.get() == 100);
}

// Test 10: Task Queue Statistics
void test_statistics() {
    ThreadPool pool(1);  // Single worker
    
    // Submit slow tasks
    for (int i = 0; i < 10; ++i) {
        pool.execute([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    test("Queued tasks tracked", pool.queued_tasks() > 0 || pool.active_workers() > 0);
    
    pool.wait();
    test("Completed tasks incremented", pool.completed_tasks() >= 10);
}

// Test 11: Shutdown
void test_shutdown() {
    ThreadPool pool(4);
    
    for (int i = 0; i < 50; ++i) {
        pool.execute([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
    }
    
    pool.shutdown();
    
    test("Pool stopped after shutdown", pool.is_stopped());
    test("No workers after shutdown", pool.num_workers() == 0);
    
    // Submitting after shutdown should throw
    bool throws_exception = false;
    try {
        pool.submit([]() { return 1; });
    } catch (const std::runtime_error&) {
        throws_exception = true;
    }
    
    test("Submit throws after shutdown", throws_exception);
}

// Test 12: Hardware Concurrency Default
void test_default_construction() {
    ThreadPool pool;
    test("Default uses hardware concurrency", 
         pool.num_workers() == std::thread::hardware_concurrency());
}

// Test 13: Concurrent Task Submission
void test_concurrent_submission() {
    ThreadPool pool(4);
    std::atomic<int> submitted{0};
    std::atomic<int> executed{0};
    
    std::vector<std::thread> submitters;
    for (int i = 0; i < 4; ++i) {
        submitters.emplace_back([&]() {
            for (int j = 0; j < 25; ++j) {
                pool.execute([&executed]() {
                    executed.fetch_add(1, std::memory_order_relaxed);
                });
                submitted.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : submitters) t.join();
    pool.wait();
    
    test("Concurrent submission all tasks submitted", submitted.load() == 100);
    test("Concurrent submission all tasks executed", executed.load() == 100);
}

// Test 14: Task Queue (Priority)
void test_task_queue() {
    TaskQueue<int> queue;
    
    queue.push(10, 1);
    queue.push(20, 3);
    queue.push(30, 2);
    
    int value;
    queue.pop(value);
    test("Task queue highest priority first", value == 20);
    
    queue.pop(value);
    test("Task queue second priority", value == 30);
    
    queue.pop(value);
    test("Task queue lowest priority", value == 10);
}

// Test 15: Work Stealing Thread Pool
void test_work_stealing() {
    WorkStealingThreadPool pool(4);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 100; ++i) {
        pool.submit([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    test("Work stealing pool executes tasks", counter.load() == 100);
    test("Work stealing pool has workers", pool.num_workers() == 4);
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Thread Pool Test Suite               ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    test_creation();
    test_submit_with_return();
    test_submit_with_params();
    test_multiple_tasks();
    test_execute();
    test_wait();
    test_parallel_computation();
    test_parallel_for();
    test_exception_handling();
    test_statistics();
    test_shutdown();
    test_default_construction();
    test_concurrent_submission();
    test_task_queue();
    test_work_stealing();
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Tests passed: " << g_tests_passed << "\n";
    std::cout << "Tests failed: " << g_tests_failed << "\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return g_tests_failed == 0 ? 0 : 1;
}


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: with threads, line ordering / counts may vary between runs.
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║          Thread Pool Test Suite               ║
╚════════════════════════════════════════════════╝

✓ Thread pool created
✓ Thread pool not stopped initially
✓ No active workers initially
✓ No queued tasks initially
✓ Submit returns correct value
✓ Submit with params returns correct value
✓ Multiple tasks all execute correctly
✓ Completed tasks counter
✓ Execute fire-and-forget
✓ Wait completes all tasks
✓ No queued tasks after wait
✓ No active workers after wait
✓ Parallel computation correct
✓ Parallel for executes correctly
✓ Exception propagated through future
✓ Pool still works after exception
✓ Queued tasks tracked
✓ Completed tasks incremented
✓ Pool stopped after shutdown
✓ No workers after shutdown
✓ Submit throws after shutdown
✓ Default uses hardware concurrency
✓ Concurrent submission all tasks submitted
✓ Concurrent submission all tasks executed
✓ Task queue highest priority first
✓ Task queue second priority
✓ Task queue lowest priority
✓ Work stealing pool executes tasks
✓ Work stealing pool has workers

════════════════════════════════════════════════
Tests passed: 29
Tests failed: 0
════════════════════════════════════════════════
 * ============================================================================ */
