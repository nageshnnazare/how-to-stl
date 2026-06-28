/**
 * @file string_example.cpp
 * @brief Runnable tour of the hand-rolled String class (SSO, growth, search).
 *
 * Demonstrates:
 *   - Construction, assignment, and element access
 *   - Capacity / SSO vs heap (reserve, shrink_to_fit, long strings)
 *   - Modifiers: append, insert, erase, replace, push_back / pop_back
 *   - Search: find, rfind, substr, compare
 *   - Concatenation, comparisons, iterators, move semantics, swap
 */

#include "string/string.hpp"
#include <iostream>
#include <vector>

// ============================================================================
// EXAMPLE FUNCTIONS
// ============================================================================

/**
 * Example 1: Basic construction and assignment
 */
void example1_construction() {
    std::cout << "\n=== Example 1: Construction ===\n";
    
    // Default constructor
    String s1;
    std::cout << "Empty string: '" << s1 << "' (size: " << s1.size() << ")\n";
    
    // From C-string
    String s2("Hello, World!");
    std::cout << "From C-string: '" << s2 << "'\n";
    
    // Repeated character
    String s3(5, '*');
    std::cout << "Repeated char: '" << s3 << "'\n";
    
    // Copy constructor
    String s4(s2);
    std::cout << "Copy: '" << s4 << "'\n";
    
    // Substring
    String s5(s2, 7, 5);
    std::cout << "Substring: '" << s5 << "'\n";
}

/**
 * Example 2: Assignment operations
 */
void example2_assignment() {
    std::cout << "\n=== Example 2: Assignment ===\n";
    
    String s1 = "Initial";
    std::cout << "Initial: '" << s1 << "'\n";
    
    // Copy assignment
    String s2;
    s2 = s1;
    std::cout << "Copy assigned: '" << s2 << "'\n";
    
    // C-string assignment
    s1 = "Modified";
    std::cout << "C-string assigned: '" << s1 << "'\n";
    
    // Character assignment
    s1 = 'X';
    std::cout << "Char assigned: '" << s1 << "'\n";
}

/**
 * Example 3: Element access
 */
