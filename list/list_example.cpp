/**
 * @file list_example.cpp
 * @brief Examples demonstrating the List class implementation
 */

#include "list/list.hpp"
#include <iostream>
#include <string>

void example1_basic_operations() {
    std::cout << "\n=== Example 1: Basic Operations ===\n";
    
    List<int> l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);
    l.push_front(0);
    
    std::cout << "List: ";
    for (const auto& val : l) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    std::cout << "Front: " << l.front() << ", Back: " << l.back() << "\n";
    std::cout << "Size: " << l.size() << "\n";
}

void example2_initialization() {
    std::cout << "\n=== Example 2: Initializer List ===\n";
    
    List<int> l = {10, 20, 30, 40, 50};
    
    std::cout << "Initialized list: ";
    for (const auto& val : l) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

void example3_bidirectional_iteration() {
    std::cout << "\n=== Example 3: Bidirectional Iteration ===\n";
    
    List<std::string> words = {"apple", "banana", "cherry"};
    
    std::cout << "Forward: ";
    for (auto it = words.begin(); it != words.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

void example4_copy_and_move() {
    std::cout << "\n=== Example 4: Copy and Move ===\n";
    
    List<int> l1 = {1, 2, 3};
    
    // Copy
    List<int> l2 = l1;
    std::cout << "Copied list size: " << l2.size() << "\n";
    
    // Move
    List<int> l3 = std::move(l1);
    std::cout << "Moved list size: " << l3.size() << "\n";
    std::cout << "Original after move: " << l1.size() << "\n";
}

void example5_lru_cache_simulation() {
    std::cout << "\n=== Example 5: LRU Cache Simulation ===\n";
    
    List<int> cache;
    int capacity = 3;
    
    auto access = [&](int page) {
        std::cout << "  Access page " << page << ": ";
        
        // Remove if exists
        for (auto it = cache.begin(); it != cache.end(); ++it) {
            if (*it == page) {
                // Would remove here (simplified)
                std::cout << "(hit) ";
                break;
            }
        }
        
        // Add to front
        cache.push_front(page);
        
        // Evict if over capacity
        while (cache.size() > static_cast<size_t>(capacity)) {
            cache.pop_back();
        }
        
        std::cout << "Cache: ";
        for (const auto& p : cache) {
            std::cout << p << " ";
        }
        std::cout << "\n";
    };
    
    access(1);
    access(2);
    access(3);
    access(1);
    access(4);
}

void example6_playlist() {
    std::cout << "\n=== Example 6: Music Playlist ===\n";
    
    List<std::string> playlist;
    
    playlist.push_back("Song A");
    playlist.push_back("Song B");
    playlist.push_back("Song C");
    
    std::cout << "Playlist:\n";
    int track = 1;
    for (const auto& song : playlist) {
        std::cout << "  " << track++ << ". " << song << "\n";
    }
    
    std::cout << "\nNow playing: " << playlist.front() << "\n";
}

void example7_comparison_with_vector() {
    std::cout << "\n=== Example 7: When to Use List vs Vector ===\n";
    
    std::cout << "List advantages:\n";
    std::cout << "  - O(1) insert/erase at any position (with iterator)\n";
    std::cout << "  - No reallocation needed\n";
    std::cout << "  - Stable iterators/pointers\n";
    
    std::cout << "\nVector advantages:\n";
    std::cout << "  - O(1) random access\n";
    std::cout << "  - Better cache locality\n";
    std::cout << "  - Less memory overhead per element\n";
}

void example8_empty_and_clear() {
    std::cout << "\n=== Example 8: Empty and Clear ===\n";
    
    List<int> l = {1, 2, 3, 4, 5};
    
    std::cout << "Initial size: " << l.size() << "\n";
    std::cout << "Is empty: " << (l.empty() ? "yes" : "no") << "\n";
    
    l.clear();
    
    std::cout << "After clear - size: " << l.size() << "\n";
    std::cout << "Is empty: " << (l.empty() ? "yes" : "no") << "\n";
}

void example9_push_pop_both_ends() {
    std::cout << "\n=== Example 9: Double-Ended Operations ===\n";
    
    List<int> l;
    
    l.push_back(2);
    l.push_back(3);
    l.push_front(1);
    l.push_back(4);
    l.push_front(0);
    
    std::cout << "After pushes: ";
    for (const auto& val : l) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    l.pop_front();
    l.pop_back();
    
    std::cout << "After pops: ";
    for (const auto& val : l) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

void example10_strings() {
    std::cout << "\n=== Example 10: List of Strings ===\n";
    
    List<std::string> tasks;
    
    tasks.push_back("Write code");
    tasks.push_back("Test code");
    tasks.push_back("Deploy code");
    
    std::cout << "TODO List:\n";
    for (const auto& task : tasks) {
        std::cout << "  [ ] " << task << "\n";
    }
    
    tasks.pop_front();
    std::cout << "\nAfter completing first task:\n";
    for (const auto& task : tasks) {
        std::cout << "  [ ] " << task << "\n";
    }
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "            List Class Examples                  \n";
    std::cout << "=================================================\n";
    
    try {
        example1_basic_operations();
        example2_initialization();
        example3_bidirectional_iteration();
        example4_copy_and_move();
        example5_lru_cache_simulation();
        example6_playlist();
        example7_comparison_with_vector();
        example8_empty_and_clear();
        example9_push_pop_both_ends();
        example10_strings();
        
        std::cout << "\n=================================================\n";
        std::cout << "   All examples completed successfully!        \n";
        std::cout << "=================================================\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
