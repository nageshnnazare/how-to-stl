#include "tuple/tuple.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== Tuple Test Suite ===\n\n";
    
    int passed = 0, total = 10;
    
    std::cout << "Test 1: Two elements... ";
    { 
        Tuple<int, std::string> t(1, "one"); 
        if (get<0>(t) == 1 && get<1>(t) == "one") { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 2: Three elements... ";
    { 
        Tuple<int, double, std::string> t(1, 2.5, "test"); 
        if (get<0>(t) == 1 && get<1>(t) == 2.5 && get<2>(t) == "test") { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 3: Modify element... ";
    { 
        Tuple<int, int> t(1, 2); 
        get<0>(t) = 10; 
        if (get<0>(t) == 10) { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 4: Equality 2... ";
    { 
        Tuple<int, int> t1(1, 2); 
        Tuple<int, int> t2(1, 2); 
        if (t1 == t2) { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 5: Equality 3... ";
    { 
        Tuple<int, int, int> t1(1, 2, 3); 
        Tuple<int, int, int> t2(1, 2, 3); 
        if (t1 == t2) { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 6: Copy... ";
    { 
        Tuple<int, int> t1(1, 2); 
        Tuple<int, int> t2 = t1; 
        if (get<0>(t2) == 1 && get<1>(t2) == 2) { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 7: Move... ";
    { 
        Tuple<std::string, int> t1("test", 10); 
        Tuple<std::string, int> t2 = std::move(t1); 
        if (get<0>(t2) == "test") { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 8: Default construct... ";
    { 
        Tuple<int, int> t; 
        if (get<0>(t) == 0 && get<1>(t) == 0) { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 9: String tuple... ";
    { 
        Tuple<std::string, std::string> t("hello", "world"); 
        if (get<0>(t) == "hello" && get<1>(t) == "world") { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "Test 10: Mixed types... ";
    { 
        Tuple<int, double, std::string> t(42, 3.14, "pi"); 
        if (get<0>(t) == 42 && get<1>(t) == 3.14 && get<2>(t) == "pi") { 
            std::cout << "PASSED\n"; ++passed; 
        } else { 
            std::cout << "FAILED\n"; return 1; 
        } 
    }
    
    std::cout << "\n" << passed << "/" << total << " tests passed\n";
    std::cout << "✓ All tests PASSED!\n";
    return 0;
}
