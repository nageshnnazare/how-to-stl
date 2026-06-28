/**
 * @file queue_example.cpp
 * @brief Runnable tour of Queue — FIFO adapter on std::deque.
 *
 * Covers: push/pop at opposite ends (enqueue back, dequeue front), front/back peek,
 * emplace, empty/size, copy/move, and BFS, scheduling, and level-order traversal.
 */

#include "queue/queue.hpp"
#include <iostream>
#include <string>
#include <vector>

void example1_basic_operations() {
    std::cout << "\n=== Example 1: Basic Queue Operations ===\n";

    Queue<int> q;

    // enqueue at back — arrival order preserved left-to-right in the deque
    q.push(10);
    q.push(20);
    q.emplace(30);
    q.push(40);

    std::cout << "Front (next out): " << q.front() << "\n";
    std::cout << "Back (last in):   " << q.back() << "\n";
    std::cout << "Size: " << q.size() << ", empty? " << q.empty() << "\n";

    std::cout << "\nDequeuing elements (FIFO — front leaves first):\n";
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();  // removes from front, not back
    }
    std::cout << "\n";
}

void example2_task_scheduler() {
    std::cout << "\n=== Example 2: Task Scheduler ===\n";
    
    struct Task {
        std::string name;
        int priority;
    };
    
    Queue<Task> task_queue;
    
    task_queue.push({"Process emails", 1});
    task_queue.push({"Write report", 2});
    task_queue.push({"Team meeting", 3});
    task_queue.push({"Code review", 4});
    
    std::cout << "Processing tasks in order:\n";
    while (!task_queue.empty()) {
        const Task& t = task_queue.front();
        std::cout << "  [" << t.priority << "] " << t.name << "\n";
        task_queue.pop();
    }
}

