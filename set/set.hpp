#ifndef SET_HPP
#define SET_HPP

#include <cstddef>      // for size_t
#include <utility>      // for pair
#include <functional>   // for less
#include <initializer_list>

// ============================================================================
//  Set<T> -- a hand-rolled std::set (ordered unique elements, Red-Black Tree)
// ============================================================================
//
// WHAT IT IS
// ----------
// A Set stores every element at most once, kept in sorted order (default:
// ascending via std::less<T>). Lookups, inserts, and erases are O(log n)
// because the backing structure is a self-balancing Red-Black binary search tree.
//
// THE FOUR FIELDS
// ---------------
//     root_   -> top of the BST (points at nil_ when empty)
//     nil_    -> sentinel "null" node (BLACK); all missing children point here
//     size_   -> number of live elements in the tree
//     comp_   -> strict weak ordering (default: std::less<T>)
//
// TREE LAYOUT (example: {1, 3, 5, 7, 9} inserted in that order)
// ---------------------------------------------------------------
//
//     Set object (stack)              Red-Black tree (B = BLACK, r = RED)
//     ┌──────────────┐
//     │ root_   ●────┼──▶        (3:B)
//     │ nil_    ●────┼──┐       /       \
//     │ size_ = 5    │  │    (1:B)     (7:B)
//     │ comp_        │  │       \       /   \
//     └──────────────┘  │      (nil) (5:r) (9:B)
//                       │                  /   \
//                       └────────────▶ (nil) (nil)
//
//     nil_ (sentinel, always BLACK):
//     ┌──────────────────────────────────────┐
//     │ value = default T()  (never read)    │
//     │ color = BLACK                        │
//     │ left = right = parent = self (nil_)  │  ◀── loops to itself
//     └──────────────────────────────────────┘
//
// In-order traversal (left, node, right) visits elements in sorted order.
// Iterators walk this order; end() is an iterator sitting on nil_.
//
// THE FIVE RED-BLACK INVARIANTS (why height stays O(log n))
// ---------------------------------------------------------
//   1. Every node is RED or BLACK.
//   2. The root is BLACK.
//   3. Every leaf is the nil_ sentinel (no real nullptr children).
//   4. RED nodes have only BLACK children (no two consecutive REDs on a path).
//   5. Every root-to-nil path has the same number of BLACK nodes ("black-height").
//
// Invariant 4 + 5 together bound tree height to at most 2*log₂(n+1): on any
// path, at most half the nodes can be RED, so the longest path is ~2× the
// shortest. Balanced black-height ⇒ no path is longer than O(log n).
//
// INSERT OVERVIEW
// ---------------
//   (1) BST insert as a new RED leaf (standard binary-search descent).
//   (2) insert_fixup: recolor or rotate to restore invariants 2–5.
//   (3) Force root BLACK (invariant 2).
//
// DELETE OVERVIEW (high level)
// ----------------------------
//   (1) BST delete (0/1/2-child cases; 2-child uses successor swap).
//   (2) If a BLACK node was removed, an "extra black" deficit may appear on
//       some root-to-leaf path; delete_fixup recolors/rotates until balanced.
//
// Key characteristics:
// - Sorted unique elements, bidirectional iterators
// - O(log n) insert / erase / find via Red-Black balancing
// - nil_ sentinel eliminates nullptr edge cases in rotations and fixups
// - RAII: destructor deletes every real node plus the sentinel
// ============================================================================

template<typename T, typename Compare = std::less<T>>
class Set {
private:
    enum Color { RED, BLACK };  // node color for Red-Black invariants
    
    struct Node {
        T value;           // the element stored in this node
        Color color;       // RED (new inserts) or BLACK (stable nodes)
        Node* left;        // left child, or nil_ if absent
        Node* right;       // right child, or nil_ if absent
        Node* parent;      // parent link (nil_ when node is root and has no parent)
        
