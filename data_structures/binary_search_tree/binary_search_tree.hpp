#ifndef DS_BINARY_SEARCH_TREE_HPP
#define DS_BINARY_SEARCH_TREE_HPP

// ============================================================================
//  BST<T> -- a textbook binary search tree
// ============================================================================
//
// THE IDEA
// --------
// Every node holds a value and (at most) two children. The *ordering invariant*:
//
//     for every node N:
//         everything in N.left  is < N.value
//         everything in N.right is > N.value
//
// That invariant means an in-order walk (left, node, right) visits values in
// sorted order — the tree *is* a sorted structure, not just a container.
//
// A SMALL BST (insert 8, 3, 10, 1, 6, 14, 4)
//
//                    (8)
//                   /   \
//                (3)     (10)
//               /   \       \
//            (1)   (6)     (14)
//                 /
//              (4)
//
// In-order: 1 3 4 6 8 10 14
//
// ERASE HAS THREE CASES
// ---------------------
//   1. Leaf           — unlink and delete.
//   2. One child      — splice the child up in place of the node.
//   3. Two children   — copy the in-order successor (min of right subtree)
//                       into the node, then erase that successor.
//
// COMPLEXITY (balanced tree would be O(log N); unbalanced can degrade to O(N))
//     insert / contains / erase ... O(h)   h = height
//     min / max ................... O(h)
//     in_order (visit all) ........ O(N)
//     height ...................... O(N)
// ============================================================================

#include <cstddef>      // std::size_t
#include <functional>   // std::function
#include <vector>       // std::vector

namespace ds {

template <typename T>
class BST {
public:
    BST() : root_(nullptr), size_(0) {}

    BST(const BST& other) : root_(nullptr), size_(0) {
        copy_from(other.root_);
    }

    BST& operator=(BST other) {
        swap(other);
        return *this;
    }

    ~BST() { clear(); }

    // ---- size / shape ------------------------------------------------------

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // Height of an empty tree is 0; a single node has height 1.
    std::size_t height() const { return height_of(root_); }

    void clear() {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
    }

    // ---- core operations ---------------------------------------------------

    void insert(const T& value) {
        root_ = insert_node(root_, value);
    }

    bool contains(const T& value) const {
        return find_node(root_, value) != nullptr;
    }

    // Returns true if the value was present and removed.
    bool erase(const T& value) {
        std::size_t before = size_;
        root_ = erase_node(root_, value);
        return size_ < before;
    }

    // Leftmost (minimum) and rightmost (maximum) values.
    const T& min() const { return min_node(root_)->value; }
    const T& max() const { return max_node(root_)->value; }

    // ---- traversal ---------------------------------------------------------

    // Visit every value in sorted (in-order) order.
    void in_order(const std::function<void(const T&)>& visit) const {
        in_order_walk(root_, visit);
    }

    // Collect in-order values into a vector (sorted).
    std::vector<T> in_order() const {
        std::vector<T> out;
        out.reserve(size_);
        in_order([&](const T& v) { out.push_back(v); });
        return out;
    }

    void swap(BST& other) noexcept {
        std::swap(root_, other.root_);
        std::swap(size_, other.size_);
    }

private:
    struct Node {
        T value;
        Node* left;
        Node* right;

        explicit Node(const T& v) : value(v), left(nullptr), right(nullptr) {}
    };

    Node* root_;
    std::size_t size_;

    static std::size_t height_of(const Node* n) {
        if (!n) return 0;
        std::size_t hl = height_of(n->left);
        std::size_t hr = height_of(n->right);
        return 1 + (hl > hr ? hl : hr);
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
        // duplicate: ignore (already present)
        return n;
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

    static Node* max_node(Node* n) {
        while (n && n->right) n = n->right;
        return n;
    }

    Node* erase_node(Node* n, const T& value) {
        if (!n) return nullptr;

        if (value < n->value) {
            n->left = erase_node(n->left, value);
        } else if (n->value < value) {
            n->right = erase_node(n->right, value);
        } else {
            // Found the target — handle the three cases.
            if (!n->left && !n->right) {
                // Case 1: leaf.
                delete n;
                --size_;
                return nullptr;
            }
            if (!n->left) {
                // Case 2a: only right child.
                Node* child = n->right;
                delete n;
                --size_;
                return child;
            }
            if (!n->right) {
                // Case 2b: only left child.
                Node* child = n->left;
                delete n;
                --size_;
                return child;
            }
            // Case 3: two children — swap with in-order successor.
            Node* succ = min_node(n->right);
            n->value = succ->value;
            n->right = erase_node(n->right, succ->value);
        }
        return n;
    }

    static void in_order_walk(const Node* n, const std::function<void(const T&)>& visit) {
        if (!n) return;
        in_order_walk(n->left, visit);
        visit(n->value);
        in_order_walk(n->right, visit);
    }
};

}  // namespace ds

#endif  // DS_BINARY_SEARCH_TREE_HPP
