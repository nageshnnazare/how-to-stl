#include "tuple/tuple.hpp"
#include <iostream>
#include <string>

void example1_basic() {
    std::cout << "\n=== Example 1: Basic Tuple ===\n";
    
    Tuple<int, double, std::string> t(42, 3.14, "hello");
    
    std::cout << "Element 0: " << get<0>(t) << "\n";
    std::cout << "Element 1: " << get<1>(t) << "\n";
    std::cout << "Element 2: " << get<2>(t) << "\n";
}

void example2_two_elements() {
    std::cout << "\n=== Example 2: Two Elements ===\n";
    
    Tuple<std::string, int> person("Alice", 30);
    
    std::cout << "Name: " << get<0>(person) << "\n";
    std::cout << "Age: " << get<1>(person) << "\n";
}

void example3_return_multiple() {
    std::cout << "\n=== Example 3: Return Multiple Values ===\n";
    
    auto divide = [](int a, int b) -> Tuple<bool, int, std::string> {
        if (b == 0) {
            return Tuple<bool, int, std::string>(false, 0, "Division by zero");
        }
        return Tuple<bool, int, std::string>(true, a / b, "Success");
    };
    
    auto result = divide(10, 2);
    std::cout << "Success: " << get<0>(result) << "\n";
    std::cout << "Result: " << get<1>(result) << "\n";
    std::cout << "Message: " << get<2>(result) << "\n";
}

void example4_coordinates() {
    std::cout << "\n=== Example 4: 3D Coordinates ===\n";
    
    Tuple<double, double, double> point(1.5, 2.5, 3.5);
    
    std::cout << "Point: (" 
              << get<0>(point) << ", " 
              << get<1>(point) << ", " 
              << get<2>(point) << ")\n";
}

void example5_equality() {
    std::cout << "\n=== Example 5: Equality ===\n";
    
    Tuple<int, int> t1(1, 2);
    Tuple<int, int> t2(1, 2);
    Tuple<int, int> t3(2, 1);
    
    std::cout << "t1 == t2: " << (t1 == t2 ? "true" : "false") << "\n";
    std::cout << "t1 == t3: " << (t1 == t3 ? "true" : "false") << "\n";
}

int main() {
    std::cout << "=== Tuple Examples ===\n";
    
    example1_basic();
    example2_two_elements();
    example3_return_multiple();
    example4_coordinates();
    example5_equality();
    
    std::cout << "\n✓ All examples completed!\n";
    return 0;
}
