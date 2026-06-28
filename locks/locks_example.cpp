// ============================================================================
//  locks_example.cpp — synchronization primitives under contention
// ============================================================================
//
// Demonstrates:
//   1. MutexLock + Guard     — atomic counter (10 threads × 1000 increments)
//   2. PointerLock           — lock-free stack push/pop with CAS retries
//   3. ReadWriteLock         — many readers + few writers on shared "database"
//   4. RecursiveRWLock       — nested read/write on same thread
//   5. RecursiveRWPointerLock — guarded Account with concurrent deposit/read
//   6. Performance           — MutexLock vs std::mutex micro-benchmark
//
// Build (from repo root):
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -pthread -I. \
//       locks/locks_example.cpp -o /tmp/x_locks
// ============================================================================

#include "locks.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// ============================================================================
// EXAMPLE 1: Mutex Lock - Basic Mutual Exclusion
// ============================================================================
void example_mutex_lock() {
    print_divider("Mutex Lock - Counter Protection");
    
    MutexLock mutex;
    int counter = 0;
    const int NUM_THREADS = 10;
    const int INCREMENTS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    
    auto worker = [&]() {
        for (int i = 0; i < INCREMENTS_PER_THREAD; ++i) {
            MutexLock::Guard guard(mutex);  // RAII lock
            counter++;
        }
    };
    
    std::cout << "Starting " << NUM_THREADS << " threads, each incrementing counter " 
              << INCREMENTS_PER_THREAD << " times\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Final counter: " << counter << "\n";
    std::cout << "Expected: " << (NUM_THREADS * INCREMENTS_PER_THREAD) << "\n";
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Result: " << (counter == NUM_THREADS * INCREMENTS_PER_THREAD ? "✅ CORRECT" : "❌ RACE CONDITION") << "\n";
}

// ============================================================================
// EXAMPLE 2: Pointer Lock - Lock-free Pointer Updates
// ============================================================================
struct Node {
    int value;
    Node* next;
    Node(int v) : value(v), next(nullptr) {}
};