        Node(const T& val, Color c = RED)
            : value(val), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
    };
    
    Node* root_;     // pointer to tree root; equals nil_ when the set is empty
    Node* nil_;    // sentinel leaf: BLACK, self-referential, replaces nullptr
    size_t size_;  // count of elements (excludes the sentinel)
    Compare comp_; // strict weak ordering predicate for placement and search
    
    /**
     * @brief Create the nil_ sentinel and point root_ at it.
     *
     * The sentinel is a real heap node (not nullptr) so every "missing child"
     * is a valid Node* we can read .color from during fixups. It points to
     * itself on left/right/parent, and is always BLACK.
     *
     *     after init_nil():
     *         root_ ──▶ nil_ ◀──┐
     *                     │     │  left, right, parent all loop back
     *                     └─────┘
     */
    void init_nil() {
        nil_ = new Node(T(), BLACK);
        nil_->left = nil_;
        nil_->right = nil_;
        nil_->parent = nil_;
        root_ = nil_;
    }
    
    /**
     * @brief Left rotation around pivot x (x moves down, its right child rises).
     *
     * Used when a RED child and RED parent form a "triangle" that must become
     * a "line" before the final rotation, and during delete_fixup.
     *
     * BEFORE (x is pivot; β is x's left subtree of y, moved under x):
     *
     *         P                      P
     *         │                      │
     *         x                      y
     *        / \                    / \
     *       α   y        ──▶      x   γ
     *          / \                / \
     *         β   γ              α   β
     *
     * Steps:
     *   (1) y = x->right; hang β on x->right (reparent β if not nil_).
     *   (2) y->parent = x->parent; rewire P's child pointer to y.
     *   (3) y->left = x; x->parent = y.
     */
    void rotate_left(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        
        if (y->left != nil_) {
            y->left->parent = x;
        }
        
        y->parent = x->parent;
        
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        
        y->left = x;
        x->parent = y;
    }
    
    /**
     * @brief Right rotation around pivot x (mirror of rotate_left).
     *
     * BEFORE:
     *
     *         P                      P
     *         │                      │
     *         x                      y
     *        / \                    / \
     *       y   γ        ──▶      β   x
     *      / \                        / \
     *     β   α                      α   γ
     *
     * β (y's right child) becomes x's left child; y replaces x under P.
     */
    void rotate_right(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        
        if (y->right != nil_) {
            y->right->parent = x;
        }
        
        y->parent = x->parent;
        
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
        
        y->right = x;
        x->parent = y;
    }
    