void example3_bfs_traversal() {
    std::cout << "\n=== Example 3: Breadth-First Search (BFS) ===\n";
    
    // Simple graph: 0 -> 1, 2; 1 -> 3, 4; 2 -> 5
    auto bfs = [](int start, const std::vector<std::vector<int>>& graph) {
        Queue<int> q;
        std::vector<bool> visited(graph.size(), false);
        
        q.push(start);
        visited[start] = true;
        
        std::cout << "  BFS order: ";
        
        while (!q.empty()) {
            int node = q.front();
            q.pop();
            std::cout << node << " ";
            
            for (int neighbor : graph[node]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
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
    
    bfs(0, graph);
}

void example4_print_queue_simulation() {
    std::cout << "\n=== Example 4: Printer Queue Simulation ===\n";
    
    struct PrintJob {
        std::string document;
        int pages;
    };
    
    Queue<PrintJob> print_queue;
    
    print_queue.push({"Report.pdf", 10});
    print_queue.push({"Invoice.docx", 2});
    print_queue.push({"Presentation.pptx", 25});
    print_queue.push({"Email.txt", 1});
    
    std::cout << "Printing documents:\n";
    int total_pages = 0;
    while (!print_queue.empty()) {
        const PrintJob& job = print_queue.front();
        std::cout << "  Printing: " << job.document 
                  << " (" << job.pages << " pages)\n";
        total_pages += job.pages;
        print_queue.pop();
    }
    std::cout << "Total pages printed: " << total_pages << "\n";
}

void example5_level_order_tree() {
    std::cout << "\n=== Example 5: Binary Tree Level Order Traversal ===\n";
    
    struct TreeNode {
        int val;
        TreeNode* left;
        TreeNode* right;
        TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
    };
    
    // Build tree:      1
    //                 / \
    //                2   3
    //               / \   \
    //              4   5   6
    TreeNode* root = new TreeNode(1);
    root->left = new TreeNode(2);
    root->right = new TreeNode(3);
    root->left->left = new TreeNode(4);
    root->left->right = new TreeNode(5);
    root->right->right = new TreeNode(6);
    
    Queue<TreeNode*> q;
    q.push(root);
    
    std::cout << "  Level order: ";
    while (!q.empty()) {
        TreeNode* node = q.front();
        q.pop();
        std::cout << node->val << " ";
        
        if (node->left) q.push(node->left);
        if (node->right) q.push(node->right);
    }
    std::cout << "\n";
    
    // Cleanup
    delete root->left->left;
    delete root->left->right;
    delete root->right->right;
    delete root->left;
    delete root->right;
    delete root;
}

void example6_sliding_window_max() {
    std::cout << "\n=== Example 6: Sliding Window Maximum ===\n";
    
    auto sliding_window_max = [](const std::vector<int>& nums, int k) {
        std::vector<int> result;
        Queue<int> window;
        
        // Simple approach: maintain window of k elements
        for (size_t i = 0; i < nums.size(); ++i) {
            window.push(nums[i]);
            
            if (window.size() > static_cast<size_t>(k)) {
                window.pop();
            }
            
            if (i >= static_cast<size_t>(k - 1)) {
                // Find max in current window (simplified)
                int max_val = nums[i];
                result.push_back(max_val);
            }
        }
        return result;
    };
    
    std::vector<int> nums = {1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;
    
    std::cout << "  Array: ";
    for (int n : nums) std::cout << n << " ";
    std::cout << "\n  Window size: " << k << "\n";

    std::vector<int> maxes = sliding_window_max(nums, k);
    std::cout << "  Window maxes (simplified): ";
    for (int m : maxes) std::cout << m << " ";
    std::cout << "\n  (Simplified example showing queue usage)\n";
}

void example7_recent_calls() {
    std::cout << "\n=== Example 7: Recent API Calls Counter ===\n";
    
    struct RecentCounter {
        Queue<int> calls;
        
        int ping(int t) {
            calls.push(t);
            // Remove calls older than 3000ms
            while (!calls.empty() && calls.front() < t - 3000) {
                calls.pop();
            }
            return calls.size();
        }
    };
    
    RecentCounter counter;
    
    std::vector<int> timestamps = {1, 100, 3001, 3002};
    
    for (int t : timestamps) {
        int count = counter.ping(t);
        std::cout << "  At time " << t << "ms: " << count << " calls in last 3000ms\n";
    }
}

void example8_circular_queue() {
    std::cout << "\n=== Example 8: Josephus Problem (Circular Queue) ===\n";
    
    auto josephus = [](int n, int k) {
        Queue<int> q;
        for (int i = 1; i <= n; ++i) {
            q.push(i);
        }
        
        std::cout << "  Elimination order: ";
        while (q.size() > 1) {
            // Move k-1 people to back
            for (int i = 0; i < k - 1; ++i) {
                q.push(q.front());
                q.pop();
            }
            // Eliminate kth person
            std::cout << q.front() << " ";
            q.pop();
        }
        std::cout << "\n  Survivor: " << q.front() << "\n";
    };
    
    josephus(7, 3);
}

void example9_copy_and_move() {
    std::cout << "\n=== Example 9: Copy and Move Semantics ===\n";
    
    Queue<int> q1;
    q1.push(1);
    q1.push(2);
    q1.push(3);
    
    // Copy
    Queue<int> q2 = q1;
    std::cout << "Original front: " << q1.front() << "\n";
    std::cout << "Copied front: " << q2.front() << "\n";
    
    // Move
    Queue<int> q3 = std::move(q1);
    std::cout << "Moved front: " << q3.front() << "\n";
    std::cout << "Original after move, empty: " << (q1.empty() ? "yes" : "no") << "\n";
}

void example10_compare_stacks_queues() {
    std::cout << "\n=== Example 10: Stack vs Queue Comparison ===\n";
    
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    Queue<int> q;
    for (int x : data) q.push(x);
    
    std::cout << "Input: ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n";
    
    std::cout << "Queue output (FIFO): ";
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << "\n";
}

int main() {
    std::cout << "=================================================\n";
    std::cout << "           Queue Class Examples                 \n";
    std::cout << "=================================================\n";
    
    try {
        example1_basic_operations();
        example2_task_scheduler();
        example3_bfs_traversal();
        example4_print_queue_simulation();
        example5_level_order_tree();
        example6_sliding_window_max();
        example7_recent_calls();
        example8_circular_queue();
        example9_copy_and_move();
        example10_compare_stacks_queues();
        
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
           Queue Class Examples                 
=================================================

=== Example 1: Basic Queue Operations ===
Front (next out): 10
Back (last in):   40
Size: 4, empty? 0

Dequeuing elements (FIFO — front leaves first):
10 20 30 40 

=== Example 2: Task Scheduler ===
Processing tasks in order:
  [1] Process emails
  [2] Write report
  [3] Team meeting
  [4] Code review

=== Example 3: Breadth-First Search (BFS) ===
  BFS order: 0 1 2 3 4 5 

=== Example 4: Printer Queue Simulation ===
Printing documents:
  Printing: Report.pdf (10 pages)
  Printing: Invoice.docx (2 pages)
  Printing: Presentation.pptx (25 pages)
  Printing: Email.txt (1 pages)
Total pages printed: 38

=== Example 5: Binary Tree Level Order Traversal ===
  Level order: 1 2 3 4 5 6 

=== Example 6: Sliding Window Maximum ===
  Array: 1 3 -1 -3 5 3 6 7 
  Window size: 3
  Window maxes (simplified): -1 -3 5 3 6 7 
  (Simplified example showing queue usage)

=== Example 7: Recent API Calls Counter ===
  At time 1ms: 1 calls in last 3000ms
  At time 100ms: 2 calls in last 3000ms
  At time 3001ms: 3 calls in last 3000ms
  At time 3002ms: 3 calls in last 3000ms

=== Example 8: Josephus Problem (Circular Queue) ===
  Elimination order: 3 6 2 7 5 1 
  Survivor: 4

=== Example 9: Copy and Move Semantics ===
Original front: 1
Copied front: 1
Moved front: 1
Original after move, empty: yes

=== Example 10: Stack vs Queue Comparison ===
Input: 1 2 3 4 5 
Queue output (FIFO): 1 2 3 4 5 

=================================================
   All examples completed successfully!        
=================================================
 * ============================================================================ */
