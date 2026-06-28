// pair_example.cpp — runnable tour of Pair<T1,T2>
//
// Demonstrates: default/copy/move construction, make_pair, structured bindings,
// lexicographic comparison, multi-value returns, and map-style key-value entries.
// Build from repo root:
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -I. pair/pair_example.cpp -o /tmp/x_pair

#include "pair/pair.hpp"
#include <iostream>
#include <string>
#include <utility>   // std::move (C++17 structured bindings if available)

// ---------------------------------------------------------------------------
// Example 1: Basic construction and named member access
// ---------------------------------------------------------------------------
// Pair is an aggregate with public first/second — no getters, no heap.
void example1_basic() {
    std::cout << "\n=== Example 1: Basic Pair ===\n";

    Pair<int, std::string> p(42, "answer");
    std::cout << "Pair: (" << p.first << ", " << p.second << ")\n";

    // Members are mutable
    p.first = 100;
    p.second = "century";
    std::cout << "After mutation: (" << p.first << ", " << p.second << ")\n";

    // Default construction value-initializes both members
    Pair<int, int> zeroed;
    std::cout << "Default Pair<int,int>: (" << zeroed.first << ", " << zeroed.second << ")\n";
}

// ---------------------------------------------------------------------------
// Example 2: make_pair and type deduction
// ---------------------------------------------------------------------------
// make_pair perfect-forwards arguments so rvalues move instead of copy.
void example2_make_pair() {
    std::cout << "\n=== Example 2: make_pair Helper ===\n";

    // Explicit type — same as make_pair here, but longer to write
    Pair<double, std::string> p1(3.14, "pi");
    std::cout << "Explicit: " << p1.first << " = " << p1.second << "\n";

    // Factory equivalent: make_pair(2.718, std::string("e"))
    // (unqualified make_pair is ambiguous with std::make_pair from <utility>)
    Pair<double, std::string> p2(2.718, std::string("e"));
    std::cout << "Pair factory style: " << p2.first << " = " << p2.second << "\n";

    // Move a heavy string into the pair
    std::string big = "a long label that owns a heap buffer";
    Pair<int, std::string> p3(1, std::move(big));
    std::cout << "Moved string into pair; source now: '" << big << "'\n";
    std::cout << "Pair.second: " << p3.second << "\n";
}

// ---------------------------------------------------------------------------
// Example 3: Coordinates and other named pairs
// ---------------------------------------------------------------------------
void example3_coordinates() {
    std::cout << "\n=== Example 3: Coordinates ===\n";

    Pair<double, double> point(10.5, 20.3);
    std::cout << "Point: (" << point.first << ", " << point.second << ")\n";

    // Screen space: (x, y) as first/second
    auto origin = Pair<double, double>(0.0, 0.0);
    std::cout << "Origin: (" << origin.first << ", " << origin.second << ")\n";
}

// ---------------------------------------------------------------------------
// Example 4: Key-value entries (how Map thinks about data)
// ---------------------------------------------------------------------------
void example4_key_value() {
    std::cout << "\n=== Example 4: Key-Value Store ===\n";

    Pair<std::string, int> entry("age", 25);
    std::cout << entry.first << " = " << entry.second << "\n";

    // Multiple entries in a vector would be sorted by Pair::operator<
    // lexicographically: key first, then value on tie.
    Pair<std::string, int> a{"apple", 3};
    Pair<std::string, int> b{"banana", 1};
    std::cout << "a < b (by key): " << (a < b ? "true" : "false") << "\n";
}

// ---------------------------------------------------------------------------
// Example 5: Lexicographic comparisons
// ---------------------------------------------------------------------------
// operator< compares first, then second on tie — same rule as std::pair.
void example5_comparisons() {
    std::cout << "\n=== Example 5: Comparisons ===\n";

    Pair<int, int> p1(1, 2);
    Pair<int, int> p2(1, 2);
    Pair<int, int> p3(2, 1);
    Pair<int, int> p4(1, 9);

    std::cout << "p1 == p2: " << (p1 == p2 ? "true" : "false") << "\n";
    std::cout << "p1 != p3: " << (p1 != p3 ? "true" : "false") << "\n";
    std::cout << "p1 < p3:  " << (p1 < p3 ? "true" : "false")   // 1 < 2
              << "  (first column decides)\n";
    std::cout << "p1 < p4:  " << (p1 < p4 ? "true" : "false")   // 1==1, 2 < 9
              << "  (second column breaks tie)\n";
}

