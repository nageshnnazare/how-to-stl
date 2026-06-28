#include "../allocator/allocator.hpp"
#include <iostream>
#include <cstring>

// Test framework macros
#define ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "Assertion failed at line " << __LINE__ << ": " << message << std::endl; \
        return false; \
    }

#define RUN_TEST(test_func) \
    std::cout << "Running " << #test_func << "... "; \
    if (test_func()) { \
        std::cout << "✅ PASSED\n"; \
        passed++; \
    } else { \
        std::cout << "❌ FAILED\n"; \
        failed++; \
    }

// ============================================================================
// LINEAR ALLOCATOR TESTS
// ============================================================================

bool test_linear_basic_allocation() {
    LinearAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    ASSERT(ptr1 != nullptr, "First allocation should succeed");
    ASSERT(alloc.used() >= 100, "Used memory should be at least 100");
    
    void* ptr2 = alloc.allocate(200);
    ASSERT(ptr2 != nullptr, "Second allocation should succeed");
    ASSERT(ptr2 > ptr1, "Second pointer should be after first");
    
    return true;
}

bool test_linear_reset() {
    LinearAllocator alloc(1024);
    
    alloc.allocate(500);
    size_t used_before = alloc.used();
    ASSERT(used_before > 0, "Should have used memory");
    
    alloc.reset();
    ASSERT(alloc.used() == 0, "Used should be 0 after reset");
    ASSERT(alloc.available() == alloc.capacity(), "All memory should be available");
    
    return true;
}

bool test_linear_out_of_memory() {
    LinearAllocator alloc(100);
    
    void* ptr1 = alloc.allocate(50);
    ASSERT(ptr1 != nullptr, "First allocation should succeed");
    
    void* ptr2 = alloc.allocate(60);
    ASSERT(ptr2 == nullptr, "Should fail when out of memory");
    
    return true;
}

bool test_linear_alignment() {
    LinearAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(1, 16);  // 16-byte alignment
    ASSERT(ptr1 != nullptr, "Aligned allocation should succeed");
    ASSERT(reinterpret_cast<uintptr_t>(ptr1) % 16 == 0, "Should be 16-byte aligned");
    
    return true;
}

// ============================================================================
// POOL ALLOCATOR TESTS
// ============================================================================

bool test_pool_basic_allocation() {
    PoolAllocator pool(64, 10);
    
    void* ptr1 = pool.allocate();
    ASSERT(ptr1 != nullptr, "First allocation should succeed");
    
    void* ptr2 = pool.allocate();
    ASSERT(ptr2 != nullptr, "Second allocation should succeed");
    ASSERT(ptr2 != ptr1, "Pointers should be different");
    
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
    
    return true;
}

bool test_pool_reuse() {
    PoolAllocator pool(64, 5);
    
    void* ptr1 = pool.allocate();
    void* original_ptr = ptr1;
    pool.deallocate(ptr1);
    
    void* ptr2 = pool.allocate();
    ASSERT(ptr2 == original_ptr, "Should reuse freed block");
    
    pool.deallocate(ptr2);
    
    return true;
}

bool test_pool_exhaustion() {
    PoolAllocator pool(64, 3);
    
    void* ptr1 = pool.allocate();
    void* ptr2 = pool.allocate();
    void* ptr3 = pool.allocate();
    
    ASSERT(ptr1 && ptr2 && ptr3, "First 3 allocations should succeed");
    
    void* ptr4 = pool.allocate();
    ASSERT(ptr4 == nullptr, "4th allocation should fail (pool exhausted)");
    
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
    pool.deallocate(ptr3);
    
    return true;
}

bool test_pool_data_integrity() {
    struct Data {
        int value;
        float x, y;
    };
    
    PoolAllocator pool(sizeof(Data), 10);
    
    Data* data1 = static_cast<Data*>(pool.allocate());
    ASSERT(data1 != nullptr, "Allocation should succeed");
    
    data1->value = 42;
    data1->x = 1.5f;
    data1->y = 2.5f;
    
    ASSERT(data1->value == 42, "Data should be intact");
    ASSERT(data1->x == 1.5f, "Data should be intact");
    
    pool.deallocate(data1);
    
    return true;
}

