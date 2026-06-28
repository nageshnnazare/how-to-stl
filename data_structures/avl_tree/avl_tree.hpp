#ifndef DS_AVL_TREE_HPP
#define DS_AVL_TREE_HPP

// ============================================================================
//  AVLTree<T> -- a self-balancing binary search tree
// ============================================================================
//
// THE IDEA
// --------
// A BST can become a tall skinny chain if keys arrive sorted. An AVL tree keeps
// the BST ordering invariant *and* enforces balance: at every node
//
//     balance_factor = height(left) - height(right)  in  {-1, 0, 1}
//
// After insert, if some ancestor has |balance| > 1, we fix it with a rotation.
// Each node stores its subtree height so balance is O(1) to read.
//
// FOUR ROTATION PATTERNS (insert into the heavy side)
//
//   LL (right rotation)          RR (left rotation)
//   before:    z                before:  z
//             /                          \
//            y        ----->           y        <-----      z
//           /                              \                /
//          x                                x              y
//                                           \            /
//                                            x          x
//
//   LR: left-rotate left child, then right-rotate root.
//   RL: right-rotate right child, then left-rotate root.
//
// COMPLEXITY (height is always O(log N))
//     insert / contains ......... O(log N)
//     in_order .................. O(N)
//     height / size ............. O(1) cached / O(N) walk
// ============================================================================

#include <algorithm>    // std::max
#include <cstddef>      // std::size_t
#include <functional>   // std::function
#include <vector>       // std::vector

namespace ds {

template <typename T>
class AVLTree {
public:
    AVLTree() : root_(nullptr), size_(0) {}

    AVLTree(const AVLTree& other) : root_(nullptr), size_(0) {
        copy_from(other.root_);
    }

    AVLTree& operator=(AVLTree other) {
        swap(other);
        return *this;
    }

    ~AVLTree() { clear(); }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    std::size_t height() const {
        return root_ ? root_->height : 0;
    }

    void clear() {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
    }

    void insert(const T& value) {
        root_ = insert_node(root_, value);
    }

    bool contains(const T& value) const {
        return find_node(root_, value) != nullptr;
    }

    bool erase(const T& value) {
        std::size_t before = size_;
        root_ = erase_node(root_, value);
        return size_ < before;
    }

    void in_order(const std::function<void(const T&)>& visit) const {
        in_order_walk(root_, visit);
    }

    std::vector<T> in_order() const {
        std::vector<T> out;
        out.reserve(size_);
        in_order([&](const T& v) { out.push_back(v); });
        return out;
    }

    void swap(AVLTree& other) noexcept {
        std::swap(root_, other.root_);
        std::swap(size_, other.size_);
    }

private:
    struct Node {
        T value;
        Node* left;
        Node* right;
        std::size_t height;  // 1 for a leaf

        explicit Node(const T& v)
            : value(v), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root_;
    std::size_t size_;

    static std::size_t node_height(const Node* n) {
        return n ? n->height : 0;
    }

    static int balance_factor(const Node* n) {
        return static_cast<int>(node_height(n->left)) -
               static_cast<int>(node_height(n->right));
    }

    static void update_height(Node* n) {
        n->height = 1 + std::max(node_height(n->left), node_height(n->right));
    }

    // Right rotation around y (fixes LL).
    //
    //     y                x
    //    / \              / \
    //   x   C    -->     A   y
    //  / \                  / \
    // A   B                B   C
    static Node* rotate_right(Node* y) {
        Node* x = y->left;
        Node* B = x->right;
        x->right = y;
        y->left = B;
        update_height(y);
        update_height(x);
        return x;
    }

    // Left rotation around x (fixes RR).
    static Node* rotate_left(Node* x) {
        Node* y = x->right;
        Node* B = y->left;
        y->left = x;
        x->right = B;
        update_height(x);
        update_height(y);
        return y;
    }

    static Node* rebalance(Node* n) {
        update_height(n);
        int bf = balance_factor(n);

        if (bf > 1) {
            // Left-heavy.
            if (balance_factor(n->left) < 0)
                n->left = rotate_left(n->left);  // LR
            return rotate_right(n);               // LL
        }
        if (bf < -1) {
            // Right-heavy.
            if (balance_factor(n->right) > 0)
                n->right = rotate_right(n->right);  // RL
            return rotate_left(n);                  // RR
        }
        return n;
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    void copy_from(const Node* n) {
        if (!n) return;
        copy_from(n->left);
        insert(n->value);
        copy_from(n->right);
    }

    Node* insert_node(Node* n, const T& value) {
        if (!n) {
            ++size_;
            return new Node(value);
        }
        if (value < n->value)
            n->left = insert_node(n->left, value);
        else if (n->value < value)
            n->right = insert_node(n->right, value);
        else
            return n;  // duplicate
        return rebalance(n);
    }

    Node* find_node(Node* n, const T& value) const {
        if (!n) return nullptr;
        if (value < n->value) return find_node(n->left, value);
        if (n->value < value) return find_node(n->right, value);
        return n;
    }

    static Node* min_node(Node* n) {
        while (n && n->left) n = n->left;
        return n;
    }

    Node* erase_node(Node* n, const T& value) {
        if (!n) return nullptr;

        if (value < n->value) {
            n->left = erase_node(n->left, value);
        } else if (n->value < value) {
            n->right = erase_node(n->right, value);
        } else {
            if (!n->left || !n->right) {
                Node* child = n->left ? n->left : n->right;
                delete n;
                --size_;
                return child;
            }
            Node* succ = min_node(n->right);
            n->value = succ->value;
            n->right = erase_node(n->right, succ->value);
        }
        return rebalance(n);
    }

    static void in_order_walk(const Node* n, const std::function<void(const T&)>& visit) {
        if (!n) return;
        in_order_walk(n->left, visit);
        visit(n->value);
        in_order_walk(n->right, visit);
    }
};

}  // namespace ds

#endif  // DS_AVL_TREE_HPP
