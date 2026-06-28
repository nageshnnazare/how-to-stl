// ============================================================================
//  allocator_example.cpp — runnable tour of four custom allocators
// ============================================================================
//
// Demonstrates:
//   1. LinearAllocator   — per-frame bump + reset
//   2. PoolAllocator     — fixed-size free list (spawn/despawn)
//   3. StackAllocator    — scope markers (nested unwind)
//   4. FreeListAllocator — variable sizes + coalescing
//   5. Performance       — linear/pool vs malloc
//   6. Use-case summary
//
// Build (from repo root):
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. \
//       allocator/allocator_example.cpp -o /tmp/x_allocator
// ============================================================================

#include "allocator.hpp"
#include <iostream>
#include <vector>
#include <chrono>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// ============================================================================
// EXAMPLE 1: Linear Allocator - Fast Sequential Allocation
// ============================================================================
void example_linear_allocator() {
    print_divider("Linear Allocator - Frame Allocations");
    
    LinearAllocator allocator(1024 * 1024);  // 1 MB
    
    std::cout << "Initial state:\n";
    std::cout << "  Capacity: " << allocator.capacity() << " bytes\n";
    std::cout << "  Used: " << allocator.used() << " bytes\n";
    std::cout << "  Available: " << allocator.available() << " bytes\n\n";
    
    // Allocate some data
    int* numbers = static_cast<int*>(allocator.allocate(10 * sizeof(int)));
    if (numbers) {
        for (int i = 0; i < 10; ++i) {
            numbers[i] = i * 10;
        }
        std::cout << "Allocated 10 integers: ";
        for (int i = 0; i < 10; ++i) {
            std::cout << numbers[i] << " ";
        }
        std::cout << "\n";
    }
    
    std::cout << "After allocation:\n";
    std::cout << "  Used: " << allocator.used() << " bytes\n";
    std::cout << "  Available: " << allocator.available() << " bytes\n\n";
    
    // Simulate frame allocations
    std::cout << "Simulating frame allocations...\n";
    for (int frame = 0; frame < 3; ++frame) {
        std::cout << "Frame " << frame << ":\n";
        
        // Allocate temporary data for this frame
        float* temp_data = static_cast<float*>(allocator.allocate(100 * sizeof(float)));
        if (temp_data) {
            temp_data[0] = frame * 1.5f;
            std::cout << "  Allocated temp data, first value: " << temp_data[0] << "\n";
        }
        
        std::cout << "  Used: " << allocator.used() << " bytes\n";
        
        // At end of frame, reset allocator
        allocator.reset();
        std::cout << "  After reset: " << allocator.used() << " bytes\n";
    }
}

// ============================================================================
// EXAMPLE 2: Pool Allocator - Object Pools
// ============================================================================
struct GameObject {
    int id;
    float x, y, z;
    bool active;
    
    GameObject() : id(0), x(0), y(0), z(0), active(true) {}
};

void example_pool_allocator() {
    print_divider("Pool Allocator - GameObject Pool");
    
    const size_t MAX_OBJECTS = 100;
    PoolAllocator pool(sizeof(GameObject), MAX_OBJECTS);
    
    std::cout << "Created pool for " << MAX_OBJECTS << " GameObjects\n";
    std::cout << "Block size: " << pool.block_size() << " bytes\n\n";
    
    // Allocate some game objects
    std::vector<GameObject*> objects;
    
    std::cout << "Spawning 5 game objects...\n";
    for (int i = 0; i < 5; ++i) {
        GameObject* obj = static_cast<GameObject*>(pool.allocate());
        if (obj) {
            new (obj) GameObject();  // Placement new
            obj->id = i + 1;
            obj->x = i * 10.0f;
            obj->y = i * 5.0f;
            obj->z = 0.0f;
            objects.push_back(obj);
            
            std::cout << "  GameObject " << obj->id 
                      << " at (" << obj->x << ", " << obj->y << ", " << obj->z << ")\n";
        }
    }
    
    std::cout << "\nDestroying object 3...\n";
    if (objects.size() > 2) {
        GameObject* obj = objects[2];
        std::cout << "  Destroying GameObject " << obj->id << "\n";
        obj->~GameObject();  // Manual destructor call
        pool.deallocate(obj);
        objects[2] = nullptr;
    }
    
    std::cout << "\nSpawning new object (reuses freed memory)...\n";
    GameObject* new_obj = static_cast<GameObject*>(pool.allocate());
    if (new_obj) {
        new (new_obj) GameObject();
        new_obj->id = 99;
        new_obj->x = 999.0f;
        std::cout << "  New GameObject " << new_obj->id << " at (" << new_obj->x << ")\n";
    }
    
    // Cleanup
    for (GameObject* obj : objects) {
        if (obj) {
            obj->~GameObject();
            pool.deallocate(obj);
        }
    }
    if (new_obj) {
        new_obj->~GameObject();
        pool.deallocate(new_obj);
    }
}

