/**
 * @file priority_queue_example.cpp
 * @brief Runnable tour of PriorityQueue — binary max-heap in a vector.
 *
 * Covers: push/pop/top (sift-up / sift-down), min-heap via std::greater,
 * Floyd build_heap from range, emplace, is_heap(), copy/move, and heap algorithms.
 */

#include "priority_queue/priority_queue.hpp"
#include <iostream>
#include <string>

void example1_basic_operations() {
    std::cout << "\n=== Example 1: Basic Operations (Max Heap) ===\n";
    
    PriorityQueue<int> pq;
    
    // Insert elements
    pq.push(30);
    pq.push(10);
    pq.push(50);
    pq.push(20);
    pq.push(40);
    
    std::cout << "Top element (max): " << pq.top() << "\n";
    std::cout << "Size: " << pq.size() << "\n";
    std::cout << "Heap property holds: " << (pq.is_heap() ? "yes" : "no") << "\n";

    std::cout << "\nPopping all elements (descending order):\n";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n";
}

void example2_min_heap() {
    std::cout << "\n=== Example 2: Min Heap (Custom Comparator) ===\n";
    
    // Use greater<int> for min heap
    PriorityQueue<int, std::vector<int>, std::greater<int>> min_pq;
    
    min_pq.push(30);
    min_pq.push(10);
    min_pq.push(50);
    min_pq.push(20);
    min_pq.push(40);
    
    std::cout << "Top element (min): " << min_pq.top() << "\n";
    
    std::cout << "\nPopping all elements (ascending order):\n";
    while (!min_pq.empty()) {
        std::cout << min_pq.top() << " ";
        min_pq.pop();
    }
    std::cout << "\n";
}

void example3_range_constructor() {
    std::cout << "\n=== Example 3: Range Constructor ===\n";
    
    std::vector<int> data = {5, 2, 8, 1, 9, 3, 7};
    
    PriorityQueue<int> pq(data.begin(), data.end());
    
    std::cout << "Constructed from vector: {5, 2, 8, 1, 9, 3, 7}\n";
    std::cout << "Top element: " << pq.top() << "\n";
    
    std::cout << "All elements (descending):\n";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n";
}

void example4_custom_type() {
    std::cout << "\n=== Example 4: Custom Type (Tasks by Priority) ===\n";
    
    struct Task {
        std::string name;
        int priority;
        
        Task(const std::string& n, int p) : name(n), priority(p) {}
    };
    
    // Custom comparator: higher priority = higher value
    auto cmp = [](const Task& a, const Task& b) {
        return a.priority < b.priority;
    };
    
    PriorityQueue<Task, std::vector<Task>, decltype(cmp)> task_queue(cmp);
    
    task_queue.push(Task("Email client", 2));
    task_queue.push(Task("Fix critical bug", 5));
    task_queue.push(Task("Update docs", 1));
    task_queue.push(Task("Review PR", 3));
    task_queue.push(Task("Security patch", 5));
    
    std::cout << "Processing tasks by priority:\n";
    while (!task_queue.empty()) {
        const Task& t = task_queue.top();
        std::cout << "  [Priority " << t.priority << "] " << t.name << "\n";
        task_queue.pop();
    }
}

void example5_emplace() {
    std::cout << "\n=== Example 5: Emplace (In-place Construction) ===\n";
    
    struct Point {
        int x, y;
        Point(int x_, int y_) : x(x_), y(y_) {
            std::cout << "  Constructed Point(" << x << ", " << y << ")\n";
        }
    };
    
    auto cmp = [](const Point& a, const Point& b) {
        return (a.x + a.y) < (b.x + b.y);
    };
    
    PriorityQueue<Point, std::vector<Point>, decltype(cmp)> pq(cmp);
    
    std::cout << "Using emplace:\n";
    pq.emplace(3, 4);  // sum = 7
    pq.emplace(1, 2);  // sum = 3
    pq.emplace(5, 1);  // sum = 6
    
    std::cout << "\nTop point: (" << pq.top().x << ", " << pq.top().y 
              << ") with sum " << (pq.top().x + pq.top().y) << "\n";
}

