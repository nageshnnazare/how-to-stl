/**
 * @file map_example.cpp
 * @brief Runnable tour of Map<Key,T> — ordered unique keys on a Red-Black tree.
 *
 * Demonstrates operator[], at(), insert, find/contains, erase, initializer-list
 * construction, and in-order iteration (sorted by key, not insertion order).
 */

#include "map/map.hpp"
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Section 1: operator[] creates or updates entries (default-inserts on miss)
// ---------------------------------------------------------------------------
void example_subscript() {
    std::cout << "\n=== 1. operator[] — find or default-insert ===\n";

    Map<std::string, int> ages;
    ages["Alice"] = 30;   // insert {"Alice", 0} then assign 30
    ages["Bob"] = 25;
    ages["Charlie"] = 35;

    std::cout << "After assignments (iteration is key-sorted):\n";
    for (const auto& p : ages) {
        std::cout << "  " << p.first << ": " << p.second << "\n";
    }
}

// ---------------------------------------------------------------------------
// Section 2: at() — bounds-checked read; throws if key missing
// ---------------------------------------------------------------------------
void example_at() {
    std::cout << "\n=== 2. at() — checked access ===\n";

    Map<std::string, int> ages;
    ages["Alice"] = 30;

    std::cout << "Alice via at(): " << ages.at("Alice") << "\n";

    try {
        (void)ages.at("Nobody");
    } catch (const std::exception& e) {
        std::cout << "at(\"Nobody\") threw: " << e.what() << "\n";
    }
}

// ---------------------------------------------------------------------------
// Section 3: insert returns {iterator, bool}; duplicate key does not overwrite
// ---------------------------------------------------------------------------
void example_insert() {
    std::cout << "\n=== 3. insert — no silent overwrite on duplicate key ===\n";

    Map<int, std::string> items;
    auto result1 = items.insert({1, "one"});
    auto result2 = items.insert({1, "ONE"});  // same key — rejected

    std::cout << "First insert ok=" << result1.second << ", second ok=" << result2.second << "\n";
    std::cout << "Value at key 1: " << result1.first->second << " (still \"one\")\n";
}

// ---------------------------------------------------------------------------
// Section 4: Lookup — find, contains, count
// ---------------------------------------------------------------------------
void example_lookup() {
    std::cout << "\n=== 4. find / contains / count ===\n";

    Map<std::string, int> scores = {{"Alice", 90}, {"Bob", 85}};

    if (scores.contains("Bob")) {
        auto it = scores.find("Bob");
        std::cout << "Bob's score: " << it->second << "\n";
    }
    std::cout << "Count Alice: " << scores.count("Alice") << "\n";
    std::cout << "Count Eve: " << scores.count("Eve") << "\n";
}

// ---------------------------------------------------------------------------
// Section 5: Erase by key — BST delete + rebalance
// ---------------------------------------------------------------------------
void example_erase() {
    std::cout << "\n=== 5. erase ===\n";

    Map<std::string, int> ages;
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    ages["Charlie"] = 35;

    std::cout << "Size before erase Bob: " << ages.size() << "\n";
    ages.erase("Bob");
    std::cout << "Size after: " << ages.size() << "\n";

    std::cout << "Remaining keys: ";
    for (const auto& p : ages) {
        std::cout << p.first << " ";
    }
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Section 6: Initializer list — bulk insert in arbitrary order, sorted by key
// ---------------------------------------------------------------------------
void example_initializer_list() {
    std::cout << "\n=== 6. Initializer list (sorted key order when printed) ===\n";

    Map<int, std::string> items = {{3, "three"}, {1, "one"}, {2, "two"}};

    for (const auto& p : items) {
        std::cout << "  " << p.first << " -> " << p.second << "\n";
    }
}

// ---------------------------------------------------------------------------
// Section 7: Copy and move — deep clone vs O(1) steal
// ---------------------------------------------------------------------------
void example_copy_move() {
    std::cout << "\n=== 7. Copy and move ===\n";

    Map<int, int> m1 = {{10, 100}, {20, 200}};
    Map<int, int> m2 = m1;

    std::cout << "Copy size: " << m2.size() << ", key 10 -> " << m2[10] << "\n";

    Map<int, int> m3 = std::move(m1);
    std::cout << "After move: m3 size=" << m3.size()
              << ", m1 size=" << m1.size() << " (emptied)\n";
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "          Map Class Examples                    \n";
    std::cout << "=================================================\n";

    try {
        example_subscript();
        example_at();
        example_insert();
        example_lookup();
        example_erase();
        example_initializer_list();
        example_copy_move();

        std::cout << "\n=================================================\n";
        std::cout << "   All examples completed successfully!        \n";
        std::cout << "=================================================\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
=================================================
          Map Class Examples                    
=================================================

=== 1. operator[] — find or default-insert ===
After assignments (iteration is key-sorted):
  Alice: 30
  Bob: 25
  Charlie: 35

=== 2. at() — checked access ===
Alice via at(): 30
at("Nobody") threw: Map::at: key not found

=== 3. insert — no silent overwrite on duplicate key ===
First insert ok=1, second ok=0
Value at key 1: one (still "one")

=== 4. find / contains / count ===
Bob's score: 85
Count Alice: 1
Count Eve: 0

=== 5. erase ===
Size before erase Bob: 3
Size after: 2
Remaining keys: Alice Charlie 

=== 6. Initializer list (sorted key order when printed) ===
  1 -> one
  2 -> two
  3 -> three

=== 7. Copy and move ===
Copy size: 2, key 10 -> 100
After move: m3 size=2, m1 size=0 (emptied)

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
