#include "arena_allocator.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Example 1: Basic Arena Usage
void example_basic_arena() {
    print_divider("Basic Thread Arena Usage");
    
    ThreadArenaAllocator allocator(1024 * 64);  // 64 KB per thread
    
    std::cout << "Allocating from main thread...\n";
    
    int* numbers = static_cast<int*>(allocator.allocate(10 * sizeof(int)));
    if (numbers) {
        for (int i = 0; i < 10; ++i) {
            numbers[i] = i * 10;
        }
        
        std::cout << "Numbers: ";
        for (int i = 0; i < 10; ++i) {
            std::cout << numbers[i] << " ";
        }
        std::cout << "\n";
    }
    
    auto stats = allocator.get_thread_stats();
    std::cout << "Thread stats:\n";
    std::cout << "  Used: " << stats.used << " bytes\n";
    std::cout << "  Available: " << stats.available << " bytes\n";
    std::cout << "  Capacity: " << stats.capacity << " bytes\n";
}

// Example 2: Multi-threaded Allocation
void example_multithreaded() {
    print_divider("Multi-threaded Arena Allocation");
    
    ThreadArenaAllocator allocator(1024 * 128);  // 128 KB per thread
    const int NUM_THREADS = 4;
    const int ALLOCATIONS_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> total_allocations{0};
    
    auto worker = [&](int thread_id) {
        std::cout << "Thread " << thread_id << " starting...\n";
        
        for (int i = 0; i < ALLOCATIONS_PER_THREAD; ++i) {
            void* ptr = allocator.allocate(128);  // Allocate 128 bytes
            if (ptr) {
                total_allocations.fetch_add(1, std::memory_order_relaxed);
            }
        }
        
        auto stats = allocator.get_thread_stats();
        std::cout << "Thread " << thread_id << " stats:\n";
        std::cout << "  Used: " << stats.used << " bytes\n";
        std::cout << "  Utilization: " << std::fixed << std::setprecision(1)
                  << (100.0 * stats.used / stats.capacity) << "%\n";
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    auto global_stats = allocator.get_global_stats();
    std::cout << "\nGlobal stats:\n";
    std::cout << "  Number of arenas: " << global_stats.num_arenas << "\n";
    std::cout << "  Total capacity: " << global_stats.total_capacity << " bytes\n";
    std::cout << "  Total used: " << global_stats.total_used << " bytes\n";
    std::cout << "  Total allocations: " << total_allocations.load() << "\n";
    std::cout << "  Time: " << duration.count() << " μs\n";
}

// Example 3: Scoped Arena (RAII)
void example_scoped_arena() {
    print_divider("Scoped Arena (RAII)");
    
    ThreadArenaAllocator allocator(1024 * 32);
    
    std::cout << "Before scope:\n";
    auto stats1 = allocator.get_thread_stats();
    std::cout << "  Used: " << stats1.used << " bytes\n";
    
    {
        ScopedArena scope(allocator);
        
        // Allocate within scope
        int* data = static_cast<int*>(scope.allocate(100 * sizeof(int)));
        if (data) {
            for (int i = 0; i < 100; ++i) {
                data[i] = i;
            }
            std::cout << "Allocated 100 integers\n";
        }
        
        auto stats2 = allocator.get_thread_stats();
        std::cout << "Within scope:\n";
        std::cout << "  Used: " << stats2.used << " bytes\n";
        
    }  // Scope ends - arena resets!
    
    std::cout << "After scope (arena reset):\n";
    auto stats3 = allocator.get_thread_stats();
    std::cout << "  Used: " << stats3.used << " bytes\n";
}

// Example 4: Arena Vector
void example_arena_vector() {
    print_divider("Arena Vector");
    
    ThreadArenaAllocator allocator(1024 * 64);
    
    ArenaVector<int> vec(allocator);
    
    std::cout << "Pushing elements...\n";
    for (int i = 0; i < 20; ++i) {
        vec.push_back(i * 2);
    }
    
    std::cout << "Vector contents (" << vec.size() << " elements): ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << "\n";
    
    std::cout << "Capacity: " << vec.capacity() << "\n";
    
    auto stats = allocator.get_thread_stats();
    std::cout << "Arena used: " << stats.used << " bytes\n";
}

// Example 5: Performance Comparison
void example_performance() {
    print_divider("Performance Comparison");
    
    const int NUM_THREADS = 8;
    const int ALLOCATIONS = 10000;
    
    // Test with Arena Allocator
    {
        ThreadArenaAllocator allocator(1024 * 1024);  // 1 MB per thread
        std::vector<std::thread> threads;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < ALLOCATIONS; ++j) {
                    allocator.allocate(64);  // 64 bytes each
                }
            });
        }
        
        for (auto& t : threads) t.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Arena Allocator:\n";
        std::cout << "  Time: " << duration.count() << " ms\n";
        std::cout << "  Allocations: " << (NUM_THREADS * ALLOCATIONS) << "\n";
        std::cout << "  Ops/sec: " << ((NUM_THREADS * ALLOCATIONS * 1000LL) / duration.count()) << "\n";
    }
    
    // Test with malloc (baseline)
    {
        std::vector<std::thread> threads;
        std::vector<std::vector<void*>> ptrs_per_thread(NUM_THREADS);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < ALLOCATIONS; ++j) {
                    void* ptr = std::malloc(64);
                    ptrs_per_thread[i].push_back(ptr);
                }
            });
        }
        
        for (auto& t : threads) t.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "\nmalloc (baseline):\n";
        std::cout << "  Time: " << duration.count() << " ms\n";
        std::cout << "  Allocations: " << (NUM_THREADS * ALLOCATIONS) << "\n";
        std::cout << "  Ops/sec: " << ((NUM_THREADS * ALLOCATIONS * 1000LL) / duration.count()) << "\n";
        
        // Cleanup
        for (auto& ptrs : ptrs_per_thread) {
            for (void* ptr : ptrs) {
                std::free(ptr);
            }
        }
    }
}

// Example 6: Use Cases
void example_use_cases() {
    print_divider("Real-world Use Cases");
    
    std::cout << "1. Request Handler (Web Server):\n";
    std::cout << "   - Each request gets own arena\n";
    std::cout << "   - Fast allocation during request processing\n";
    std::cout << "   - Reset arena after request completes\n\n";
    
    std::cout << "2. Compiler/Parser:\n";
    std::cout << "   - Per-file compilation arena\n";
    std::cout << "   - AST nodes allocated from arena\n";
    std::cout << "   - Reset after file completes\n\n";
    
    std::cout << "3. Game Engine Frame:\n";
    std::cout << "   - Per-frame temporary allocations\n";
    std::cout << "   - No locks (thread-local)\n";
    std::cout << "   - Reset at frame end\n\n";
    
    std::cout << "4. Database Query:\n";
    std::cout << "   - Query result buffers\n";
    std::cout << "   - Intermediate data structures\n";
    std::cout << "   - Reset after query completes\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║      Thread Arena Allocator Examples           ║\n";
    std::cout << "║        Per-Thread Memory Pools                 ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_basic_arena();
    example_multithreaded();
    example_scoped_arena();
    example_arena_vector();
    example_performance();
    example_use_cases();
    
    std::cout << "\n✅ All arena allocator examples completed successfully!\n";
    return 0;
}

