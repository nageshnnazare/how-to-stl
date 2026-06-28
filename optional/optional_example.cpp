// optional_example.cpp — runnable tour of Optional<T> (maybe-value container)
//
// Demonstrates: disengaged vs engaged states, placement-new lifetime, value_or,
// exceptions from value(), reset, arrow/dereference, copy/move, and config defaults.
// Build from repo root:
//   g++ -std=c++14 -Wall -Wextra -Wpedantic -I. optional/optional_example.cpp -o /tmp/x_optional

#include "optional/optional.hpp"
#include <iostream>
#include <string>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// ---------------------------------------------------------------------------
// Example 1: Disengaged vs engaged — the two states
// ---------------------------------------------------------------------------
// DISENGAGED: has_value_=false, no T in storage_
// ENGAGED:    has_value_=true,  live T from placement new
void example_basic() {
    print_divider("Basic Optional — Two States");

    Optional<int> empty;
    std::cout << "Empty optional has_value: " << empty.has_value() << "\n";
    std::cout << "Empty optional bool:      " << (empty ? "true" : "false") << "\n";

    Optional<int> opt(42);
    std::cout << "Engaged has_value: " << opt.has_value() << "\n";
    std::cout << "Engaged value():   " << opt.value() << "\n";
    std::cout << "Engaged via *:     " << *opt << "\n";

    // Engaged with value zero is STILL engaged
    Optional<int> zero(0);
    std::cout << "Optional(0) is engaged: " << (zero ? "true" : "false")
              << ", value=" << *zero << "\n";
}

// ---------------------------------------------------------------------------
// Example 2: value_or — safe fallback without exceptions
// ---------------------------------------------------------------------------
void example_value_or() {
    print_divider("value_or — Default Values");

    Optional<int> opt1(100);
    Optional<int> opt2;  // disengaged

    std::cout << "opt1.value_or(999): " << opt1.value_or(999) << "\n";
    std::cout << "opt2.value_or(999): " << opt2.value_or(999) << "\n";

    Optional<int> port;  // user did not configure
    int server_port = port.value_or(8080);
    std::cout << "Server port (default): " << server_port << "\n";
}

// ---------------------------------------------------------------------------
// Example 3: value() throws when disengaged
// ---------------------------------------------------------------------------
void example_exceptions() {
    print_divider("Exception Handling");

    Optional<std::string> opt;

    try {
        std::cout << "Calling value() on empty optional...\n";
        std::string val = opt.value();
        std::cout << "Value: " << val << "\n";
    } catch (const std::runtime_error& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }

    if (opt) {
        std::cout << "Has value: " << *opt << "\n";
    } else {
        std::cout << "Guarded check: optional is empty — skip dereference\n";
    }
}

// ---------------------------------------------------------------------------
// Example 4: reset() — explicit ~T() then disengage
// ---------------------------------------------------------------------------
void example_reset() {
    print_divider("Reset and Reassignment");

    Optional<int> opt(123);
    std::cout << "Initial: " << *opt << " (engaged)\n";

    opt.reset();
    std::cout << "After reset, has_value: " << opt.has_value() << "\n";

    opt = Optional<int>(456);
    std::cout << "After reassignment: " << *opt << "\n";
}

// ---------------------------------------------------------------------------
// Example 5: Complex type — visible ctor/dtor in storage_
// ---------------------------------------------------------------------------
struct Person {
    std::string name;
    int age;

    Person(const std::string& n, int a) : name(n), age(a) {
        std::cout << "  Person constructed: " << name << "\n";
    }
    ~Person() {
        std::cout << "  Person destructed: " << name << "\n";
    }
    Person(const Person& other) : name(other.name), age(other.age) {
        std::cout << "  Person copied: " << name << "\n";
    }
};

void example_complex_types() {
    print_divider("Optional with Complex Types");

    {
        Optional<Person> opt(Person("Alice", 30));
        std::cout << "Name: " << opt->name << ", Age: " << opt->age << "\n";
        opt->age = 31;
        std::cout << "Updated age via ->: " << opt->age << "\n";
    }
    std::cout << "Block ended — Optional destructor ran ~Person if still engaged\n";
}

// ---------------------------------------------------------------------------
// Example 6: Function returns — empty means "no result"
// ---------------------------------------------------------------------------
Optional<int> divide(int a, int b) {
    if (b == 0) return Optional<int>();
    return Optional<int>(a / b);
}

Optional<std::string> find_user(int id) {
    if (id == 1) return Optional<std::string>("Alice");
    if (id == 2) return Optional<std::string>("Bob");
    return Optional<std::string>();
}