    /**
     * @brief Restore Red-Black invariants after inserting RED leaf z.
     *
     * Only violations: z is RED and z->parent may be RED (invariant 4).
     * Loop while parent is RED; z walks up after recolor cases.
     *
     * CASE A — uncle y is RED (recolor, climb):
     *
     *         gp:B                gp:r  ◀── was BLACK, now RED (problem moves up)
     *        /    \              /    \
     *     parent:r  y:r   ──▶  parent:B  y:B
     *        /                 /
     *       z:r               z:r
     *
     *     Paint parent and uncle BLACK, grandparent RED, set z = grandparent.
     *
     * CASE B — uncle y is BLACK, z is outer child (line case → one rotation):
     *
     *         gp:B                 gp:B
     *        /                    /
     *     parent:r               z:B  ◀── parent
     *        \                  /
     *         z:r      ──▶    parent:r
     *
     *     Recolor parent BLACK, grandparent RED, rotate_right(grandparent).
     *
     * CASE C — uncle BLACK, z is inner child (triangle → rotate to line first):
     *
     *         gp:B                gp:B
     *        /                    /
     *     parent:r              parent:r
     *        /                      \
     *       z:r          ──▶         z:r   then apply Case B
     *
     *     rotate_left(parent); z = parent; fall through to Case B.
     *
     * (Symmetric cases when parent is a right child are mirrored.)
     * Finally force root_->color = BLACK.
     */
    void insert_fixup(Node* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotate_left(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotate_right(z->parent->parent);
                }
            } else {
                Node* y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotate_right(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rotate_left(z->parent->parent);
                }
            }
        }
        root_->color = BLACK;
    }
    
    /**
     * @brief Replace subtree rooted at u with subtree rooted at v.
     *
     * Rewires u->parent's left or right (or root_) to v and sets v->parent.
     * Does not touch u->left / u->right — caller manages those.
     *
     *     before:  parent ──▶ u         after:  parent ──▶ v
     *                         / \                           ...
     *                        ... ...
     */
    void transplant(Node* u, Node* v) {
        if (u->parent == nil_) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }
    
    /**
     * @brief Leftmost (minimum) node in subtree — walk left until nil_.
     *
     * In-order successor of a node with a non-nil right child.
     */
    Node* minimum(Node* node) const {
        while (node->left != nil_) {
            node = node->left;
        }
        return node;
    }
    
    /**
     * @brief Rightmost (maximum) node in subtree — walk right until nil_.
     */
    Node* maximum(Node* node) const {
        while (node->right != nil_) {
            node = node->right;
        }
        return node;
    }
    
    /**
     * @brief Restore black-height after deleting a BLACK node.
     *
     * Concept: removing a BLACK node leaves one root-to-leaf path with one
     * fewer BLACK node than the others — node x carries "extra black". We
     * recolor and rotate until extra black is absorbed and invariants hold.
     *
     *     path with deficit:  root ... x(B) ... nil_
     *                              ▲
     *                         "double-black" until fixed
     *
     * Main cases (x is left child of parent; mirror if x is right):
     *   • Sibling w is RED → rotate parent, recolor, reclassify (w becomes BLACK sibling).
     *   • w BLACK, both nephews BLACK → paint w RED, push deficit to parent.
     *   • w BLACK, far nephew RED → recolor + rotate parent (done).
     *   • w BLACK, near nephew RED only → rotate w, then far-nephew case.
     *
     * Loop until x is root or x is RED; then x->color = BLACK.
     */
    void delete_fixup(Node* x) {
        while (x != root_ && x->color == BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_left(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rotate_right(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    rotate_left(x->parent);
                    x = root_;
                }
            } else {
                Node* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotate_right(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        rotate_left(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rotate_right(x->parent);
                    x = root_;
                }
            }
        }
        x->color = BLACK;
    }
    
    /**
     * @brief BST search for value; return node or nil_ if absent.
     *
     * Descend from root_: go left if value < node, right if node < value,
     * stop on equality or when hitting nil_.
     *
     *     find 5 in tree:  compare at each node, O(log n) depth
     */
    Node* find_node(const T& value) const {
        Node* current = root_;
        while (current != nil_) {
            if (comp_(value, current->value)) {
                current = current->left;
            } else if (comp_(current->value, value)) {
                current = current->right;
            } else {
                return current;
            }
        }
        return nil_;
    }
    
    /**
     * @brief Deep-copy subtree rooted at node (for copy ctor / assignment).
     *
     * Recursively clones structure and colors; maps other_nil leaves to nil_.
     */
    Node* copy_tree(Node* node, Node* parent, Node* other_nil) {
        if (node == other_nil) {
            return nil_;
        }
        
        Node* new_node = new Node(node->value, node->color);
        new_node->parent = parent;
        new_node->left = copy_tree(node->left, new_node, other_nil);
        new_node->right = copy_tree(node->right, new_node, other_nil);
        
        return new_node;
    }
    
    /**
     * @brief Post-order delete of all real nodes (skips nil_ sentinel).
     */
    void delete_tree(Node* node) {
        if (node != nil_) {
            delete_tree(node->left);
            delete_tree(node->right);
            delete node;
        }
    }

public:
    // Type aliases
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    
    // Iterator class
    class iterator {
    private:
        Node* node_;
        Node* nil_;
        const Set* set_;
        
        friend class Set;
        
        iterator(Node* n, Node* nil, const Set* s) : node_(n), nil_(nil), set_(s) {}
        
    public:
        iterator() : node_(nullptr), nil_(nullptr), set_(nullptr) {}
        
        const T& operator*() const { return node_->value; }
        const T* operator->() const { return &node_->value; }
        
        /**
         * @brief In-order successor: next larger element.
         *
         * If right subtree exists → minimum of right (leftmost in right subtree).
         * Else climb until we came from a left child (first ancestor greater than us).
         *
         *     ++it at node 5:          ++it at node 7 (no right):
         *         5                       7
         *        / \                     /
         *       3   7        → 7        5   (climb to parent 9...)
         *          /
         *         6
         */
        iterator& operator++() {
            if (node_->right != nil_) {
                node_ = set_->minimum(node_->right);
            } else {
                Node* parent = node_->parent;
                while (parent != nil_ && node_ == parent->right) {
                    node_ = parent;
                    parent = parent->parent;
                }
                node_ = parent;
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        /**
         * @brief In-order predecessor: previous smaller element.
         *
         * If at end() (nil_), jump to maximum of whole tree.
         * Else if left subtree → maximum of left; else climb from right children.
         */
        iterator& operator--() {
            if (node_ == nil_) {
                node_ = set_->maximum(set_->root_);
            } else if (node_->left != nil_) {
                node_ = set_->maximum(node_->left);
            } else {
                Node* parent = node_->parent;
                while (parent != nil_ && node_ == parent->left) {
                    node_ = parent;
                    parent = parent->parent;
                }
                node_ = parent;
            }
            return *this;
        }
        
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        bool operator==(const iterator& other) const {
            return node_ == other.node_;
        }
        
        bool operator!=(const iterator& other) const {
            return node_ != other.node_;
        }
    };
    
    using const_iterator = iterator;
    
    // ============================================================================
    // CONSTRUCTORS
    // ============================================================================
    
    /**
     * @brief Default constructor
     */
    Set() : root_(nullptr), nil_(nullptr), size_(0) {
        init_nil();
    }
    
    /**
     * @brief Construct from initializer list
     */
    Set(std::initializer_list<T> init) : Set() {
        for (const auto& val : init) {
            insert(val);
        }
    }
    
    /**
     * @brief Copy constructor
     */
    Set(const Set& other) : Set() {
        root_ = copy_tree(other.root_, nil_, other.nil_);
        size_ = other.size_;
    }
    
    /**
     * @brief Move constructor
     */
    Set(Set&& other) noexcept
        : root_(other.root_), nil_(other.nil_), size_(other.size_), comp_(other.comp_) {
        other.root_ = nullptr;
        other.nil_ = nullptr;
        other.size_ = 0;
        other.init_nil();
    }
    
    /**
     * @brief Destructor
     */
    ~Set() {
        clear();
        delete nil_;
    }
    
    // ============================================================================
    // ASSIGNMENT OPERATORS
    // ============================================================================
    
    /**
     * @brief Copy assignment
     */
    Set& operator=(const Set& other) {
        if (this != &other) {
            clear();
            root_ = copy_tree(other.root_, nil_, other.nil_);
            size_ = other.size_;
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     */
    Set& operator=(Set&& other) noexcept {
        if (this != &other) {
            clear();
            delete nil_;
            
            root_ = other.root_;
            nil_ = other.nil_;
            size_ = other.size_;
            comp_ = other.comp_;
            
            other.root_ = nullptr;
            other.nil_ = nullptr;
            other.size_ = 0;
            other.init_nil();
        }
        return *this;
    }
    
    // ============================================================================
    // ITERATORS
    // ============================================================================
    
    iterator begin() {
        if (root_ == nil_) {
            return iterator(nil_, nil_, this);
        }
        return iterator(minimum(root_), nil_, this);
    }
    
    const_iterator begin() const {
        if (root_ == nil_) {
            return const_iterator(nil_, nil_, this);
        }
        return const_iterator(minimum(root_), nil_, this);
    }
    
    iterator end() {
        return iterator(nil_, nil_, this);
    }
    
    const_iterator end() const {
        return const_iterator(nil_, nil_, this);
    }
    
    // ============================================================================
    // CAPACITY
    // ============================================================================
    
    bool empty() const noexcept {
        return size_ == 0;
    }
    
    size_type size() const noexcept {
        return size_;
    }
    
    // ============================================================================
    // MODIFIERS
    // ============================================================================
    
    /**
     * @brief Clear all elements
     */
    void clear() {
        delete_tree(root_);
        root_ = nil_;
        size_ = 0;
    }
    
    /**
     * @brief Insert value if not already present; rebalance with insert_fixup.
     *
     *   (1) BST descent to nil_ leaf position (track parent).
     *   (2) If equal key found → return {iterator, false}.
     *   (3) Allocate RED leaf, link under parent (or become root).
     *   (4) insert_fixup(new_node); ++size_; return {iterator, true}.
     *
     *     insert 4 into tree rooted at 3:
     *         (3)              (3)
     *        /     → add →    / \
     *       2                 2   (4:r)  → fixup if needed
     */
    std::pair<iterator, bool> insert(const T& value) {
        Node* parent = nil_;
        Node* current = root_;
        
        // Find insertion position
        while (current != nil_) {
            parent = current;
            if (comp_(value, current->value)) {
                current = current->left;
            } else if (comp_(current->value, value)) {
                current = current->right;
            } else {
                // Value already exists
                return {iterator(current, nil_, this), false};
            }
        }
        
        // Create new node
        Node* new_node = new Node(value, RED);
        new_node->left = nil_;
        new_node->right = nil_;
        new_node->parent = parent;
        
        // Insert node
        if (parent == nil_) {
            root_ = new_node;
        } else if (comp_(value, parent->value)) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        
        // Fix Red-Black Tree properties
        insert_fixup(new_node);
        
        ++size_;
        return {iterator(new_node, nil_, this), true};
    }
    
    /**
     * @brief Erase by value (CLRS delete); delete_fixup if removed node was BLACK.
     *
     * Three BST cases for node z:
     *   • 0 or 1 child: transplant child up, delete z.
     *   • 2 children: swap role with successor y = min(z->right), then delete y.
     *
     * If y_original_color was BLACK, the black-height on path through x may
     * be short by one → delete_fixup(x).
     */
    size_type erase(const T& value) {
        Node* z = find_node(value);
        if (z == nil_) {
            return 0;
        }
        
        Node* y = z;
        Node* x;
        Color y_original_color = y->color;
        
        if (z->left == nil_) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil_) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;
            
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        
        if (y_original_color == BLACK) {
            delete_fixup(x);
        }
        
        delete z;
        --size_;
        return 1;
    }
    
    /**
     * @brief Erase at pos; return iterator to successor (STL erase idiom).
     */
    iterator erase(iterator pos) {
        iterator next = pos;
        ++next;
        erase(*pos);
        return next;
    }
    
    // ============================================================================
    // LOOKUP
    // ============================================================================
    
    /** @brief Iterator to element or end() (nil_) if not found. */
    iterator find(const T& value) {
        Node* node = find_node(value);
        return iterator(node, nil_, this);
    }
    
    const_iterator find(const T& value) const {
        Node* node = find_node(value);
        return const_iterator(node, nil_, this);
    }
    
    /** @brief 0 or 1 — set never holds duplicates. */
    size_type count(const T& value) const {
        return find_node(value) != nil_ ? 1 : 0;
    }
    
    /** @brief Membership test via find_node. */
    bool contains(const T& value) const {
        return find_node(value) != nil_;
    }
};

#endif // SET_HPP

