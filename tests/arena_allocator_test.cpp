#include "arena_allocator/arena_allocator.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <atomic>

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

// Test 1: Basic Allocation
void test_basic_allocation() {
    ThreadArenaAllocator allocator(1024);
    
    void* ptr1 = allocator.allocate(64);
    test("Basic allocation returns non-null", ptr1 != nullptr);
    
    void* ptr2 = allocator.allocate(128);
    test("Second allocation returns non-null", ptr2 != nullptr);
    
    test("Allocations are different", ptr1 != ptr2);
    
    auto stats = allocator.get_thread_stats();
    test("Arena used bytes > 0", stats.used > 0);
    test("Arena capacity correct", stats.capacity == 1024);
}

// Test 2: Alignment
void test_alignment() {
    // Use separate allocators for each alignment to ensure clean state
    {
        ThreadArenaAllocator allocator(1024);
        void* ptr1 = allocator.allocate(17, 16);
        test("16-byte aligned allocation", ptr1 && reinterpret_cast<uintptr_t>(ptr1) % 16 == 0);
    }
    
    {
        ThreadArenaAllocator allocator(1024);
        void* ptr2 = allocator.allocate(33, 32);
        test("32-byte aligned allocation", ptr2 && reinterpret_cast<uintptr_t>(ptr2) % 32 == 0);
    }
    
    {
        ThreadArenaAllocator allocator(1024);
        void* ptr3 = allocator.allocate(65, 64);
        test("64-byte aligned allocation", ptr3 && reinterpret_cast<uintptr_t>(ptr3) % 64 == 0);
    }
}

// Test 3: Arena Exhaustion
void test_arena_exhaustion() {
    ThreadArenaAllocator allocator(512);
    
    void* ptr1 = allocator.allocate(256);
    test("First large allocation succeeds", ptr1 != nullptr);
    
    void* ptr2 = allocator.allocate(256);
    test("Second large allocation succeeds", ptr2 != nullptr);
    
    void* ptr3 = allocator.allocate(100);
    test("Allocation beyond capacity fails", ptr3 == nullptr);
}

// Test 4: Reset Thread Arena
void test_reset() {
    ThreadArenaAllocator allocator(1024);
    
    allocator.allocate(512);
    auto stats1 = allocator.get_thread_stats();
    test("Arena has used memory", stats1.used > 0);
    
    allocator.reset_thread_arena();
    auto stats2 = allocator.get_thread_stats();
    test("Arena reset clears memory", stats2.used == 0);
    test("Arena capacity unchanged after reset", stats2.capacity == 1024);
    
    void* ptr = allocator.allocate(64);
    test("Can allocate after reset", ptr != nullptr);
}

// Test 5: Multi-threaded Allocation
void test_multithreaded() {
    ThreadArenaAllocator allocator(1024 * 64);
    const int NUM_THREADS = 4;
    const int ALLOCS_PER_THREAD = 100;
    
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < ALLOCS_PER_THREAD; ++j) {
                void* ptr = allocator.allocate(64);
                if (ptr) success_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    test("All threads allocated successfully", 
         success_count.load() == NUM_THREADS * ALLOCS_PER_THREAD);
    
    auto global_stats = allocator.get_global_stats();
    test("Correct number of arenas created", global_stats.num_arenas == NUM_THREADS);
    test("Total capacity correct", 
         global_stats.total_capacity == NUM_THREADS * 1024 * 64);
}

// Test 6: Scoped Arena
void test_scoped_arena() {
    ThreadArenaAllocator allocator(1024);
    
    auto stats_before = allocator.get_thread_stats();
    
    {
        ScopedArena scope(allocator);
        scope.allocate(256);
        
        auto stats_during = allocator.get_thread_stats();
        test("Scoped arena allocates", stats_during.used > stats_before.used);
    }
    
    auto stats_after = allocator.get_thread_stats();
    test("Scoped arena resets on destruction", stats_after.used == 0);
}

// Test 7: Scoped Arena Create
void test_scoped_create() {
    ThreadArenaAllocator allocator(1024);
    
    {
        ScopedArena scope(allocator);
        
        int* num = scope.create<int>(42);
        test("Scoped create returns non-null", num != nullptr);
        test("Scoped create initializes value", *num == 42);
        
        struct TestStruct {
            int a, b;
            TestStruct(int x, int y) : a(x), b(y) {}
        };
        
        TestStruct* obj = scope.create<TestStruct>(10, 20);
        test("Scoped create with constructor", obj && obj->a == 10 && obj->b == 20);
    }
}