void example_return_values() {
    print_divider("Function Return Values");

    auto result1 = divide(10, 2);
    if (result1) {
        std::cout << "10 / 2 = " << *result1 << "\n";
    }

    auto result2 = divide(10, 0);
    if (!result2) {
        std::cout << "10 / 0 — no result (disengaged optional)\n";
    }

    std::cout << "User 1: " << find_user(1).value_or("Not found") << "\n";
    std::cout << "User 3: " << find_user(3).value_or("Not found") << "\n";
}

// ---------------------------------------------------------------------------
// Example 7: Copy and move — duplicate or steal engaged value
// ---------------------------------------------------------------------------
void example_copy_move() {
    print_divider("Copy and Move Semantics");

    Optional<int> opt1(100);

    Optional<int> opt2(opt1);
    std::cout << "After copy: opt1=" << *opt1 << ", opt2=" << *opt2 << "\n";

    Optional<int> opt3(std::move(opt1));
    std::cout << "After move: opt3=" << *opt3 << "\n";
    std::cout << "opt1 after move has_value: " << opt1.has_value() << " (source emptied)\n";

    Optional<int> opt4;
    opt4 = opt2;
    std::cout << "Copy assign opt4: " << *opt4 << "\n";
}

// ---------------------------------------------------------------------------
// Example 8: Configuration pattern — unset fields pick defaults
// ---------------------------------------------------------------------------
struct Config {
    Optional<int> port;
    Optional<std::string> host;
    Optional<int> timeout;
};

void example_config_pattern() {
    print_divider("Configuration Pattern");

    Config defaults_only;
    std::cout << "Defaults:\n";
    std::cout << "  host: " << defaults_only.host.value_or("localhost") << "\n";
    std::cout << "  port: " << defaults_only.port.value_or(8080) << "\n";
    std::cout << "  timeout: " << defaults_only.timeout.value_or(30) << "s\n";

    Config partial;
    partial.port = Optional<int>(9000);
    partial.host = Optional<std::string>("api.example.com");

    std::cout << "\nPartial override:\n";
    std::cout << "  host: " << partial.host.value_or("localhost") << "\n";
    std::cout << "  port: " << partial.port.value_or(8080) << "\n";
    std::cout << "  timeout: " << partial.timeout.value_or(30) << "s\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║         Optional Container Examples            ║\n";
    std::cout << "║   Engaged (live T) vs disengaged (empty)       ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    example_basic();
    example_value_or();
    example_exceptions();
    example_reset();
    example_complex_types();
    example_return_values();
    example_copy_move();
    example_config_pattern();

    std::cout << "\n✅ All Optional examples completed successfully!\n";
    return 0;
}

/* ===== EXPECTED OUTPUT (sample run) ============================================
 * Auto-generated by running this program (see tests/README.md).
 * ----------------------------------------------------------------------------
╔════════════════════════════════════════════════╗
║         Optional Container Examples            ║
║   Engaged (live T) vs disengaged (empty)       ║
╚════════════════════════════════════════════════╝

=== Basic Optional — Two States ===
Empty optional has_value: 0
Empty optional bool:      false
Engaged has_value: 1
Engaged value():   42
Engaged via *:     42
Optional(0) is engaged: true, value=0

=== value_or — Default Values ===
opt1.value_or(999): 100
opt2.value_or(999): 999
Server port (default): 8080

=== Exception Handling ===
Calling value() on empty optional...
Caught: Optional::value: no value
Guarded check: optional is empty — skip dereference

=== Reset and Reassignment ===
Initial: 123 (engaged)
After reset, has_value: 0
After reassignment: 456

=== Optional with Complex Types ===
  Person constructed: Alice
  Person copied: Alice
  Person destructed: Alice
Name: Alice, Age: 30
Updated age via ->: 31
  Person destructed: Alice
Block ended — Optional destructor ran ~Person if still engaged

=== Function Return Values ===
10 / 2 = 5
10 / 0 — no result (disengaged optional)
User 1: Alice
User 3: Not found

=== Copy and Move Semantics ===
After copy: opt1=100, opt2=100
After move: opt3=100
opt1 after move has_value: 0 (source emptied)
Copy assign opt4: 100

=== Configuration Pattern ===
Defaults:
  host: localhost
  port: 8080
  timeout: 30s

Partial override:
  host: api.example.com
  port: 9000
  timeout: 30s

✅ All Optional examples completed successfully!
 * ============================================================================ */