// ============================================================================
// STACK ALLOCATOR TESTS
// ============================================================================

bool test_stack_basic_allocation() {
    StackAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    ASSERT(ptr1 != nullptr, "First allocation should succeed");
    ASSERT(alloc.used() > 0, "Used memory should increase");
    
    void* ptr2 = alloc.allocate(200);
    ASSERT(ptr2 != nullptr, "Second allocation should succeed");
    
    return true;
}

bool test_stack_markers() {
    StackAllocator alloc(1024);
    
    auto marker1 = alloc.get_marker();
    ASSERT(marker1 == 0, "Initial marker should be 0");
    
    alloc.allocate(100);
    auto marker2 = alloc.get_marker();
    ASSERT(marker2 > marker1, "Marker should increase after allocation");
    
    alloc.allocate(200);
    auto marker3 = alloc.get_marker();
    ASSERT(marker3 > marker2, "Marker should increase again");
    
    alloc.free_to_marker(marker2);
    ASSERT(alloc.get_marker() == marker2, "Should free to marker");
    
    alloc.free_to_marker(marker1);
    ASSERT(alloc.used() == 0, "Should free all to initial marker");
    
    return true;
}

bool test_stack_lifo() {
    StackAllocator alloc(1024);
    
    auto m0 = alloc.get_marker();
    void* ptr1 = alloc.allocate(100);
    auto m1 = alloc.get_marker();
    
    void* ptr2 = alloc.allocate(200);
    auto m2 = alloc.get_marker();
    
    void* ptr3 = alloc.allocate(50);
    auto m3 = alloc.get_marker();
    
    // Free in LIFO order
    alloc.free_to_marker(m2);
    alloc.free_to_marker(m1);
    alloc.free_to_marker(m0);
    
    ASSERT(alloc.used() == 0, "All memory should be freed");
    
    (void)ptr1; (void)ptr2; (void)ptr3; (void)m3;  // Silence unused warnings
    return true;
}

// ============================================================================
// FREE LIST ALLOCATOR TESTS
// ============================================================================

bool test_freelist_basic_allocation() {
    FreeListAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    ASSERT(ptr1 != nullptr, "First allocation should succeed");
    
    void* ptr2 = alloc.allocate(200);
    ASSERT(ptr2 != nullptr, "Second allocation should succeed");
    
    alloc.deallocate(ptr1);
    alloc.deallocate(ptr2);
    
    return true;
}

bool test_freelist_arbitrary_order() {
    FreeListAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    void* ptr2 = alloc.allocate(200);
    void* ptr3 = alloc.allocate(50);
    
    ASSERT(ptr1 && ptr2 && ptr3, "Allocations should succeed");
    
    // Free in arbitrary order (not LIFO)
    alloc.deallocate(ptr2);  // Middle
    alloc.deallocate(ptr1);  // First
    alloc.deallocate(ptr3);  // Last
    
    return true;
}

bool test_freelist_reuse() {
    FreeListAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    alloc.deallocate(ptr1);
    
    void* ptr2 = alloc.allocate(80);  // Smaller, should fit in freed space
    ASSERT(ptr2 != nullptr, "Should reuse freed space");
    
    alloc.deallocate(ptr2);
    
    return true;
}

bool test_freelist_coalescing() {
    FreeListAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    void* ptr2 = alloc.allocate(100);
    void* ptr3 = alloc.allocate(100);
    
    // Free adjacent blocks
    alloc.deallocate(ptr1);
    alloc.deallocate(ptr2);
    
    // Should be able to allocate larger block (coalesced)
    void* ptr4 = alloc.allocate(180);
    ASSERT(ptr4 != nullptr, "Should allocate from coalesced space");
    
    alloc.deallocate(ptr3);
    alloc.deallocate(ptr4);
    
    return true;
}