// ============================================================================
// EXAMPLE 3: Stack Allocator - Scope-based Allocation
// ============================================================================
void example_stack_allocator() {
    print_divider("Stack Allocator - Scope-based Memory");
    
    StackAllocator allocator(1024 * 64);  // 64 KB
    
    std::cout << "Available: " << allocator.available() << " bytes\n\n";
    
    // Outer scope
    {
        std::cout << "Outer scope:\n";
        auto marker1 = allocator.get_marker();
        
        int* data1 = static_cast<int*>(allocator.allocate(10 * sizeof(int)));
        if (data1) {
            data1[0] = 100;
            std::cout << "  Allocated data1, value: " << data1[0] << "\n";
            std::cout << "  Used: " << allocator.used() << " bytes\n";
        }
        
        // Inner scope
        {
            std::cout << "\n  Inner scope:\n";
            auto marker2 = allocator.get_marker();
            
            float* data2 = static_cast<float*>(allocator.allocate(20 * sizeof(float)));
            if (data2) {
                data2[0] = 3.14f;
                std::cout << "    Allocated data2, value: " << data2[0] << "\n";
                std::cout << "    Used: " << allocator.used() << " bytes\n";
            }
            
            // Exit inner scope - free to marker
            std::cout << "\n  Exiting inner scope...\n";
            allocator.free_to_marker(marker2);
            std::cout << "  After free: Used = " << allocator.used() << " bytes\n";
        }
        
        // Exit outer scope
        std::cout << "\nExiting outer scope...\n";
        allocator.free_to_marker(marker1);
        std::cout << "After free: Used = " << allocator.used() << " bytes\n";
    }
}

// ============================================================================
// EXAMPLE 4: Free List Allocator - General Purpose
// ============================================================================
void example_freelist_allocator() {
    print_divider("Free List Allocator - General Purpose");
    
    FreeListAllocator allocator(1024);  // 1 KB
    
    std::cout << "Capacity: " << allocator.capacity() << " bytes\n\n";
    
    // Allocate various sizes
    std::cout << "Allocating various sizes:\n";
    void* ptr1 = allocator.allocate(100);
    std::cout << "  ptr1: 100 bytes at " << ptr1 << "\n";
    
    void* ptr2 = allocator.allocate(200);
    std::cout << "  ptr2: 200 bytes at " << ptr2 << "\n";
    
    void* ptr3 = allocator.allocate(50);
    std::cout << "  ptr3: 50 bytes at " << ptr3 << "\n";
    
    // Free middle block
    std::cout << "\nFreeing ptr2 (creates hole)...\n";
    allocator.deallocate(ptr2);
    
    // Allocate smaller size (should fit in hole)
    std::cout << "Allocating 150 bytes (should fit in freed space)...\n";
    void* ptr4 = allocator.allocate(150);
    std::cout << "  ptr4: 150 bytes at " << ptr4 << "\n";
    
    // Cleanup
    std::cout << "\nCleaning up...\n";
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);
    allocator.deallocate(ptr4);
    std::cout << "All memory freed!\n";
}