void example3_element_access() {
    std::cout << "\n=== Example 3: Element Access ===\n";
    
    String s = "Programming";
    
    // Subscript operator
    std::cout << "First char: " << s[0] << "\n";
    std::cout << "Last char: " << s[s.size() - 1] << "\n";
    
    // at() with bounds checking
    try {
        std::cout << "Char at 5: " << s.at(5) << "\n";
        std::cout << "Trying out of bounds...\n";
        char c = s.at(100); // Will throw
        (void)c;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
    
    // front() and back()
    std::cout << "Front: " << s.front() << "\n";
    std::cout << "Back: " << s.back() << "\n";
    
    // Modify characters
    s[0] = 'p';
    std::cout << "After modification: '" << s << "'\n";
}

/**
 * Example 4: Capacity management
 */
void example4_capacity() {
    std::cout << "\n=== Example 4: Capacity ===\n";
    
    String s = "Short";
    std::cout << "String: '" << s << "'\n";
    std::cout << "  Size: " << s.size() << "\n";
    std::cout << "  Capacity: " << s.capacity() << "\n";
    std::cout << "  Empty: " << (s.empty() ? "yes" : "no") << "\n";
    
    // Reserve capacity
    s.reserve(100);
    std::cout << "\nAfter reserve(100):\n";
    std::cout << "  Capacity: " << s.capacity() << "\n";
    std::cout << "  String unchanged: '" << s << "'\n";
    
    // Resize
    s.resize(10, '!');
    std::cout << "\nAfter resize(10, '!'):\n";
    std::cout << "  String: '" << s << "'\n";
    std::cout << "  Size: " << s.size() << "\n";
}

/**
 * Example 5: Append operations
 */
void example5_append() {
    std::cout << "\n=== Example 5: Append ===\n";
    
    String s = "Hello";
    std::cout << "Initial: '" << s << "'\n";
    
    // Append string
    s.append(" World");
    std::cout << "After append(\" World\"): '" << s << "'\n";
    
    // Append using +=
    s += "!";
    std::cout << "After += \"!\": '" << s << "'\n";
    
    // Append repeated character
    s += String(3, '.');
    std::cout << "After appending '...': '" << s << "'\n";
    
    // push_back
    s.push_back('?');
    std::cout << "After push_back('?'): '" << s << "'\n";
}

/**
 * Example 6: Insert and erase operations
 */
void example6_insert_erase() {
    std::cout << "\n=== Example 6: Insert and Erase ===\n";
    
    String s = "Hello World";
    std::cout << "Initial: '" << s << "'\n";
    
    // Insert
    s.insert(5, ", Beautiful");
    std::cout << "After insert(5, \", Beautiful\"): '" << s << "'\n";
    
    // Erase
    s.erase(5, 11);  // Remove ", Beautiful"
    std::cout << "After erase(5, 11): '" << s << "'\n";
    
    // pop_back
    s.pop_back();
    std::cout << "After pop_back(): '" << s << "'\n";
}

/**
 * Example 7: Replace operations
 */
void example7_replace() {
    std::cout << "\n=== Example 7: Replace ===\n";
    
    String s = "Hello World";
    std::cout << "Initial: '" << s << "'\n";
    
    // Replace substring
    s.replace(6, 5, "C++");
    std::cout << "After replace(6, 5, \"C++\"): '" << s << "'\n";
    
    // Replace with longer string
    s.replace(0, 5, "Greetings");
    std::cout << "After replace(0, 5, \"Greetings\"): '" << s << "'\n";
}

/**
 * Example 8: String operations (find, substr, compare)
 */
void example8_operations() {
    std::cout << "\n=== Example 8: String Operations ===\n";
    
    String s = "The quick brown fox jumps over the lazy dog";
    std::cout << "String: '" << s << "'\n";
    
    // Find
    auto pos = s.find("fox");
    if (pos != String::npos) {
        std::cout << "Found 'fox' at position " << pos << "\n";
    }
    
    pos = s.find('q');
    std::cout << "Found 'q' at position " << pos << "\n";
    
    // Find not found
    pos = s.find("cat");
    std::cout << "Find 'cat': " << (pos == String::npos ? "not found" : "found") << "\n";
    
    // Reverse find
    pos = s.rfind('o');
    std::cout << "Last 'o' at position " << pos << "\n";
    
    // Substring
    String sub = s.substr(16, 3);
    std::cout << "Substring (16, 3): '" << sub << "'\n";
    
    // Compare
    String s1 = "apple";
    String s2 = "banana";
    std::cout << "\nCompare 'apple' vs 'banana': " << s1.compare(s2) << " (negative = less)\n";
    std::cout << "Compare 'banana' vs 'apple': " << s2.compare(s1) << " (positive = greater)\n";
    std::cout << "Compare 'apple' vs 'apple': " << s1.compare("apple") << " (zero = equal)\n";
}

/**
 * Example 9: Concatenation
 */
void example9_concatenation() {
    std::cout << "\n=== Example 9: Concatenation ===\n";
    
    String s1 = "Hello";
    String s2 = "World";
    
    // String + String
    String s3 = s1 + " " + s2;
    std::cout << "s1 + \" \" + s2 = '" << s3 << "'\n";
    
    // String + C-string
    String s4 = s1 + ", " + s2 + "!";
    std::cout << "Concatenation result: '" << s4 << "'\n";
    
    // char + String
    String s5 = '[' + s1 + ']';
    std::cout << "'[' + s1 + ']' = '" << s5 << "'\n";
}

/**
 * Example 10: Comparison operators
 */
void example10_comparisons() {
    std::cout << "\n=== Example 10: Comparisons ===\n";
    
    String s1 = "apple";
    String s2 = "banana";
    String s3 = "apple";
    
    std::cout << "s1 = '" << s1 << "', s2 = '" << s2 << "', s3 = '" << s3 << "'\n";
    
    std::cout << "s1 == s3: " << (s1 == s3 ? "true" : "false") << "\n";
    std::cout << "s1 != s2: " << (s1 != s2 ? "true" : "false") << "\n";
    std::cout << "s1 < s2: " << (s1 < s2 ? "true" : "false") << "\n";
    std::cout << "s2 > s1: " << (s2 > s1 ? "true" : "false") << "\n";
    std::cout << "s1 <= s3: " << (s1 <= s3 ? "true" : "false") << "\n";
    
    // Compare with C-string
    std::cout << "s1 == \"apple\": " << (s1 == "apple" ? "true" : "false") << "\n";
}

/**
 * Example 11: Iterators
 */
void example11_iterators() {
    std::cout << "\n=== Example 11: Iterators ===\n";
    
    String s = "Iterator";
    
    // Forward iteration
    std::cout << "Forward: ";
    for (String::iterator it = s.begin(); it != s.end(); ++it) {
        std::cout << *it;
    }
    std::cout << "\n";
    
    // Range-based for loop
    std::cout << "Range-based: ";
    for (char c : s) {
        std::cout << c;
    }
    std::cout << "\n";
    
    // Modify via iterator
    for (char& c : s) {
        c = std::toupper(c);
    }
    std::cout << "Uppercase: '" << s << "'\n";
}

/**
 * Example 12: Move semantics
 */
void example12_move_semantics() {
    std::cout << "\n=== Example 12: Move Semantics ===\n";
    
    String s1 = "This is a longer string to avoid SSO";
    std::cout << "s1: '" << s1 << "' (capacity: " << s1.capacity() << ")\n";
    
    // Move constructor
    String s2(std::move(s1));
    std::cout << "\nAfter move construction:\n";
    std::cout << "  s1: '" << s1 << "' (size: " << s1.size() << ")\n";
    std::cout << "  s2: '" << s2 << "' (capacity: " << s2.capacity() << ")\n";
    
    // Move assignment
    String s3;
    s3 = std::move(s2);
    std::cout << "\nAfter move assignment:\n";
    std::cout << "  s2: '" << s2 << "' (size: " << s2.size() << ")\n";
    std::cout << "  s3: '" << s3 << "'\n";
}

/**
 * Example 13: Container usage
 */
void example13_containers() {
    std::cout << "\n=== Example 13: Container Usage ===\n";
    
    std::vector<String> words;
    words.push_back("Hello");
    words.push_back("World");
    words.push_back("from");
    words.push_back("C++");
    
    std::cout << "Vector of strings:\n";
    for (size_t i = 0; i < words.size(); ++i) {
        std::cout << "  [" << i << "] = '" << words[i] << "'\n";
    }
    
    // Create sentence
    String sentence;
    for (const auto& word : words) {
        if (!sentence.empty()) {
            sentence += " ";
        }
        sentence += word;
    }
    std::cout << "Sentence: '" << sentence << "'\n";
}

/**
 * Example 14: Small String Optimization (SSO)
 */
void example14_sso() {
    std::cout << "\n=== Example 14: Small String Optimization ===\n";
    
    // Short string (uses SSO)
    String short_str = "Short";
    std::cout << "Short string: '" << short_str << "'\n";
    std::cout << "  Size: " << short_str.size() << "\n";
    std::cout << "  Capacity: " << short_str.capacity() << " (SSO buffer)\n";
    
    // Long string (uses heap)
    String long_str = "This is a much longer string that exceeds SSO capacity";
    std::cout << "\nLong string: '" << long_str << "'\n";
    std::cout << "  Size: " << long_str.size() << "\n";
    std::cout << "  Capacity: " << long_str.capacity() << " (heap allocated)\n";
}

/**
 * Example 15: Swap
 */
void example15_swap() {
    std::cout << "\n=== Example 15: Swap ===\n";
    
    String s1 = "First";
    String s2 = "Second";
    
    std::cout << "Before swap:\n";
    std::cout << "  s1 = '" << s1 << "'\n";
    std::cout << "  s2 = '" << s2 << "'\n";
    
    s1.swap(s2);
    
    std::cout << "After swap:\n";
    std::cout << "  s1 = '" << s1 << "'\n";
    std::cout << "  s2 = '" << s2 << "'\n";
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=================================================\n";
    std::cout << "     Custom String Class Examples               \n";
    std::cout << "=================================================\n";
    
    try {
        example1_construction();
        example2_assignment();
        example3_element_access();
        example4_capacity();
        example5_append();
        example6_insert_erase();
        example7_replace();
        example8_operations();
        example9_concatenation();
        example10_comparisons();
        example11_iterators();
        example12_move_semantics();
        example13_containers();
        example14_sso();
        example15_swap();
        
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
     Custom String Class Examples               
=================================================

=== Example 1: Construction ===
Empty string: '' (size: 0)
From C-string: 'Hello, World!'
Repeated char: '*****'
Copy: 'Hello, World!'
Substring: 'World'

=== Example 2: Assignment ===
Initial: 'Initial'
Copy assigned: 'Initial'
C-string assigned: 'Modified'
Char assigned: 'X'

=== Example 3: Element Access ===
First char: P
Last char: g
Char at 5: a
Trying out of bounds...
Caught exception: String::at: position out of range
Front: P
Back: g
After modification: 'programming'

=== Example 4: Capacity ===
String: 'Short'
  Size: 5
  Capacity: 15
  Empty: no

After reserve(100):
  Capacity: 100
  String unchanged: 'Short'

After resize(10, '!'):
  String: 'Short!!!!!'
  Size: 10

=== Example 5: Append ===
Initial: 'Hello'
After append(" World"): 'Hello World'
After += "!": 'Hello World!'
After appending '...': 'Hello World!...'
After push_back('?'): 'Hello World!...?'

=== Example 6: Insert and Erase ===
Initial: 'Hello World'
After insert(5, ", Beautiful"): 'Hello, Beautiful World'
After erase(5, 11): 'Hello World'
After pop_back(): 'Hello Worl'

=== Example 7: Replace ===
Initial: 'Hello World'
After replace(6, 5, "C++"): 'Hello C++'
After replace(0, 5, "Greetings"): 'Greetings C++'

=== Example 8: String Operations ===
String: 'The quick brown fox jumps over the lazy dog'
Found 'fox' at position 16
Found 'q' at position 4
Find 'cat': not found
Last 'o' at position 41
Substring (16, 3): 'fox'

Compare 'apple' vs 'banana': -1 (negative = less)
Compare 'banana' vs 'apple': 1 (positive = greater)
Compare 'apple' vs 'apple': 0 (zero = equal)

=== Example 9: Concatenation ===
s1 + " " + s2 = 'Hello World'
Concatenation result: 'Hello, World!'
'[' + s1 + ']' = '[Hello]'

=== Example 10: Comparisons ===
s1 = 'apple', s2 = 'banana', s3 = 'apple'
s1 == s3: true
s1 != s2: true
s1 < s2: true
s2 > s1: true
s1 <= s3: true
s1 == "apple": true

=== Example 11: Iterators ===
Forward: Iterator
Range-based: Iterator
Uppercase: 'ITERATOR'

=== Example 12: Move Semantics ===
s1: 'This is a longer string to avoid SSO' (capacity: 36)

After move construction:
  s1: '' (size: 0)
  s2: 'This is a longer string to avoid SSO' (capacity: 36)

After move assignment:
  s2: '' (size: 0)
  s3: 'This is a longer string to avoid SSO'

=== Example 13: Container Usage ===
Vector of strings:
  [0] = 'Hello'
  [1] = 'World'
  [2] = 'from'
  [3] = 'C++'
Sentence: 'Hello World from C++'

=== Example 14: Small String Optimization ===
Short string: 'Short'
  Size: 5
  Capacity: 15 (SSO buffer)

Long string: 'This is a much longer string that exceeds SSO capacity'
  Size: 54
  Capacity: 54 (heap allocated)

=== Example 15: Swap ===
Before swap:
  s1 = 'First'
  s2 = 'Second'
After swap:
  s1 = 'Second'
  s2 = 'First'

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