void example_pointer_lock() {
    print_divider("Pointer Lock - Lock-free Stack");
    
    PointerLock<Node> head;
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    
    const int NUM_THREADS = 4;
    const int OPS_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    
    // Pusher threads
    auto pusher = [&](int thread_id) {
        for (int i = 0; i < OPS_PER_THREAD; ++i) {
            Node* new_node = new Node(thread_id * 1000 + i);
            Node* old_head;
            
            do {
                old_head = head.load(std::memory_order_acquire);
                new_node->next = old_head;
            } while (!head.compare_exchange_weak(old_head, new_node));
            
            push_count.fetch_add(1, std::memory_order_relaxed);
        }
    };
    
    // Popper threads
    auto popper = [&]() {
        for (int i = 0; i < OPS_PER_THREAD; ++i) {
            Node* old_head;
            Node* new_head;
            
            do {
                old_head = head.load(std::memory_order_acquire);
                if (!old_head) break;
                new_head = old_head->next;
            } while (old_head && !head.compare_exchange_weak(old_head, new_head));
            
            if (old_head) {
                delete old_head;
                pop_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };
    
    std::cout << "Running lock-free stack with " << NUM_THREADS << " pushers and " 
              << NUM_THREADS << " poppers\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(pusher, i);
    }
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(popper);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Pushes: " << push_count.load() << "\n";
    std::cout << "Pops: " << pop_count.load() << "\n";
    std::cout << "Remaining nodes: " << (push_count.load() - pop_count.load()) << "\n";
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Final version: " << head.version() << "\n";
    
    // Cleanup remaining nodes
    Node* current = head.load();
    while (current) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

// ============================================================================
// EXAMPLE 3: Read-Write Lock - Shared Resource
// ============================================================================
void example_readwrite_lock() {
    print_divider("Read-Write Lock - Database Simulation");
    
    ReadWriteLock rwlock;
    int database_value = 0;
    std::atomic<int> read_count{0};
    std::atomic<int> write_count{0};
    
    const int NUM_READERS = 8;
    const int NUM_WRITERS = 2;
    const int OPERATIONS = 50;
    
    std::vector<std::thread> threads;
    
    // Reader threads
    auto reader = [&](int id) {
        (void)id;  // thread identity is illustrative; not used in the body
        for (int i = 0; i < OPERATIONS; ++i) {
            ReadWriteLock::ReadGuard guard(rwlock);
            int value = database_value;
            read_count.fetch_add(1, std::memory_order_relaxed);
            
            // Simulate read work
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            (void)value;
        }
    };
    
    // Writer threads
    auto writer = [&](int id) {
        (void)id;  // thread identity is illustrative; not used in the body
        for (int i = 0; i < OPERATIONS; ++i) {
            ReadWriteLock::WriteGuard guard(rwlock);
            database_value++;
            write_count.fetch_add(1, std::memory_order_relaxed);
            
            // Simulate write work
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    };
    
    std::cout << "Starting " << NUM_READERS << " readers and " << NUM_WRITERS << " writers\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_READERS; ++i) {
        threads.emplace_back(reader, i);
    }
    
    for (int i = 0; i < NUM_WRITERS; ++i) {
        threads.emplace_back(writer, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Total reads: " << read_count.load() << "\n";
    std::cout << "Total writes: " << write_count.load() << "\n";
    std::cout << "Final database value: " << database_value << "\n";
    std::cout << "Expected: " << (NUM_WRITERS * OPERATIONS) << "\n";
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Result: " << (database_value == NUM_WRITERS * OPERATIONS ? "✅ CORRECT" : "❌ ERROR") << "\n";
}

// ============================================================================
// EXAMPLE 4: Recursive RW Lock - Nested Locking
// ============================================================================
void example_recursive_rwlock() {
    print_divider("Recursive RW Lock - Nested Calls");
    
    RecursiveRWLock rwlock;
    int shared_data = 0;
    
    // Function that acquires read lock and calls another function
    auto read_level_2 = [&]() {
        RecursiveRWLock::ReadGuard guard(rwlock);
        std::cout << "  Level 2 read: " << shared_data << "\n";
    };
    
    auto read_level_1 = [&]() {
        RecursiveRWLock::ReadGuard guard(rwlock);
        std::cout << " Level 1 read: " << shared_data << "\n";
        read_level_2();  // Nested read lock - should work!
    };
    
    // Function that acquires write lock and calls itself recursively
    std::function<void(int)> recursive_write = [&](int depth) {
        RecursiveRWLock::WriteGuard guard(rwlock);
        shared_data++;
        std::cout << " Level " << depth << " write: " << shared_data << "\n";
        
        if (depth < 3) {
            recursive_write(depth + 1);  // Recursive write lock!
        }
    };
    
    std::cout << "Testing nested read locks:\n";
    read_level_1();
    
    std::cout << "\nTesting recursive write locks:\n";
    recursive_write(1);
    
    std::cout << "\nFinal shared_data: " << shared_data << "\n";
    
    // Test with multiple threads
    std::cout << "\nMulti-threaded recursive test:\n";
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&, i]() {
            RecursiveRWLock::WriteGuard guard(rwlock);
            std::cout << "Thread " << i << " acquired write lock\n";
            
            // Nested lock
            {
                RecursiveRWLock::WriteGuard nested(rwlock);
                shared_data += 10;
                std::cout << "Thread " << i << " has nested lock, data = " << shared_data << "\n";
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final value after multi-threaded: " << shared_data << "\n";
}

// ============================================================================
// EXAMPLE 5: Recursive RW Pointer Lock - Protected Pointer with Nesting
// ============================================================================
struct Account {
    std::string name;
    double balance;
    
    Account(const std::string& n, double b) : name(n), balance(b) {}
    
    void deposit(double amount) {
        balance += amount;
    }
    
    bool withdraw(double amount) {
        if (balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }
};

void example_recursive_rw_pointer_lock() {
    print_divider("Recursive RW Pointer Lock - Account Management");
    
    Account* account = new Account("John Doe", 1000.0);
    RecursiveRWPointerLock<Account> locked_account(account);
    
    std::cout << "Initial account:\n";
    {
        auto read_access = locked_account.read();
        std::cout << "  Name: " << read_access->name << "\n";
        std::cout << "  Balance: $" << std::fixed << std::setprecision(2) 
                  << read_access->balance << "\n";
        std::cout << "  Version: " << read_access.version() << "\n";
    }
    
    // Multiple readers simultaneously
    std::vector<std::thread> readers;
    std::atomic<int> read_ops{0};
    
    for (int i = 0; i < 5; ++i) {
        readers.emplace_back([&]() {
            for (int j = 0; j < 10; ++j) {
                auto read_access = locked_account.read();
                double balance = read_access->balance;
                read_ops.fetch_add(1);
                
                // Nested read (recursive!)
                {
                    auto nested_read = locked_account.read();
                    (void)nested_read->name;
                }
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                (void)balance;
            }
        });
    }
    
    // Writer threads
    std::vector<std::thread> writers;
    std::atomic<int> write_ops{0};
    
    for (int i = 0; i < 2; ++i) {
        writers.emplace_back([&]() {
            for (int j = 0; j < 5; ++j) {
                auto write_access = locked_account.write();
                write_access->deposit(10.0);
                write_ops.fetch_add(1);
                
                // Nested write (recursive!)
                {
                    auto nested_write = locked_account.write();
                    nested_write->deposit(5.0);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    for (auto& t : readers) t.join();
    for (auto& t : writers) t.join();
    
    std::cout << "\nAfter concurrent operations:\n";
    {
        auto read_access = locked_account.read();
        std::cout << "  Name: " << read_access->name << "\n";
        std::cout << "  Balance: $" << read_access->balance << "\n";
        std::cout << "  Version: " << read_access.version() << "\n";
    }
    
    std::cout << "\nStatistics:\n";
    std::cout << "  Read operations: " << read_ops.load() << "\n";
    std::cout << "  Write operations: " << write_ops.load() << "\n";
    std::cout << "  Expected balance: $" << (1000.0 + write_ops.load() * 15.0) << "\n";
    
    // Update pointer
    std::cout << "\nUpdating account pointer:\n";
    Account* new_account = new Account("Jane Smith", 5000.0);
    {
        auto write_access = locked_account.write();
        delete account;
        write_access.set(new_account);
        std::cout << "  New account: " << write_access->name << "\n";
        std::cout << "  New balance: $" << write_access->balance << "\n";
        std::cout << "  New version: " << write_access.version() << "\n";
    }
    
    // Cleanup
    {
        auto write_access = locked_account.write();
        delete write_access.get();
        write_access.set(nullptr);
    }
}

// ============================================================================
// EXAMPLE 6: Performance Comparison
// ============================================================================
void example_performance() {
    print_divider("Performance Comparison");
    
    const int NUM_THREADS = 4;
    const int OPERATIONS = 10000;
    
    // Test MutexLock
    {
        MutexLock mutex;
        int counter = 0;
        std::vector<std::thread> threads;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < OPERATIONS; ++j) {
                    MutexLock::Guard guard(mutex);
                    counter++;
                }
            });
        }
        
        for (auto& t : threads) t.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "MutexLock:\n";
        std::cout << "  Time: " << duration.count() << " μs\n";
        std::cout << "  Ops/sec: " << ((NUM_THREADS * OPERATIONS * 1000000LL) / duration.count()) << "\n";
    }
    
    // Test std::mutex (baseline)
    {
        std::mutex mutex;
        int counter = 0;
        std::vector<std::thread> threads;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < OPERATIONS; ++j) {
                    std::lock_guard<std::mutex> guard(mutex);
                    counter++;
                }
            });
        }
        
        for (auto& t : threads) t.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "\nstd::mutex (baseline):\n";
        std::cout << "  Time: " << duration.count() << " μs\n";
        std::cout << "  Ops/sec: " << ((NUM_THREADS * OPERATIONS * 1000000LL) / duration.count()) << "\n";
    }
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║          Custom Lock Examples                  ║\n";
    std::cout << "║  Mutex, Pointer, RW, Recursive Locks           ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";
    
    example_mutex_lock();
    example_pointer_lock();
    example_readwrite_lock();
    example_recursive_rwlock();
    example_recursive_rw_pointer_lock();
    example_performance();
    
    std::cout << "\n✅ All lock examples completed successfully!\n";
    return 0;
}


/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * NOTE: with threads, line ordering / counts may vary between runs.
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║          Custom Lock Examples                  ║
║  Mutex, Pointer, RW, Recursive Locks           ║
╚════════════════════════════════════════════════╝

=== Mutex Lock - Counter Protection ===
Starting 10 threads, each incrementing counter 1000 times
Final counter: 10000
Expected: 10000
Time: 0 ms
Result: ✅ CORRECT

=== Pointer Lock - Lock-free Stack ===
Running lock-free stack with 4 pushers and 4 poppers
Pushes: 400
Pops: 400
Remaining nodes: 0
Time: 0 ms
Final version: 800

=== Read-Write Lock - Database Simulation ===
Starting 8 readers and 2 writers
Total reads: 400
Total writes: 100
Final database value: 100
Expected: 100
Time: 9 ms
Result: ✅ CORRECT

=== Recursive RW Lock - Nested Calls ===
Testing nested read locks:
 Level 1 read: 0
  Level 2 read: 0

Testing recursive write locks:
 Level 1 write: 1
 Level 2 write: 2
 Level 3 write: 3

Final shared_data: 3

Multi-threaded recursive test:
Thread 0 acquired write lock
Thread 0 has nested lock, data = 13
Thread 1 acquired write lock
Thread 1 has nested lock, data = 23
Thread 2 acquired write lock
Thread 2 has nested lock, data = 33
Final value after multi-threaded: 33

=== Recursive RW Pointer Lock - Account Management ===
Initial account:
  Name: John Doe
  Balance: $1000.00
  Version: 0

After concurrent operations:
  Name: John Doe
  Balance: $1150.00
  Version: 0

Statistics:
  Read operations: 50
  Write operations: 10
  Expected balance: $1150.00

Updating account pointer:
  New account: Jane Smith
  New balance: $5000.00
  New version: 1

=== Performance Comparison ===
MutexLock:
  Time: 1298 μs
  Ops/sec: 30816640

std::mutex (baseline):
  Time: 2672 μs
  Ops/sec: 14970059

✅ All lock examples completed successfully!
 * ============================================================================ */
