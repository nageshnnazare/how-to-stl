// ============================================================================
//  arena_allocator_example.cpp — per-thread bump arenas in action
// ============================================================================
//
// Demonstrates:
//   1. Basic single-thread bump allocation + stats
//   2. Multi-threaded allocation (one arena per thread, no alloc locks)
//   3. ScopedArena RAII reset at scope exit
//   4. ArenaVector growth backed by arena bumps
//   5. Throughput comparison vs malloc under contention
//   6. Real-world use-case patterns
//
// Build (from repo root):
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. \
//       arena_allocator/arena_allocator_example.cpp -o /tmp/x_arena_allocator
// ============================================================================

#include "arena_allocator.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Example 1: Basic Arena Usage — single thread, bump + stats
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

// Example 2: Multi-threaded — each worker gets a private arena (no alloc lock)
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

// Example 3: ScopedArena — RAII bulk reset when scope ends
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

// Example 4: ArenaVector — vector-like container using arena storage
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


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: with threads, line ordering / counts may vary between runs.
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║      Thread Arena Allocator Examples           ║
║        Per-Thread Memory Pools                 ║
╚════════════════════════════════════════════════╝

=== Basic Thread Arena Usage ===
Allocating from main thread...
Numbers: 0 10 20 30 40 50 60 70 80 90 
Thread stats:
  Used: 40 bytes
  Available: 65496 bytes
  Capacity: 65536 bytes

=== Multi-threaded Arena Allocation ===
Thread 0 starting...
Thread 1 starting...
Thread 2 starting...
Thread 3 starting...
Thread 0 stats:
  Used: 12800 bytes
  Utilization: 9.8%
Thread 1 stats:
  Used: 12800 bytes
  Utilization: 9.8%
Thread 2 stats:
  Used: 12800 bytes
  Utilization: 9.8%
Thread 3 stats:
  Used: 12800 bytes
  Utilization: 9.8%

Global stats:
  Number of arenas: 4
  Total capacity: 524288 bytes
  Total used: 51200 bytes
  Total allocations: 400
  Time: 418 μs

=== Scoped Arena (RAII) ===
Before scope:
  Used: 0 bytes
Allocated 100 integers
Within scope:
  Used: 400 bytes
After scope (arena reset):
  Used: 0 bytes

=== Arena Vector ===
Pushing elements...
Vector contents (20 elements): 0 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 
Capacity: 32
Arena used: 192 bytes

=== Performance Comparison ===
Arena Allocator:
  Time: 38 ms
  Allocations: 80000
  Ops/sec: 2105263

malloc (baseline):
  Time: 2 ms
  Allocations: 80000
  Ops/sec: 40000000

=== Real-world Use Cases ===
1. Request Handler (Web Server):
   - Each request gets own arena
   - Fast allocation during request processing
   - Reset arena after request completes

2. Compiler/Parser:
   - Per-file compilation arena
   - AST nodes allocated from arena
   - Reset after file completes

3. Game Engine Frame:
   - Per-frame temporary allocations
   - No locks (thread-local)
   - Reset at frame end

4. Database Query:
   - Query result buffers
   - Intermediate data structures
   - Reset after query completes

✅ All arena allocator examples completed successfully!
 * ============================================================================ */
