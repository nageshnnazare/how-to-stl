// ============================================================================
//  thread_pool_example.cpp — producer/consumer task scheduling
// ============================================================================
//
// Demonstrates:
//   1. Basic submit()     — futures with sleep and arithmetic
//   2. Parallel sum       — manual chunking across workers
//   3. ParallelFor        — helper splitting [0, N) index range
//   4. Fire-and-forget    — execute() + wait() for quiescence
//   5. Pool statistics    — queued/active/completed counters
//
// Build (from repo root):
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. \
//       thread_pool/thread_pool_example.cpp -o /tmp/x_thread_pool
// ============================================================================

#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <numeric>
#include <atomic>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Example 1: submit() returns futures; workers wake on condition_variable
void example_basic() {
    print_divider("Basic Thread Pool");
    
    ThreadPool pool(4);
    
    std::cout << "Thread pool created with " << pool.num_workers() << " workers\n";
    
    // Submit tasks
    auto future1 = pool.submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 42;
    });
    
    auto future2 = pool.submit([](int x, int y) {
        return x + y;
    }, 10, 20);
    
    std::cout << "Result 1: " << future1.get() << "\n";
    std::cout << "Result 2: " << future2.get() << "\n";
}

// Example 2: Parallel Computation
void example_parallel_computation() {
    print_divider("Parallel Computation");
    
    ThreadPool pool(std::thread::hardware_concurrency());
    
    const int N = 1000000;
    std::vector<int> numbers(N);
    std::iota(numbers.begin(), numbers.end(), 1);
    
    std::cout << "Computing sum of " << N << " numbers in parallel...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    size_t chunk_size = N / pool.num_workers();
    std::vector<std::future<long long>> futures;
    
    for (size_t i = 0; i < pool.num_workers(); ++i) {
        size_t start_idx = i * chunk_size;
        size_t end_idx = (i == pool.num_workers() - 1) ? N : (i + 1) * chunk_size;
        
        futures.push_back(pool.submit([&numbers, start_idx, end_idx]() {
            long long sum = 0;
            for (size_t j = start_idx; j < end_idx; ++j) {
                sum += numbers[j];
            }
            return sum;
        }));
    }
    
    long long total = 0;
    for (auto& future : futures) {
        total += future.get();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sum: " << total << "\n";
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Completed tasks: " << pool.completed_tasks() << "\n";
}

// Example 3: Parallel For
void example_parallel_for() {
    print_divider("Parallel For Loop");
    
    ThreadPool pool(8);
    ParallelFor pfor(pool);
    
    std::vector<int> data(1000);
    
    std::cout << "Processing 1000 elements in parallel...\n";
    
    pfor.execute(0, data.size(), [&data](size_t i) {
        data[i] = i * i;
    });
    
    std::cout << "First 10 results: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << "\n";
}

// Example 4: Fire and Forget
void example_fire_and_forget() {
    print_divider("Fire and Forget Tasks");
    
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 100; ++i) {
        pool.execute([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    pool.wait();
    
    std::cout << "Counter: " << counter.load() << "\n";
    std::cout << "Completed: " << pool.completed_tasks() << "\n";
}

// Example 5: Statistics — observe queue depth and completion counter
void example_statistics() {
    print_divider("Pool Statistics");

    ThreadPool pool(2);

    for (int i = 0; i < 5; ++i) {
        pool.execute([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            (void)i;
        });
    }

    std::cout << "Workers: " << pool.num_workers() << "\n";
    std::cout << "Queued (snapshot): " << pool.queued_tasks() << "\n";
    std::cout << "Active: " << pool.active_workers() << "\n";

    pool.wait();

    std::cout << "After wait — completed: " << pool.completed_tasks() << "\n";
    std::cout << "Stopped: " << (pool.is_stopped() ? "yes" : "no") << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Thread Pool Examples                  ║\n";
    std::cout << "║         Task-based Parallelism                 ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_basic();
    example_parallel_computation();
    example_parallel_for();
    example_fire_and_forget();
    example_statistics();
    
    std::cout << "\n✅ All thread pool examples completed successfully!\n";
    return 0;
}


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: with threads, line ordering / counts may vary between runs.
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║          Thread Pool Examples                  ║
║         Task-based Parallelism                 ║
╚════════════════════════════════════════════════╝

=== Basic Thread Pool ===
Thread pool created with 4 workers
Result 1: 42
Result 2: 30

=== Parallel Computation ===
Computing sum of 1000000 numbers in parallel...
Sum: 500000500000
Time: 0 ms
Completed tasks: 10

=== Parallel For Loop ===
Processing 1000 elements in parallel...
First 10 results: 0 1 4 9 16 25 36 49 64 81 

=== Fire and Forget Tasks ===
Counter: 100
Completed: 100

=== Pool Statistics ===
Workers: 2
Queued (snapshot): 3
Active: 2
After wait — completed: 5
Stopped: no

✅ All thread pool examples completed successfully!
 * ============================================================================ */