// Test 8: Arena Vector
void test_arena_vector() {
    ThreadArenaAllocator allocator(1024 * 8);
    
    ArenaVector<int> vec(allocator);
    test("Arena vector starts empty", vec.empty());
    test("Arena vector initial size is 0", vec.size() == 0);
    
    for (int i = 0; i < 50; ++i) {
        vec.push_back(i * 2);
    }
    
    test("Arena vector size after pushes", vec.size() == 50);
    test("Arena vector not empty", !vec.empty());
    test("Arena vector first element", vec[0] == 0);
    test("Arena vector last element", vec[49] == 98);
    test("Arena vector capacity grew", vec.capacity() >= 50);
}

// Test 9: Arena Vector Iteration
void test_arena_vector_iteration() {
    ThreadArenaAllocator allocator(1024);
    
    ArenaVector<int> vec(allocator);
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    int sum = 0;
    for (int val : vec) {
        sum += val;
    }
    
    test("Arena vector iteration", sum == 45);  // 0+1+2+...+9 = 45
}

// Test 10: Global Statistics
void test_global_stats() {
    ThreadArenaAllocator allocator(1024);
    
    allocator.allocate(256);
    
    auto stats = allocator.get_global_stats();
    test("Global stats num_arenas", stats.num_arenas >= 1);
    test("Global stats total_capacity", stats.total_capacity >= 1024);
    test("Global stats total_used", stats.total_used >= 256);
    test("Global stats total_available", stats.total_available <= stats.total_capacity);
}

// Test 11: Reset All Arenas
void test_reset_all() {
    ThreadArenaAllocator allocator(1024);
    const int NUM_THREADS = 4;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            allocator.allocate(256);
        });
    }
    for (auto& t : threads) t.join();
    
    auto stats_before = allocator.get_global_stats();
    test("Arenas have used memory", stats_before.total_used > 0);
    
    allocator.reset_all();
    
    auto stats_after = allocator.get_global_stats();
    test("Reset all clears all arenas", stats_after.total_used == 0);
    test("Arenas still exist after reset all", 
         stats_after.num_arenas == stats_before.num_arenas);
}

// Test 12: Type-safe Allocator
void test_typed_allocator() {
    ThreadArenaAllocator allocator(1024);
    TypedArenaAllocator<int> typed(allocator);
    
    int* numbers = typed.allocate(10);
    test("Typed allocator allocation", numbers != nullptr);
    
    for (int i = 0; i < 10; ++i) {
        typed.construct(&numbers[i], i * 5);
    }
    
    test("Typed allocator construct", numbers[0] == 0 && numbers[9] == 45);
    
    for (int i = 0; i < 10; ++i) {
        typed.destroy(&numbers[i]);
    }
    
    typed.deallocate(numbers, 10);  // No-op for arena
}

// Test 13: Arena Vector Growth
void test_arena_vector_growth() {
    ThreadArenaAllocator allocator(1024 * 16);
    
    ArenaVector<int> vec(allocator, 4);
    test("Initial capacity", vec.capacity() == 4);
    
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    test("Capacity grew after exceeding initial", vec.capacity() >= 8);
    test("All elements preserved", vec[0] == 0 && vec[4] == 4);
}

// Test 14: Concurrent Arena Access
void test_concurrent_access() {
    ThreadArenaAllocator allocator(1024 * 128);
    const int NUM_THREADS = 8;
    std::atomic<bool> all_succeeded{true};
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 100; ++j) {
                void* ptr = allocator.allocate(128);
                if (!ptr) {
                    all_succeeded.store(false, std::memory_order_relaxed);
                    break;
                }
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    test("Concurrent access all succeeded", all_succeeded.load());
    test("Correct number of arenas after concurrent access", 
         allocator.num_arenas() == NUM_THREADS);
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║     Thread Arena Allocator Test Suite         ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    test_basic_allocation();
    test_alignment();
    test_arena_exhaustion();
    test_reset();
    test_multithreaded();
    test_scoped_arena();
    test_scoped_create();
    test_arena_vector();
    test_arena_vector_iteration();
    test_global_stats();
    test_reset_all();
    test_typed_allocator();
    test_arena_vector_growth();
    test_concurrent_access();
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Tests passed: " << g_tests_passed << "\n";
    std::cout << "Tests failed: " << g_tests_failed << "\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return g_tests_failed == 0 ? 0 : 1;
}