void example6_median_finder() {
    std::cout << "\n=== Example 6: Finding Median with Two Heaps ===\n";
    
    // Max heap for lower half
    PriorityQueue<int> lower_half;
    
    // Min heap for upper half
    PriorityQueue<int, std::vector<int>, std::greater<int>> upper_half;
    
    auto add_number = [&](int num) {
        if (lower_half.empty() || num <= lower_half.top()) {
            lower_half.push(num);
        } else {
            upper_half.push(num);
        }
        
        // Balance heaps
        if (lower_half.size() > upper_half.size() + 1) {
            upper_half.push(lower_half.top());
            lower_half.pop();
        } else if (upper_half.size() > lower_half.size()) {
            lower_half.push(upper_half.top());
            upper_half.pop();
        }
    };
    
    auto get_median = [&]() -> double {
        if (lower_half.size() == upper_half.size()) {
            return (lower_half.top() + upper_half.top()) / 2.0;
        }
        return lower_half.top();
    };
    
    std::vector<int> stream = {5, 15, 1, 3, 8, 7};
    
    std::cout << "Processing stream: ";
    for (int num : stream) {
        std::cout << num << " ";
        add_number(num);
    }
    std::cout << "\n\nRunning medians:\n";
    
    // Reset and show median after each insertion
    lower_half = PriorityQueue<int>();
    upper_half = PriorityQueue<int, std::vector<int>, std::greater<int>>();
    
    for (int num : stream) {
        add_number(num);
        std::cout << "  After adding " << num << ", median = " << get_median() << "\n";
    }
}

void example7_top_k_elements() {
    std::cout << "\n=== Example 7: Top K Largest Elements ===\n";
    
    auto top_k = [](const std::vector<int>& nums, int k) {
        // Use min heap of size k
        PriorityQueue<int, std::vector<int>, std::greater<int>> pq;
        
        for (int num : nums) {
            pq.push(num);
            if (pq.size() > static_cast<size_t>(k)) {
                pq.pop();  // Remove smallest
            }
        }
        
        std::vector<int> result;
        while (!pq.empty()) {
            result.push_back(pq.top());
            pq.pop();
        }
        return result;
    };
    
    std::vector<int> nums = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    int k = 3;
    
    std::cout << "Array: ";
    for (int n : nums) std::cout << n << " ";
    std::cout << "\n";
    
    auto result = top_k(nums, k);
    std::cout << "Top " << k << " largest elements: ";
    for (int n : result) std::cout << n << " ";
    std::cout << "\n";
}

void example8_heap_sort() {
    std::cout << "\n=== Example 8: Heap Sort ===\n";
    
    std::vector<int> arr = {64, 34, 25, 12, 22, 11, 90};
    
    std::cout << "Original: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n";
    
    // Insert all into priority queue
    PriorityQueue<int> pq;
    for (int x : arr) {
        pq.push(x);
    }
    
    // Extract in sorted order (descending)
    std::vector<int> sorted;
    while (!pq.empty()) {
        sorted.push_back(pq.top());
        pq.pop();
    }
    
    std::cout << "Sorted (descending): ";
    for (int x : sorted) std::cout << x << " ";
    std::cout << "\n";
}