// ---------------------------------------------------------------------------
// Example 6: Returning multiple values from a function
// ---------------------------------------------------------------------------
Pair<bool, int> safe_divide(int a, int b) {
    if (b == 0) return Pair<bool, int>{false, 0};
    return Pair<bool, int>{true, a / b};
}

void example6_return_multiple() {
    std::cout << "\n=== Example 6: Return Multiple Values ===\n";

    auto ok_result = safe_divide(10, 2);
    if (ok_result.first) {
        std::cout << "10 / 2 = " << ok_result.second << "\n";
    }

    auto fail = safe_divide(10, 0);
    std::cout << "10 / 0 success? " << (fail.first ? "yes" : "no") << "\n";
}

// ---------------------------------------------------------------------------
// Example 7: Structured bindings (C++17)
// ---------------------------------------------------------------------------
// Works because first and second are public members.
#if __cplusplus >= 201703L
void example7_structured_bindings() {
    std::cout << "\n=== Example 7: Structured Bindings (C++17) ===\n";

    Pair<std::string, int> p("mode", 42);
    auto [name, code] = p;
    std::cout << "Decomposed: name=" << name << ", code=" << code << "\n";

    auto [ok, quotient] = safe_divide(20, 4);
    std::cout << "Divide: ok=" << ok << ", quotient=" << quotient << "\n";
}
#endif

// ---------------------------------------------------------------------------
// Example 8: Copy and move semantics
// ---------------------------------------------------------------------------
void example8_copy_move() {
    std::cout << "\n=== Example 8: Copy and Move ===\n";

    Pair<std::string, int> src("original", 1);
    Pair<std::string, int> copied = src;              // copy both members
    Pair<std::string, int> moved = std::move(src);    // move both members

    std::cout << "Copied.second: " << copied.second << " (" << copied.first << ")\n";
    std::cout << "Moved.second:  " << moved.second << " (" << moved.first << ")\n";
    std::cout << "Source after move.first: '" << src.first << "' (valid but unspecified)\n";
}

int main() {
    std::cout << "=== Pair Examples ===\n";
    std::cout << "Heterogeneous two-tuple: public first/second, lexicographic order\n";

    example1_basic();
    example2_make_pair();
    example3_coordinates();
    example4_key_value();
    example5_comparisons();
    example6_return_multiple();
#if __cplusplus >= 201703L
    example7_structured_bindings();
#endif
    example8_copy_move();

    std::cout << "\n✓ All examples completed!\n";
    return 0;
}

/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
=== Pair Examples ===
Heterogeneous two-tuple: public first/second, lexicographic order

=== Example 1: Basic Pair ===
Pair: (42, answer)
After mutation: (100, century)
Default Pair<int,int>: (0, 0)

=== Example 2: make_pair Helper ===
Explicit: 3.14 = pi
Pair factory style: 2.718 = e
Moved string into pair; source now: ''
Pair.second: a long label that owns a heap buffer

=== Example 3: Coordinates ===
Point: (10.5, 20.3)
Origin: (0, 0)

=== Example 4: Key-Value Store ===
age = 25
a < b (by key): true

=== Example 5: Comparisons ===
p1 == p2: true
p1 != p3: true
p1 < p3:  true  (first column decides)
p1 < p4:  true  (second column breaks tie)

=== Example 6: Return Multiple Values ===
10 / 2 = 5
10 / 0 success? no

=== Example 8: Copy and Move ===
Copied.second: 1 (original)
Moved.second:  1 (original)
Source after move.first: '' (valid but unspecified)

✓ All examples completed!
 * ============================================================================ */
