#include "../locks/locks.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

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
// MUTEX LOCK TESTS
// ============================================================================

bool test_mutex_basic() {
    MutexLock mutex;
    mutex.lock();
    mutex.unlock();
    return true;
}

bool test_mutex_counter() {
    MutexLock mutex;
    int counter = 0;
    const int NUM_THREADS = 10;
    const int INCREMENTS = 100;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < INCREMENTS; ++j) {
                MutexLock::Guard guard(mutex);
                counter++;
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(counter == NUM_THREADS * INCREMENTS, "Counter should be correct");
    return true;
}

bool test_mutex_try_lock() {
    MutexLock mutex;
    
    ASSERT(mutex.try_lock(), "First try_lock should succeed");
    ASSERT(!mutex.try_lock(), "Second try_lock should fail (already locked)");
    
    mutex.unlock();
    ASSERT(mutex.try_lock(), "try_lock after unlock should succeed");
    mutex.unlock();
    
    return true;
}

bool test_mutex_guard() {
    MutexLock mutex;
    int value = 0;
    
    {
        MutexLock::Guard guard(mutex);
        value = 42;
    }  // Guard destructor unlocks
    
    // Should be able to lock again
    mutex.lock();
    ASSERT(value == 42, "Value should be 42");
    mutex.unlock();
    
    return true;
}

// ============================================================================
// POINTER LOCK TESTS
// ============================================================================

bool test_pointer_basic() {
    int value = 42;
    PointerLock<int> ptr(&value);
    
    ASSERT(ptr.load() == &value, "Should load correct pointer");
    ASSERT(ptr.version() == 0, "Initial version should be 0");
    
    return true;
}

bool test_pointer_store() {
    int value1 = 10;
    int value2 = 20;
    
    PointerLock<int> ptr(&value1);
    ASSERT(ptr.load() == &value1, "Initial pointer should be value1");
    
    ptr.store(&value2);
    ASSERT(ptr.load() == &value2, "After store, pointer should be value2");
    ASSERT(ptr.version() == 1, "Version should increment");
    
    return true;
}

bool test_pointer_cas() {
    int value1 = 10;
    int value2 = 20;
    int value3 = 30;
    
    PointerLock<int> ptr(&value1);
    
    int* expected = &value1;
    ASSERT(ptr.compare_exchange_strong(expected, &value2), "CAS should succeed");
    ASSERT(ptr.load() == &value2, "Pointer should be value2");
    
    expected = &value1;  // Wrong expected
    ASSERT(!ptr.compare_exchange_strong(expected, &value3), "CAS should fail");
    ASSERT(ptr.load() == &value2, "Pointer should still be value2");
    
    return true;
}

