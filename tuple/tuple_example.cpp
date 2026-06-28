// tuple_example.cpp — runnable tour of Tuple (2- and 3-element specializations)
//
// Demonstrates: get<I>(), make_tuple, member vs free get, multi-value returns,
// equality, and compile-time index dispatch. Build from repo root:
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -I. tuple/tuple_example.cpp -o /tmp/x_tuple

#include "tuple/tuple.hpp"
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Example 1: Three-element tuple and index access
// ---------------------------------------------------------------------------
// get<0>, get<1>, get<2> resolve at compile time to first_, second_, third_.
void example1_basic() {
    std::cout << "\n=== Example 1: Basic 3-Tuple ===\n";

    Tuple<int, double, std::string> t(42, 3.14, "hello");

    std::cout << "Element 0 (int):    " << get<0>(t) << "\n";
    std::cout << "Element 1 (double): " << get<1>(t) << "\n";
    std::cout << "Element 2 (string): " << get<2>(t) << "\n";

    // Mutate through reference returned by get
    get<0>(t) = 99;
    get<2>(t) = "world";
    std::cout << "After mutation: " << get<0>(t) << ", " << get<2>(t) << "\n";

    // Member syntax is equivalent
    std::cout << "Via t.get<1>(): " << t.get<1>() << "\n";
}

// ---------------------------------------------------------------------------
// Example 2: Two-element tuple
// ---------------------------------------------------------------------------
void example2_two_elements() {
    std::cout << "\n=== Example 2: Two Elements ===\n";

    Tuple<std::string, int> person("Alice", 30);

    std::cout << "Name: " << get<0>(person) << "\n";
    std::cout << "Age:  " << get<1>(person) << "\n";

    auto duo = make_tuple(std::string("Bob"), 25);
    std::cout << "make_tuple: " << get<0>(duo) << ", age " << get<1>(duo) << "\n";
}

// ---------------------------------------------------------------------------
// Example 3: Return multiple values (success flag + payload + message)
// ---------------------------------------------------------------------------
Tuple<bool, int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Tuple<bool, int, std::string>(false, 0, "Division by zero");
    }
    return Tuple<bool, int, std::string>(true, a / b, "Success");
}

void example3_return_multiple() {
    std::cout << "\n=== Example 3: Return Multiple Values ===\n";

    auto ok = divide(10, 2);
    std::cout << "10/2 — success: " << get<0>(ok)
              << ", result: " << get<1>(ok)
              << ", msg: " << get<2>(ok) << "\n";

    auto bad = divide(10, 0);
    std::cout << "10/0 — success: " << get<0>(bad)
              << ", msg: " << get<2>(bad) << "\n";
}

// ---------------------------------------------------------------------------
// Example 4: 3D coordinates
// ---------------------------------------------------------------------------
void example4_coordinates() {
    std::cout << "\n=== Example 4: 3D Coordinates ===\n";

    Tuple<double, double, double> point(1.5, 2.5, 3.5);

    std::cout << "Point: ("
              << get<0>(point) << ", "
              << get<1>(point) << ", "
              << get<2>(point) << ")\n";

    // make_tuple with literals
    auto tip = make_tuple(0.0, 0.0, 1.0);
    std::cout << "Unit Z: (" << get<0>(tip) << ", " << get<1>(tip) << ", " << get<2>(tip) << ")\n";
}

// ---------------------------------------------------------------------------
// Example 5: Equality (element-wise)
// ---------------------------------------------------------------------------
void example5_equality() {
    std::cout << "\n=== Example 5: Equality ===\n";

    Tuple<int, int> t1(1, 2);
    Tuple<int, int> t2(1, 2);
    Tuple<int, int> t3(2, 1);

    std::cout << "t1 == t2: " << (t1 == t2 ? "true" : "false") << "\n";
    std::cout << "t1 == t3: " << (t1 == t3 ? "true" : "false") << "\n";
}

// ---------------------------------------------------------------------------
// Example 6: How get<I> dispatch works (conceptual)
// ---------------------------------------------------------------------------
// At compile time, get<1>(t) becomes a direct reference to second_ — no runtime index.
void example6_compile_time_index() {
    std::cout << "\n=== Example 6: Compile-Time Index (conceptual) ===\n";

    Tuple<char, int, double> t('A', 7, 2.5);

    // Each get<I> is a separate compile-time branch (if constexpr in tuple.hpp)
    char& tag = get<0>(t);
    int& count = get<1>(t);
    double& ratio = get<2>(t);

    tag = 'B';
    count = 8;
    std::cout << "Updated: " << tag << ", " << count << ", " << ratio << "\n";

    // get<3>(t) would NOT compile on a 3-tuple — try it and see the error.
    std::cout << "(get<3> on 3-tuple is a compile-time error — index must match arity)\n";
}

// ---------------------------------------------------------------------------
// Example 7: Structured bindings (C++17) on tuple-like aggregates
// ---------------------------------------------------------------------------
#if __cplusplus >= 201703L
void example7_structured_bindings() {
    std::cout << "\n=== Example 7: Structured Bindings (C++17) ===\n";

    auto result = divide(20, 4);
    auto [success, value, message] = result;
    std::cout << "Bound: success=" << success << ", value=" << value
              << ", message=" << message << "\n";
}
#endif

// ---------------------------------------------------------------------------
// Example 8: Copy semantics — each element copied
// ---------------------------------------------------------------------------
void example8_copy() {
    std::cout << "\n=== Example 8: Copy ===\n";

    Tuple<std::string, int> a("copy-me", 1);
    Tuple<std::string, int> b = a;

    get<0>(b) = "modified";
    std::cout << "a.first: " << get<0>(a) << " (unchanged)\n";
    std::cout << "b.first: " << get<0>(b) << "\n";
}

int main() {
    std::cout << "=== Tuple Examples ===\n";
    std::cout << "Fixed heterogeneous product; index access via get<I>()\n";

    example1_basic();
    example2_two_elements();
    example3_return_multiple();
    example4_coordinates();
    example5_equality();
    example6_compile_time_index();
#if __cplusplus >= 201703L
    example7_structured_bindings();
#endif
    example8_copy();

    std::cout << "\n✓ All examples completed!\n";
    return 0;
}

/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
=== Tuple Examples ===
Fixed heterogeneous product; index access via get<I>()

=== Example 1: Basic 3-Tuple ===
Element 0 (int):    42
Element 1 (double): 3.14
Element 2 (string): hello
After mutation: 99, world
Via t.get<1>(): 3.14

=== Example 2: Two Elements ===
Name: Alice
Age:  30
make_tuple: Bob, age 25

=== Example 3: Return Multiple Values ===
10/2 — success: 1, result: 5, msg: Success
10/0 — success: 0, msg: Division by zero

=== Example 4: 3D Coordinates ===
Point: (1.5, 2.5, 3.5)
Unit Z: (0, 0, 1)

=== Example 5: Equality ===
t1 == t2: true
t1 == t3: false

=== Example 6: Compile-Time Index (conceptual) ===
Updated: B, 8, 2.5
(get<3> on 3-tuple is a compile-time error — index must match arity)

=== Example 8: Copy ===
a.first: copy-me (unchanged)
b.first: modified

✓ All examples completed!
 * ============================================================================ */