// ============================================================================
// EXAMPLE 5: Performance Comparison
// ============================================================================
void example_performance() {
    print_divider("Performance Comparison");
    
    const int NUM_ALLOCS = 10000;
    
    // Test Linear Allocator
    {
        LinearAllocator allocator(1024 * 1024 * 10);
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_ALLOCS; ++i) {
            void* ptr = allocator.allocate(64);
            (void)ptr;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Linear Allocator:\n";
        std::cout << "  " << NUM_ALLOCS << " allocations in " << duration.count() << " μs\n";
        std::cout << "  Average: " << (duration.count() / (double)NUM_ALLOCS) << " μs/alloc\n\n";
    }
    
    // Test Pool Allocator
    {
        PoolAllocator pool(64, NUM_ALLOCS);
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<void*> ptrs;
        for (int i = 0; i < NUM_ALLOCS; ++i) {
            void* ptr = pool.allocate();
            ptrs.push_back(ptr);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Pool Allocator:\n";
        std::cout << "  " << NUM_ALLOCS << " allocations in " << duration.count() << " μs\n";
        std::cout << "  Average: " << (duration.count() / (double)NUM_ALLOCS) << " μs/alloc\n\n";
        
        // Cleanup
        for (void* ptr : ptrs) {
            pool.deallocate(ptr);
        }
    }
    
    // Test malloc (baseline)
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<void*> ptrs;
        for (int i = 0; i < NUM_ALLOCS; ++i) {
            void* ptr = std::malloc(64);
            ptrs.push_back(ptr);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "malloc (baseline):\n";
        std::cout << "  " << NUM_ALLOCS << " allocations in " << duration.count() << " μs\n";
        std::cout << "  Average: " << (duration.count() / (double)NUM_ALLOCS) << " μs/alloc\n";
        
        // Cleanup
        for (void* ptr : ptrs) {
            std::free(ptr);
        }
    }
}

// ============================================================================
// EXAMPLE 6: Real-world Use Cases
// ============================================================================
void example_use_cases() {
    print_divider("Real-world Use Cases");
    
    std::cout << "1. Game Engine Frame Allocations:\n";
    std::cout << "   - Use LinearAllocator for per-frame temporary data\n";
    std::cout << "   - Reset at end of each frame\n";
    std::cout << "   - Ultra-fast, zero fragmentation\n\n";
    
    std::cout << "2. Entity Component System:\n";
    std::cout << "   - Use PoolAllocator for entities\n";
    std::cout << "   - All entities same size\n";
    std::cout << "   - Fast spawn/despawn\n\n";
    
    std::cout << "3. Recursive Algorithms:\n";
    std::cout << "   - Use StackAllocator for call stack data\n";
    std::cout << "   - LIFO allocation/deallocation\n";
    std::cout << "   - Automatic cleanup on unwind\n\n";
    
    std::cout << "4. General Purpose (like malloc):\n";
    std::cout << "   - Use FreeListAllocator\n";
    std::cout << "   - Arbitrary sizes and order\n";
    std::cout << "   - Block coalescing reduces fragmentation\n";
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║       Custom Memory Allocator Examples         ║\n";
    std::cout << "║    Linear, Pool, Stack, Free List Allocators   ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_linear_allocator();
    example_pool_allocator();
    example_stack_allocator();
    example_freelist_allocator();
    example_performance();
    example_use_cases();
    
    std::cout << "\n✅ All allocator examples completed successfully!\n";
    return 0;
}


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: pointer addresses below are from one run and will differ on yours.
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║       Custom Memory Allocator Examples         ║
║    Linear, Pool, Stack, Free List Allocators   ║
╚════════════════════════════════════════════════╝

=== Linear Allocator - Frame Allocations ===
Initial state:
  Capacity: 1048576 bytes
  Used: 0 bytes
  Available: 1048576 bytes

Allocated 10 integers: 0 10 20 30 40 50 60 70 80 90 
After allocation:
  Used: 40 bytes
  Available: 1048536 bytes

Simulating frame allocations...
Frame 0:
  Allocated temp data, first value: 0
  Used: 440 bytes
  After reset: 0 bytes
Frame 1:
  Allocated temp data, first value: 1.5
  Used: 400 bytes
  After reset: 0 bytes
Frame 2:
  Allocated temp data, first value: 3
  Used: 400 bytes
  After reset: 0 bytes

=== Pool Allocator - GameObject Pool ===
Created pool for 100 GameObjects
Block size: 20 bytes

Spawning 5 game objects...
  GameObject 1 at (0, 0, 0)
  GameObject 2 at (10, 5, 0)
  GameObject 3 at (20, 10, 0)
  GameObject 4 at (30, 15, 0)
  GameObject 5 at (40, 20, 0)

Destroying object 3...
  Destroying GameObject 3

Spawning new object (reuses freed memory)...
  New GameObject 99 at (999)

=== Stack Allocator - Scope-based Memory ===
Available: 65536 bytes

Outer scope:
  Allocated data1, value: 100
  Used: 56 bytes

  Inner scope:
    Allocated data2, value: 3.14
    Used: 152 bytes

  Exiting inner scope...
  After free: Used = 56 bytes

Exiting outer scope...
After free: Used = 0 bytes

=== Free List Allocator - General Purpose ===
Capacity: 1024 bytes

Allocating various sizes:
  ptr1: 100 bytes at 0x10149df20
  ptr2: 200 bytes at 0x10149df94
  ptr3: 50 bytes at 0x10149e06c

Freeing ptr2 (creates hole)...
Allocating 150 bytes (should fit in freed space)...
  ptr4: 150 bytes at 0x10149df94

Cleaning up...
All memory freed!

=== Performance Comparison ===
Linear Allocator:
  10000 allocations in 68 μs
  Average: 0.0068 μs/alloc

Pool Allocator:
  10000 allocations in 405 μs
  Average: 0.0405 μs/alloc

malloc (baseline):
  10000 allocations in 746 μs
  Average: 0.0746 μs/alloc

=== Real-world Use Cases ===
1. Game Engine Frame Allocations:
   - Use LinearAllocator for per-frame temporary data
   - Reset at end of each frame
   - Ultra-fast, zero fragmentation

2. Entity Component System:
   - Use PoolAllocator for entities
   - All entities same size
   - Fast spawn/despawn

3. Recursive Algorithms:
   - Use StackAllocator for call stack data
   - LIFO allocation/deallocation
   - Automatic cleanup on unwind

4. General Purpose (like malloc):
   - Use FreeListAllocator
   - Arbitrary sizes and order
   - Block coalescing reduces fragmentation

✅ All allocator examples completed successfully!
 * ============================================================================ */