bool test_pointer_concurrent() {
    std::atomic<int> success_count{0};
    PointerLock<int> ptr(nullptr);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&, i]() {
            int* local = new int(i);
            int* expected = nullptr;
            if (ptr.compare_exchange_strong(expected, local)) {
                success_count.fetch_add(1);
            } else {
                delete local;
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(success_count.load() == 1, "Only one CAS should succeed");
    delete ptr.load();
    
    return true;
}

// ============================================================================
// READ-WRITE LOCK TESTS
// ============================================================================

bool test_rwlock_basic() {
    ReadWriteLock rwlock;
    
    rwlock.lock_read();
    rwlock.unlock_read();
    
    rwlock.lock_write();
    rwlock.unlock_write();
    
    return true;
}

bool test_rwlock_multiple_readers() {
    ReadWriteLock rwlock;
    std::atomic<int> concurrent_readers{0};
    std::atomic<int> max_concurrent{0};
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            ReadWriteLock::ReadGuard guard(rwlock);
            int current = concurrent_readers.fetch_add(1) + 1;
            
            int expected_max = max_concurrent.load();
            while (current > expected_max && 
                   !max_concurrent.compare_exchange_weak(expected_max, current)) {}
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            concurrent_readers.fetch_sub(1);
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(max_concurrent.load() > 1, "Should have concurrent readers");
    return true;
}

bool test_rwlock_writer_exclusion() {
    ReadWriteLock rwlock;
    int shared_value = 0;
    std::atomic<bool> writer_active{false};
    
    std::vector<std::thread> threads;
    
    // Writer
    threads.emplace_back([&]() {
        ReadWriteLock::WriteGuard guard(rwlock);
        writer_active.store(true);
        shared_value = 42;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        writer_active.store(false);
    });
    
    // Readers
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ReadWriteLock::ReadGuard guard(rwlock);
            ASSERT(!writer_active.load(), "No writer should be active during read");
            int val = shared_value;
            (void)val;
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(shared_value == 42, "Value should be 42");
    return true;
}

bool test_rwlock_try_locks() {
    ReadWriteLock rwlock;
    
    ASSERT(rwlock.try_lock_read(), "First try_lock_read should succeed");
    ASSERT(rwlock.try_lock_read(), "Second try_lock_read should succeed");
    ASSERT(!rwlock.try_lock_write(), "try_lock_write should fail with readers");
    
    rwlock.unlock_read();
    rwlock.unlock_read();
    
    ASSERT(rwlock.try_lock_write(), "try_lock_write should succeed");
    ASSERT(!rwlock.try_lock_read(), "try_lock_read should fail with writer");
    
    rwlock.unlock_write();
    
    return true;
}

// ============================================================================
// RECURSIVE RW LOCK TESTS
// ============================================================================

bool test_recursive_rwlock_basic() {
    RecursiveRWLock rwlock;
    
    rwlock.lock_read();
    rwlock.unlock_read();
    
    rwlock.lock_write();
    rwlock.unlock_write();
    
    return true;
}

bool test_recursive_rwlock_recursive_read() {
    RecursiveRWLock rwlock;
    
    // Same thread can acquire read lock multiple times
    rwlock.lock_read();
    rwlock.lock_read();
    rwlock.lock_read();
    
    rwlock.unlock_read();
    rwlock.unlock_read();
    rwlock.unlock_read();
    
    return true;
}

bool test_recursive_rwlock_recursive_write() {
    RecursiveRWLock rwlock;
    int value = 0;
    
    rwlock.lock_write();
    value++;
    rwlock.lock_write();  // Recursive
    value++;
    rwlock.lock_write();  // More recursive
    value++;
    
    ASSERT(value == 3, "Value should be 3");
    
    rwlock.unlock_write();
    rwlock.unlock_write();
    rwlock.unlock_write();
    
    return true;
}

bool test_recursive_rwlock_nested_locks() {
    RecursiveRWLock rwlock;
    int value = 0;
    
    {
        RecursiveRWLock::WriteGuard guard1(rwlock);
        value = 10;
        
        {
            RecursiveRWLock::WriteGuard guard2(rwlock);  // Nested
            value = 20;
        }
        
        ASSERT(value == 20, "Value should be 20");
    }
    
    // Should be fully unlocked now
    RecursiveRWLock::WriteGuard guard3(rwlock);
    value = 30;
    ASSERT(value == 30, "Value should be 30");
    
    return true;
}

bool test_recursive_rwlock_writer_can_read() {
    RecursiveRWLock rwlock;
    int value = 0;
    
    rwlock.lock_write();
    value = 42;
    
    // Writer can acquire read lock
    rwlock.lock_read();
    ASSERT(value == 42, "Should be able to read");
    rwlock.unlock_read();
    
    rwlock.unlock_write();
    
    return true;
}

// ============================================================================
// RECURSIVE RW POINTER LOCK TESTS
// ============================================================================

struct TestData {
    int value;
    TestData(int v) : value(v) {}
};

bool test_recursive_rw_pointer_basic() {
    TestData data(42);
    RecursiveRWPointerLock<TestData> ptr(&data);
    
    {
        auto read = ptr.read();
        ASSERT(read->value == 42, "Should read correct value");
    }
    
    {
        auto write = ptr.write();
        write->value = 100;
    }
    
    {
        auto read = ptr.read();
        ASSERT(read->value == 100, "Should read updated value");
    }
    
    return true;
}

bool test_recursive_rw_pointer_version() {
    TestData data(0);
    RecursiveRWPointerLock<TestData> ptr(&data);
    
    ASSERT(ptr.version() == 0, "Initial version should be 0");
    
    {
        auto write = ptr.write();
        TestData* new_data = new TestData(42);
        write.set(new_data);
    }
    
    ASSERT(ptr.version() == 1, "Version should increment after set");
    
    {
        auto write = ptr.write();
        delete write.get();
        write.set(nullptr);
    }
    
    return true;
}

bool test_recursive_rw_pointer_concurrent() {
    TestData* data = new TestData(0);
    RecursiveRWPointerLock<TestData> ptr(data);
    
    std::atomic<int> read_count{0};
    std::atomic<int> write_count{0};
    
    std::vector<std::thread> threads;
    
    // Readers
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 10; ++j) {
                auto read = ptr.read();
                int val = read->value;
                read_count.fetch_add(1);
                (void)val;
            }
        });
    }
    
    // Writers
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 5; ++j) {
                auto write = ptr.write();
                write->value++;
                write_count.fetch_add(1);
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(read_count.load() == 50, "Should have 50 reads");
    ASSERT(write_count.load() == 10, "Should have 10 writes");
    
    {
        auto read = ptr.read();
        ASSERT(read->value == 10, "Final value should be 10");
        delete read.get();
    }
    
    return true;
}

