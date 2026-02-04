#include "optional.hpp"
#include <iostream>
#include <string>

void print_divider(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}

// Example 1: Basic Optional Usage
void example_basic() {
    print_divider("Basic Optional Usage");
    
    // Empty optional
    Optional<int> empty;
    std::cout << "Empty optional has_value: " << empty.has_value() << "\n";
    std::cout << "Empty optional bool: " << (empty ? "true" : "false") << "\n";
    
    // Optional with value
    Optional<int> opt(42);
    std::cout << "opt has_value: " << opt.has_value() << "\n";
    std::cout << "opt value: " << opt.value() << "\n";
    std::cout << "opt via *: " << *opt << "\n";
}

// Example 2: value_or for Default Values
void example_value_or() {
    print_divider("value_or - Default Values");
    
    Optional<int> opt1(100);
    Optional<int> opt2;
    
    std::cout << "opt1.value_or(999): " << opt1.value_or(999) << "\n";
    std::cout << "opt2.value_or(999): " << opt2.value_or(999) << "\n";
    
    // Practical use case - configuration settings
    Optional<int> port;  // Not configured
    int server_port = port.value_or(8080);  // Use default 8080
    std::cout << "Server port: " << server_port << "\n";
}

// Example 3: Exception Handling
void example_exceptions() {
    print_divider("Exception Handling");
    
    Optional<std::string> opt;
    
    try {
        std::cout << "Accessing empty optional...\n";
        std::string val = opt.value();
        std::cout << "Value: " << val << "\n";
    } catch (const std::runtime_error& e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
    
    // Safe check before access
    if (opt) {
        std::cout << "Has value: " << *opt << "\n";
    } else {
        std::cout << "Optional is empty\n";
    }
}

// Example 4: Reset and Reassignment
void example_reset() {
    print_divider("Reset and Reassignment");
    
    Optional<int> opt(123);
    std::cout << "Initial value: " << *opt << "\n";
    
    // Reset to empty
    opt.reset();
    std::cout << "After reset, has_value: " << opt.has_value() << "\n";
    
    // Reassign
    opt = Optional<int>(456);
    std::cout << "After reassignment: " << *opt << "\n";
}

// Example 5: Optional with Complex Types
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
        
        // Modify through ->
        opt->age = 31;
        std::cout << "Updated age: " << opt->age << "\n";
    }
    std::cout << "Optional destroyed\n";
}

// Example 6: Function Return Values
Optional<int> divide(int a, int b) {
    if (b == 0) {
        return Optional<int>();  // Return empty optional
    }
    return Optional<int>(a / b);
}

Optional<std::string> find_user(int id) {
    if (id == 1) return Optional<std::string>("Alice");
    if (id == 2) return Optional<std::string>("Bob");
    return Optional<std::string>();  // User not found
}

void example_return_values() {
    print_divider("Function Return Values");
    
    // Safe division
    auto result1 = divide(10, 2);
    if (result1) {
        std::cout << "10 / 2 = " << *result1 << "\n";
    }
    
    auto result2 = divide(10, 0);
    if (result2) {
        std::cout << "10 / 0 = " << *result2 << "\n";
    } else {
        std::cout << "Division by zero - no result\n";
    }
    
    // User lookup
    auto user1 = find_user(1);
    std::cout << "User 1: " << user1.value_or("Not found") << "\n";
    
    auto user3 = find_user(3);
    std::cout << "User 3: " << user3.value_or("Not found") << "\n";
}

// Example 7: Copy and Move Semantics
void example_copy_move() {
    print_divider("Copy and Move Semantics");
    
    Optional<int> opt1(100);
    
    // Copy construction
    Optional<int> opt2(opt1);
    std::cout << "opt1: " << *opt1 << ", opt2: " << *opt2 << "\n";
    
    // Move construction
    Optional<int> opt3(std::move(opt1));
    std::cout << "opt3: " << *opt3 << "\n";
    std::cout << "opt1 after move has_value: " << opt1.has_value() << "\n";
    
    // Copy assignment
    Optional<int> opt4;
    opt4 = opt2;
    std::cout << "opt4 after copy: " << *opt4 << "\n";
}

// Example 8: Chaining with value_or
struct Config {
    Optional<int> port;
    Optional<std::string> host;
    Optional<int> timeout;
};

void example_config_pattern() {
    print_divider("Configuration Pattern");
    
    Config config;
    // User didn't configure these values
    
    // Use defaults
    int port = config.port.value_or(8080);
    std::string host = config.host.value_or("localhost");
    int timeout = config.timeout.value_or(30);
    
    std::cout << "Server config:\n";
    std::cout << "  Host: " << host << "\n";
    std::cout << "  Port: " << port << "\n";
    std::cout << "  Timeout: " << timeout << "s\n";
    
    // User provides some config
    Config config2;
    config2.port = Optional<int>(9000);
    config2.host = Optional<std::string>("api.example.com");
    
    port = config2.port.value_or(8080);
    host = config2.host.value_or("localhost");
    timeout = config2.timeout.value_or(30);
    
    std::cout << "\nUser-configured server:\n";
    std::cout << "  Host: " << host << "\n";
    std::cout << "  Port: " << port << "\n";
    std::cout << "  Timeout: " << timeout << "s\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║         Optional Container Examples            ║\n";
    std::cout << "║        Maybe-value Type (std::optional)        ║\n";
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

