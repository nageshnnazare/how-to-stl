/**
 * @file stack_example.cpp
 * @brief Runnable tour of Stack — LIFO adapter on std::deque (top = back).
 *
 * Covers: push/pop/top, emplace, empty/size, copy/move, custom vector backend,
 * and classic algorithms (parentheses, postfix, DFS, undo/redo, min-stack).
 * All operations mutate only the TOP end of the underlying container.
 */

#include "stack/stack.hpp"
#include <iostream>
#include <string>
#include <vector>

void example1_basic_operations() {
    std::cout << "\n=== Example 1: Basic Stack Operations ===\n";

    Stack<int> s;

    // push grows at top (deque back): 10, then 20 on top, etc.
    s.push(10);
    s.push(20);
    s.emplace(30);  // in-place at top — same end as push
    s.push(40);

    std::cout << "Top element: " << s.top() << "\n";
    std::cout << "Size: " << s.size() << ", empty? " << s.empty() << "\n";

    // swap exchanges underlying deques in O(1)
    Stack<int> other;
    other.push(99);
    s.swap(other);
    std::cout << "After swap, this stack top: " << s.top() << "\n";
    s.swap(other);  // restore for LIFO demo below
    while (!other.empty()) other.pop();

    std::cout << "\nPopping elements (LIFO — last pushed leaves first):\n";
    while (!s.empty()) {
        std::cout << s.top() << " ";
        s.pop();
    }
    std::cout << "\n";
}

void example2_string_stack() {
    std::cout << "\n=== Example 2: Stack of Strings ===\n";
    
    Stack<std::string> s;
    
    s.push("first");
    s.push("second");
    s.push("third");
    
    std::cout << "Processing undo operations:\n";
    while (!s.empty()) {
        std::cout << "  Undo: " << s.top() << "\n";
        s.pop();
    }
}

void example3_parentheses_matching() {
    std::cout << "\n=== Example 3: Balanced Parentheses Checker ===\n";
    
    auto is_balanced = [](const std::string& expr) {
        Stack<char> s;
        for (char c : expr) {
            if (c == '(' || c == '[' || c == '{') {
                s.push(c);
            } else if (c == ')' || c == ']' || c == '}') {
                if (s.empty()) return false;
                char top = s.top();
                s.pop();
                if ((c == ')' && top != '(') ||
                    (c == ']' && top != '[') ||
                    (c == '}' && top != '{')) {
                    return false;
                }
            }
        }
        return s.empty();
    };
    
    std::vector<std::string> expressions = {
        "(())",
        "([{}])",
        "(()",
        "([)]",
        "{[()]}"
    };
    
    for (const auto& expr : expressions) {
        std::cout << "  \"" << expr << "\" is " 
                  << (is_balanced(expr) ? "balanced" : "NOT balanced") << "\n";
    }
}

void example4_reverse_string() {
    std::cout << "\n=== Example 4: Reverse String Using Stack ===\n";
    
    std::string str = "Hello, World!";
    Stack<char> s;
    
    for (char c : str) {
        s.push(c);
    }
    
    std::string reversed;
    while (!s.empty()) {
        reversed += s.top();
        s.pop();
    }
    
    std::cout << "Original: " << str << "\n";
    std::cout << "Reversed: " << reversed << "\n";
}

void example5_evaluate_postfix() {
    std::cout << "\n=== Example 5: Evaluate Postfix Expression ===\n";
    
    auto evaluate_postfix = [](const std::vector<std::string>& tokens) {
        Stack<int> s;
        
        for (const auto& token : tokens) {
            if (token == "+" || token == "-" || token == "*" || token == "/") {
                int b = s.top(); s.pop();
                int a = s.top(); s.pop();
                
                if (token == "+") s.push(a + b);
                else if (token == "-") s.push(a - b);
                else if (token == "*") s.push(a * b);
                else if (token == "/") s.push(a / b);
            } else {
                s.push(std::stoi(token));
            }
        }
        
        return s.top();
    };
    
    std::vector<std::string> expr = {"2", "3", "+", "4", "*"};  // (2 + 3) * 4 = 20
    
    std::cout << "Postfix: ";
    for (const auto& t : expr) std::cout << t << " ";
    std::cout << "\n";
    
    int result = evaluate_postfix(expr);
    std::cout << "Result: " << result << "\n";
}

void example6_undo_redo() {
    std::cout << "\n=== Example 6: Undo/Redo System ===\n";
    
    Stack<std::string> undo_stack;
    Stack<std::string> redo_stack;
    std::string current_state = "";
    
    auto execute = [&](const std::string& action) {
        undo_stack.push(current_state);
        current_state = action;
        // Clear redo stack on new action
        while (!redo_stack.empty()) redo_stack.pop();
        std::cout << "  Executed: " << action << "\n";
    };
    
    auto undo = [&]() {
        if (!undo_stack.empty()) {
            redo_stack.push(current_state);
            current_state = undo_stack.top();
            undo_stack.pop();
            std::cout << "  Undo to: " << (current_state.empty() ? "(empty)" : current_state) << "\n";
        }
    };
    
    auto redo = [&]() {
        if (!redo_stack.empty()) {
            undo_stack.push(current_state);
            current_state = redo_stack.top();
            redo_stack.pop();
            std::cout << "  Redo to: " << current_state << "\n";
        }
    };
    
    execute("Type A");
    execute("Type B");
    execute("Type C");
    undo();
    undo();
    redo();
    execute("Type D");
}

