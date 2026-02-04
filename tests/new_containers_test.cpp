#include "list/list.hpp"
#include "pair/pair.hpp"
#include "array/array.hpp"
#include "optional/optional.hpp"
#include "bitset/bitset.hpp"
#include <iostream>

int main() {
    std::cout << "=== Testing New Containers ===\n\n";
    
    // Test List
    std::cout << "Testing List... ";
    {
        List<int> l;
        l.push_back(1);
        l.push_back(2);
        l.push_front(0);
        if (l.front() != 0 || l.back() != 2 || l.size() != 3) {
            std::cout << "FAILED\n"; return 1;
        }
    }
    std::cout << "PASSED\n";
    
    // Test Pair
    std::cout << "Testing Pair... ";
    {
        Pair<int, std::string> p(42, "hello");
        if (p.first != 42 || p.second != "hello") {
            std::cout << "FAILED\n"; return 1;
        }
        auto p2 = make_pair(1, 2.5);
        if (p2.first != 1 || p2.second != 2.5) {
            std::cout << "FAILED\n"; return 1;
        }
    }
    std::cout << "PASSED\n";
    
    // Test Array
    std::cout << "Testing Array... ";
    {
        Array<int, 5> arr = {1, 2, 3, 4, 5};
        if (arr[0] != 1 || arr.size() != 5) {
            std::cout << "FAILED\n"; return 1;
        }
        arr.fill(7);
        if (arr[0] != 7) {
            std::cout << "FAILED\n"; return 1;
        }
    }
    std::cout << "PASSED\n";
    
    // Test Optional
    std::cout << "Testing Optional... ";
    {
        Optional<int> opt1;
        if (opt1.has_value()) {
            std::cout << "FAILED\n"; return 1;
        }
        
        Optional<int> opt2(42);
        if (!opt2.has_value() || opt2.value() != 42) {
            std::cout << "FAILED\n"; return 1;
        }
        
        if (opt1.value_or(10) != 10) {
            std::cout << "FAILED\n"; return 1;
        }
    }
    std::cout << "PASSED\n";
    
    // Test Bitset
    std::cout << "Testing Bitset... ";
    {
        Bitset<8> bs;
        bs.set(0);
        bs.set(3);
        bs.set(7);
        
        if (!bs.test(0) || !bs.test(3) || !bs.test(7)) {
            std::cout << "FAILED\n"; return 1;
        }
        if (bs.count() != 3) {
            std::cout << "FAILED\n"; return 1;
        }
        
        bs.flip(0);
        if (bs.test(0)) {
            std::cout << "FAILED\n"; return 1;
        }
    }
    std::cout << "PASSED\n";
    
    std::cout << "\n✅ All 5 new containers PASSED!\n";
    return 0;
}