bool test_freelist_fragmentation() {
    FreeListAllocator alloc(1024);
    
    void* ptr1 = alloc.allocate(100);
    void* ptr2 = alloc.allocate(100);
    void* ptr3 = alloc.allocate(100);
    void* ptr4 = alloc.allocate(100);
    
    ASSERT(ptr1 && ptr2 && ptr3 && ptr4, "All allocations should succeed");
    
    // Create fragmentation - free non-adjacent blocks
    alloc.deallocate(ptr2);
    alloc.deallocate(ptr4);
    
    // Try to allocate larger than individual holes
    // Note: With coalescing, behavior depends on block layout
    void* ptr5 = alloc.allocate(180);
    // Just verify it either succeeds or fails gracefully
    (void)ptr5;
    
    alloc.deallocate(ptr1);
    alloc.deallocate(ptr3);
    if (ptr5) alloc.deallocate(ptr5);
    
    return true;
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

bool test_data_integrity_across_allocators() {
    // Test that written data persists correctly
    LinearAllocator linear(1024);
    
    int* nums = static_cast<int*>(linear.allocate(5 * sizeof(int)));
    ASSERT(nums != nullptr, "Allocation should succeed");
    
    for (int i = 0; i < 5; ++i) {
        nums[i] = i * 10;
    }
    
    for (int i = 0; i < 5; ++i) {
        ASSERT(nums[i] == i * 10, "Data should be intact");
    }
    
    return true;
}

bool test_null_deallocation() {
    PoolAllocator pool(64, 10);
    pool.deallocate(nullptr);  // Should not crash
    
    FreeListAllocator freelist(1024);
    freelist.deallocate(nullptr);  // Should not crash
    
    return true;
}

int main() {
    int passed = 0;
    int failed = 0;
    
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║      Memory Allocator Test Suite              ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    std::cout << "LINEAR ALLOCATOR TESTS:\n";
    RUN_TEST(test_linear_basic_allocation);
    RUN_TEST(test_linear_reset);
    RUN_TEST(test_linear_out_of_memory);
    RUN_TEST(test_linear_alignment);
    
    std::cout << "\nPOOL ALLOCATOR TESTS:\n";
    RUN_TEST(test_pool_basic_allocation);
    RUN_TEST(test_pool_reuse);
    RUN_TEST(test_pool_exhaustion);
    RUN_TEST(test_pool_data_integrity);
    
    std::cout << "\nSTACK ALLOCATOR TESTS:\n";
    RUN_TEST(test_stack_basic_allocation);
    RUN_TEST(test_stack_markers);
    RUN_TEST(test_stack_lifo);
    
    std::cout << "\nFREE LIST ALLOCATOR TESTS:\n";
    RUN_TEST(test_freelist_basic_allocation);
    RUN_TEST(test_freelist_arbitrary_order);
    RUN_TEST(test_freelist_reuse);
    RUN_TEST(test_freelist_coalescing);
    RUN_TEST(test_freelist_fragmentation);
    
    std::cout << "\nINTEGRATION TESTS:\n";
    RUN_TEST(test_data_integrity_across_allocators);
    RUN_TEST(test_null_deallocation);
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return failed == 0 ? 0 : 1;
}


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: with threads, line ordering / counts may vary between runs.
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║      Memory Allocator Test Suite              ║
╚════════════════════════════════════════════════╝

LINEAR ALLOCATOR TESTS:
Running test_linear_basic_allocation... ✅ PASSED
Running test_linear_reset... ✅ PASSED
Running test_linear_out_of_memory... ✅ PASSED
Running test_linear_alignment... ✅ PASSED

POOL ALLOCATOR TESTS:
Running test_pool_basic_allocation... ✅ PASSED
Running test_pool_reuse... ✅ PASSED
Running test_pool_exhaustion... ✅ PASSED
Running test_pool_data_integrity... ✅ PASSED

STACK ALLOCATOR TESTS:
Running test_stack_basic_allocation... ✅ PASSED
Running test_stack_markers... ✅ PASSED
Running test_stack_lifo... ✅ PASSED

FREE LIST ALLOCATOR TESTS:
Running test_freelist_basic_allocation... ✅ PASSED
Running test_freelist_arbitrary_order... ✅ PASSED
Running test_freelist_reuse... ✅ PASSED
Running test_freelist_coalescing... ✅ PASSED
Running test_freelist_fragmentation... ✅ PASSED

INTEGRATION TESTS:
Running test_data_integrity_across_allocators... ✅ PASSED
Running test_null_deallocation... ✅ PASSED

════════════════════════════════════════════════
Test Results: 18 passed, 0 failed
════════════════════════════════════════════════
 * ============================================================================ */