bool test_recursive_rw_pointer_nested() {
    TestData data(0);
    RecursiveRWPointerLock<TestData> ptr(&data);
    
    {
        auto write1 = ptr.write();
        write1->value = 10;
        
        {
            auto write2 = ptr.write();  // Nested
            write2->value = 20;
        }
        
        ASSERT(write1->value == 20, "Value should be 20");
    }
    
    {
        auto read = ptr.read();
        ASSERT(read->value == 20, "Final value should be 20");
    }
    
    return true;
}

// ============================================================================
// STRESS TESTS
// ============================================================================

bool test_stress_mutex() {
    MutexLock mutex;
    std::atomic<int> counter{0};
    const int NUM_THREADS = 20;
    const int OPS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < OPS_PER_THREAD; ++j) {
                MutexLock::Guard guard(mutex);
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(counter.load() == NUM_THREADS * OPS_PER_THREAD, "All operations should complete");
    return true;
}

bool test_stress_rwlock() {
    ReadWriteLock rwlock;
    int shared_data = 0;
    std::atomic<int> operations{0};
    
    std::vector<std::thread> threads;
    
    // Many readers
    for (int i = 0; i < 15; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 100; ++j) {
                ReadWriteLock::ReadGuard guard(rwlock);
                int val = shared_data;
                operations.fetch_add(1);
                (void)val;
            }
        });
    }
    
    // Few writers
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 20; ++j) {
                ReadWriteLock::WriteGuard guard(rwlock);
                shared_data++;
                operations.fetch_add(1);
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    ASSERT(shared_data == 100, "All writes should complete");
    ASSERT(operations.load() == 1600, "All operations should complete");
    return true;
}

int main() {
    int passed = 0;
    int failed = 0;
    
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Locks Test Suite                      ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";
    
    std::cout << "MUTEX LOCK TESTS:\n";
    RUN_TEST(test_mutex_basic);
    RUN_TEST(test_mutex_counter);
    RUN_TEST(test_mutex_try_lock);
    RUN_TEST(test_mutex_guard);
    
    std::cout << "\nPOINTER LOCK TESTS:\n";
    RUN_TEST(test_pointer_basic);
    RUN_TEST(test_pointer_store);
    RUN_TEST(test_pointer_cas);
    RUN_TEST(test_pointer_concurrent);
    
    std::cout << "\nREAD-WRITE LOCK TESTS:\n";
    RUN_TEST(test_rwlock_basic);
    RUN_TEST(test_rwlock_multiple_readers);
    RUN_TEST(test_rwlock_writer_exclusion);
    RUN_TEST(test_rwlock_try_locks);
    
    std::cout << "\nRECURSIVE RW LOCK TESTS:\n";
    RUN_TEST(test_recursive_rwlock_basic);
    RUN_TEST(test_recursive_rwlock_recursive_read);
    RUN_TEST(test_recursive_rwlock_recursive_write);
    RUN_TEST(test_recursive_rwlock_nested_locks);
    RUN_TEST(test_recursive_rwlock_writer_can_read);
    
    std::cout << "\nRECURSIVE RW POINTER LOCK TESTS:\n";
    RUN_TEST(test_recursive_rw_pointer_basic);
    RUN_TEST(test_recursive_rw_pointer_version);
    RUN_TEST(test_recursive_rw_pointer_concurrent);
    RUN_TEST(test_recursive_rw_pointer_nested);
    
    std::cout << "\nSTRESS TESTS:\n";
    RUN_TEST(test_stress_mutex);
    RUN_TEST(test_stress_rwlock);
    
    std::cout << "\n════════════════════════════════════════════════\n";
    std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
    std::cout << "════════════════════════════════════════════════\n";
    
    return failed == 0 ? 0 : 1;
}