void example7_dfs_traversal() {
    std::cout << "\n=== Example 7: Depth-First Search (DFS) ===\n";
    
    // Simple graph: 0 -> 1, 2; 1 -> 3, 4; 2 -> 5
    auto dfs = [](int start, const std::vector<std::vector<int>>& graph) {
        Stack<int> s;
        std::vector<bool> visited(graph.size(), false);
        
        s.push(start);
        std::cout << "  DFS order: ";
        
        while (!s.empty()) {
            int node = s.top();
            s.pop();
            
            if (!visited[node]) {
                visited[node] = true;
                std::cout << node << " ";
                
                // Push children in reverse order for correct DFS
                for (int i = graph[node].size() - 1; i >= 0; --i) {
                    if (!visited[graph[node][i]]) {
                        s.push(graph[node][i]);
                    }
                }
            }
        }
        std::cout << "\n";
    };
    
    std::vector<std::vector<int>> graph = {
        {1, 2},    // 0
        {3, 4},    // 1
        {5},       // 2
        {},        // 3
        {},        // 4
        {}         // 5
    };
    
    dfs(0, graph);
}

void example8_min_stack() {
    std::cout << "\n=== Example 8: Stack with Min Tracking ===\n";
    
    struct MinStack {
        Stack<int> data;
        Stack<int> mins;
        
        void push(int x) {
            data.push(x);
            if (mins.empty() || x <= mins.top()) {
                mins.push(x);
            }
        }
        
        void pop() {
            if (data.top() == mins.top()) {
                mins.pop();
            }
            data.pop();
        }
        
        int top() { return data.top(); }
        int get_min() { return mins.top(); }
    };
    
    MinStack ms;
    ms.push(5);
    ms.push(2);
    ms.push(7);
    ms.push(1);
    
    std::cout << "  Top: " << ms.top() << ", Min: " << ms.get_min() << "\n";
    ms.pop();
    std::cout << "  After pop - Top: " << ms.top() << ", Min: " << ms.get_min() << "\n";
}

void example9_copy_and_move() {
    std::cout << "\n=== Example 9: Copy and Move Semantics ===\n";
    
    Stack<int> s1;
    s1.push(1);
    s1.push(2);
    s1.push(3);
    
    // Copy
    Stack<int> s2 = s1;
    std::cout << "Original top: " << s1.top() << "\n";
    std::cout << "Copied top: " << s2.top() << "\n";
    
    // Move
    Stack<int> s3 = std::move(s1);
    std::cout << "Moved top: " << s3.top() << "\n";
    std::cout << "Original after move, empty: " << (s1.empty() ? "yes" : "no") << "\n";
}

void example10_custom_container() {
    std::cout << "\n=== Example 10: Stack with Vector ===\n";
    
    Stack<int, std::vector<int>> s;
    
    s.push(100);
    s.push(200);
    s.push(300);
    
    std::cout << "Stack with vector backend:\n";
    while (!s.empty()) {
        std::cout << "  " << s.top() << "\n";
        s.pop();
    }
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "           Stack Class Examples                 \n";
    std::cout << "=================================================\n";
    
    try {
        example1_basic_operations();
        example2_string_stack();
        example3_parentheses_matching();
        example4_reverse_string();
        example5_evaluate_postfix();
        example6_undo_redo();
        example7_dfs_traversal();
        example8_min_stack();
        example9_copy_and_move();
        example10_custom_container();
        
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
           Stack Class Examples                 
=================================================

=== Example 1: Basic Stack Operations ===
Top element: 40
Size: 4, empty? 0
After swap, this stack top: 99

Popping elements (LIFO — last pushed leaves first):
40 30 20 10 

=== Example 2: Stack of Strings ===
Processing undo operations:
  Undo: third
  Undo: second
  Undo: first

=== Example 3: Balanced Parentheses Checker ===
  "(())" is balanced
  "([{}])" is balanced
  "(()" is NOT balanced
  "([)]" is NOT balanced
  "{[()]}" is balanced

=== Example 4: Reverse String Using Stack ===
Original: Hello, World!
Reversed: !dlroW ,olleH

=== Example 5: Evaluate Postfix Expression ===
Postfix: 2 3 + 4 * 
Result: 20

=== Example 6: Undo/Redo System ===
  Executed: Type A
  Executed: Type B
  Executed: Type C
  Undo to: Type B
  Undo to: Type A
  Redo to: Type B
  Executed: Type D

=== Example 7: Depth-First Search (DFS) ===
  DFS order: 0 1 3 4 2 5 

=== Example 8: Stack with Min Tracking ===
  Top: 1, Min: 1
  After pop - Top: 7, Min: 2

=== Example 9: Copy and Move Semantics ===
Original top: 3
Copied top: 3
Moved top: 3
Original after move, empty: yes

=== Example 10: Stack with Vector ===
Stack with vector backend:
  300
  200
  100

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
