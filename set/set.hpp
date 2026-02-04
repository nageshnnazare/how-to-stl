#ifndef SET_HPP
#define SET_HPP

#include <cstddef>      // for size_t
#include <utility>      // for pair
#include <functional>   // for less
#include <initializer_list>

/**
 * @brief Custom implementation of std::set using Red-Black Tree
 * 
 * Set is an ordered container of unique elements.
 * 
 * Key characteristics:
 * - Elements are sorted (default: ascending order)
 * - No duplicate elements
 * - O(log n) insertion, deletion, and search
 * - Bidirectional iterators
 * - Implemented using Red-Black Tree for balanced operations
 */

template<typename T, typename Compare = std::less<T>>
class Set {
private:
    // Red-Black Tree node colors
    enum Color { RED, BLACK };
    
    // Node structure
    struct Node {
        T value;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        
        Node(const T& val, Color c = RED)
            : value(val), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
    };
    
    Node* root_;
    Node* nil_;  // Sentinel node (replaces nullptr)
    size_t size_;
    Compare comp_;
    
    /**
     * @brief Initialize nil sentinel
     */
    void init_nil() {
        nil_ = new Node(T(), BLACK);
        nil_->left = nil_;
        nil_->right = nil_;
        nil_->parent = nil_;
        root_ = nil_;
    }
    
    /**
     * @brief Left rotation around node x
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
     * @brief Right rotation around node x
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
     * @brief Fix Red-Black Tree properties after insertion
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
     * @brief Transplant subtree
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
     * @brief Find minimum node in subtree
     */
    Node* minimum(Node* node) const {
        while (node->left != nil_) {
            node = node->left;
        }
        return node;
    }
    
    /**
     * @brief Find maximum node in subtree
     */
    Node* maximum(Node* node) const {
        while (node->right != nil_) {
            node = node->right;
        }
        return node;
    }
    
    /**
     * @brief Fix Red-Black Tree properties after deletion
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
     * @brief Find node with given value
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
     * @brief Deep copy tree
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
     * @brief Delete tree recursively
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
     * @brief Insert element
     * @return pair of iterator and bool (true if inserted, false if already exists)
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
     * @brief Erase element by value
     * @return number of elements erased (0 or 1)
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
     * @brief Erase element by iterator
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
    
    /**
     * @brief Find element
     * @return iterator to element, or end() if not found
     */
    iterator find(const T& value) {
        Node* node = find_node(value);
        return iterator(node, nil_, this);
    }
    
    const_iterator find(const T& value) const {
        Node* node = find_node(value);
        return const_iterator(node, nil_, this);
    }
    
    /**
     * @brief Count occurrences of value (0 or 1 for set)
     */
    size_type count(const T& value) const {
        return find_node(value) != nil_ ? 1 : 0;
    }
    
    /**
     * @brief Check if value exists
     */
    bool contains(const T& value) const {
        return find_node(value) != nil_;
    }
};

#endif // SET_HPP