void example9_merge_k_lists() {
    std::cout << "\n=== Example 9: Merge K Sorted Lists ===\n";
    
    struct Element {
        int value;
        int list_id;
        size_t index;
        
        Element(int v, int l, size_t i) : value(v), list_id(l), index(i) {}
    };
    
    auto cmp = [](const Element& a, const Element& b) {
        return a.value > b.value;  // Min heap
    };
    
    std::vector<std::vector<int>> lists = {
        {1, 4, 7},
        {2, 5, 8},
        {3, 6, 9}
    };
    
    PriorityQueue<Element, std::vector<Element>, decltype(cmp)> pq(cmp);
    
    // Add first element from each list
    for (size_t i = 0; i < lists.size(); ++i) {
        if (!lists[i].empty()) {
            pq.emplace(lists[i][0], static_cast<int>(i), 0);
        }
    }
    
    std::cout << "Merging lists:\n";
    for (const auto& list : lists) {
        std::cout << "  ";
        for (int x : list) std::cout << x << " ";
        std::cout << "\n";
    }
    
    std::cout << "\nMerged result: ";
    while (!pq.empty()) {
        Element e = pq.top();
        pq.pop();
        
        std::cout << e.value << " ";
        
        // Add next element from same list
        size_t next_idx = e.index + 1;
        if (next_idx < lists[e.list_id].size()) {
            pq.emplace(lists[e.list_id][next_idx], e.list_id, next_idx);
        }
    }
    std::cout << "\n";
}

void example10_copy_and_move() {
    std::cout << "\n=== Example 10: Copy and Move Semantics ===\n";
    
    PriorityQueue<int> pq1;
    pq1.push(10);
    pq1.push(30);
    pq1.push(20);
    
    // Copy
    PriorityQueue<int> pq2 = pq1;
    std::cout << "Original top: " << pq1.top() << "\n";
    std::cout << "Copied top: " << pq2.top() << "\n";
    
    // Move
    PriorityQueue<int> pq3 = std::move(pq1);
    std::cout << "Moved top: " << pq3.top() << "\n";
    std::cout << "Original after move, empty: " << (pq1.empty() ? "yes" : "no") << "\n";
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "      PriorityQueue Class Examples             \n";
    std::cout << "=================================================\n";
    
    try {
        example1_basic_operations();
        example2_min_heap();
        example3_range_constructor();
        example4_custom_type();
        example5_emplace();
        example6_median_finder();
        example7_top_k_elements();
        example8_heap_sort();
        example9_merge_k_lists();
        example10_copy_and_move();
        
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
      PriorityQueue Class Examples             
=================================================

=== Example 1: Basic Operations (Max Heap) ===
Top element (max): 50
Size: 5
Heap property holds: yes

Popping all elements (descending order):
50 40 30 20 10 

=== Example 2: Min Heap (Custom Comparator) ===
Top element (min): 10

Popping all elements (ascending order):
10 20 30 40 50 

=== Example 3: Range Constructor ===
Constructed from vector: {5, 2, 8, 1, 9, 3, 7}
Top element: 9
All elements (descending):
9 8 7 5 3 2 1 

=== Example 4: Custom Type (Tasks by Priority) ===
Processing tasks by priority:
  [Priority 5] Fix critical bug
  [Priority 5] Security patch
  [Priority 3] Review PR
  [Priority 2] Email client
  [Priority 1] Update docs

=== Example 5: Emplace (In-place Construction) ===
Using emplace:
  Constructed Point(3, 4)
  Constructed Point(1, 2)
  Constructed Point(5, 1)

Top point: (3, 4) with sum 7

=== Example 6: Finding Median with Two Heaps ===
Processing stream: 5 15 1 3 8 7 

Running medians:
  After adding 5, median = 5
  After adding 15, median = 10
  After adding 1, median = 5
  After adding 3, median = 4
  After adding 8, median = 5
  After adding 7, median = 6

=== Example 7: Top K Largest Elements ===
Array: 3 1 4 1 5 9 2 6 5 3 5 
Top 3 largest elements: 5 6 9 

=== Example 8: Heap Sort ===
Original: 64 34 25 12 22 11 90 
Sorted (descending): 90 64 34 25 22 12 11 

=== Example 9: Merge K Sorted Lists ===
Merging lists:
  1 4 7 
  2 5 8 
  3 6 9 

Merged result: 1 2 3 4 5 6 7 8 9 

=== Example 10: Copy and Move Semantics ===
Original top: 30
Copied top: 30
Moved top: 30
Original after move, empty: yes

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
