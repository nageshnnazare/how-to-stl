#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <numeric>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Example 1: Basic Thread Pool
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

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Thread Pool Examples                  ║\n";
    std::cout << "║         Task-based Parallelism                 ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_basic();
    example_parallel_computation();
    example_parallel_for();
    example_fire_and_forget();
    
    std::cout << "\n✅ All thread pool examples completed successfully!\n";
    return 0;
}

