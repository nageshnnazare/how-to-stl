#ifndef MAP_HPP
#define MAP_HPP

#include <cstddef>
#include <utility>
#include <functional>
#include <initializer_list>

/**
 * @brief Custom implementation of std::map using Red-Black Tree
 * 
 * Map is an ordered container of key-value pairs with unique keys.
 * 
 * Key characteristics:
 * - Keys are sorted (default: ascending order)
 * - No duplicate keys
 * - O(log n) insertion, deletion, and search
 * - Bidirectional iterators
 * - Implemented using Red-Black Tree
 */

template<typename Key, typename T, typename Compare = std::less<Key>>
class Map {
private:
    enum Color { RED, BLACK };
    
    // Node structure (stores key-value pair)
    struct Node {
        std::pair<const Key, T> value;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        
        Node(const Key& k, const T& v, Color c = RED)
            : value(k, v), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
        
        Node(const std::pair<Key, T>& p, Color c = RED)
            : value(p.first, p.second), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
    };
    
    Node* root_;
    Node* nil_;
    size_t size_;
    Compare comp_;
    
    void init_nil() {
        nil_ = new Node(Key(), T(), BLACK);
        nil_->left = nil_;
        nil_->right = nil_;
        nil_->parent = nil_;
        root_ = nil_;
    }
    
    void rotate_left(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nil_) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) root_ = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }
    
    void rotate_right(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != nil_) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) root_ = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }
    
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
    
    void transplant(Node* u, Node* v) {
        if (u->parent == nil_) root_ = v;
        else if (u == u->parent->left) u->parent->left = v;
        else u->parent->right = v;
        v->parent = u->parent;
    }
    
    Node* minimum(Node* node) const {
        while (node->left != nil_) node = node->left;
        return node;
    }
    
    Node* maximum(Node* node) const {
        while (node->right != nil_) node = node->right;
        return node;
    }
    
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
    
    Node* find_node(const Key& key) const {
        Node* current = root_;
        while (current != nil_) {
            if (comp_(key, current->value.first)) {
                current = current->left;
            } else if (comp_(current->value.first, key)) {
                current = current->right;
            } else {
                return current;
            }
        }
        return nil_;
    }
    
    Node* copy_tree(Node* node, Node* parent, Node* other_nil) {
        if (node == other_nil) return nil_;
        Node* new_node = new Node(node->value.first, node->value.second, node->color);
        new_node->parent = parent;
        new_node->left = copy_tree(node->left, new_node, other_nil);
        new_node->right = copy_tree(node->right, new_node, other_nil);
        return new_node;
    }
    
    void delete_tree(Node* node) {
        if (node != nil_) {
            delete_tree(node->left);
            delete_tree(node->right);
            delete node;
        }
    }

public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;
    
    // Iterator class
    class iterator {
    private:
        Node* node_;
        Node* nil_;
        const Map* map_;
        
        friend class Map;
        iterator(Node* n, Node* nil, const Map* m) : node_(n), nil_(nil), map_(m) {}
        
    public:
        iterator() : node_(nullptr), nil_(nullptr), map_(nullptr) {}
        
        value_type& operator*() { return node_->value; }
        const value_type& operator*() const { return node_->value; }
        value_type* operator->() { return &node_->value; }
        const value_type* operator->() const { return &node_->value; }
        
        iterator& operator++() {
            if (node_->right != nil_) {
                node_ = map_->minimum(node_->right);
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
        
        bool operator==(const iterator& other) const { return node_ == other.node_; }
        bool operator!=(const iterator& other) const { return node_ != other.node_; }
    };
    
    using const_iterator = iterator;
    
    // Constructors
    Map() : root_(nullptr), nil_(nullptr), size_(0) {
        init_nil();
    }
    
    Map(std::initializer_list<value_type> init) : Map() {
        for (const auto& p : init) {
            insert(p);
        }
    }
    
    Map(const Map& other) : Map() {
        root_ = copy_tree(other.root_, nil_, other.nil_);
        size_ = other.size_;
    }
    
    Map(Map&& other) noexcept
        : root_(other.root_), nil_(other.nil_), size_(other.size_), comp_(other.comp_) {
        other.root_ = nullptr;
        other.nil_ = nullptr;
        other.size_ = 0;
        other.init_nil();
    }
    
    ~Map() {
        clear();
        delete nil_;
    }
    
    // Assignment
    Map& operator=(const Map& other) {
        if (this != &other) {
            clear();
            root_ = copy_tree(other.root_, nil_, other.nil_);
            size_ = other.size_;
        }
        return *this;
    }
    
    Map& operator=(Map&& other) noexcept {
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
    
    // Element access
    T& operator[](const Key& key) {
        Node* node = find_node(key);
        if (node != nil_) {
            return node->value.second;
        }
        auto result = insert({key, T()});
        return result.first->second;
    }
    
    T& at(const Key& key) {
        Node* node = find_node(key);
        if (node == nil_) {
            throw std::out_of_range("Map::at: key not found");
        }
        return node->value.second;
    }
    
    const T& at(const Key& key) const {
        Node* node = find_node(key);
        if (node == nil_) {
            throw std::out_of_range("Map::at: key not found");
        }
        return node->value.second;
    }
    
    // Iterators
    iterator begin() {
        if (root_ == nil_) return iterator(nil_, nil_, this);
        return iterator(minimum(root_), nil_, this);
    }
    
    const_iterator begin() const {
        if (root_ == nil_) return const_iterator(nil_, nil_, this);
        return const_iterator(minimum(root_), nil_, this);
    }
    
    iterator end() {
        return iterator(nil_, nil_, this);
    }
    
    const_iterator end() const {
        return const_iterator(nil_, nil_, this);
    }
    
    // Capacity
    bool empty() const noexcept { return size_ == 0; }
    size_type size() const noexcept { return size_; }
    
    // Modifiers
    void clear() {
        delete_tree(root_);
        root_ = nil_;
        size_ = 0;
    }
    
    std::pair<iterator, bool> insert(const value_type& value) {
        Node* parent = nil_;
        Node* current = root_;
        
        while (current != nil_) {
            parent = current;
            if (comp_(value.first, current->value.first)) {
                current = current->left;
            } else if (comp_(current->value.first, value.first)) {
                current = current->right;
            } else {
                return {iterator(current, nil_, this), false};
            }
        }
        
        Node* new_node = new Node(value.first, value.second, RED);
        new_node->left = nil_;
        new_node->right = nil_;
        new_node->parent = parent;
        
        if (parent == nil_) {
            root_ = new_node;
        } else if (comp_(value.first, parent->value.first)) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        
        insert_fixup(new_node);
        ++size_;
        return {iterator(new_node, nil_, this), true};
    }
    
    size_type erase(const Key& key) {
        Node* z = find_node(key);
        if (z == nil_) return 0;
        
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
    
    // Lookup
    iterator find(const Key& key) {
        Node* node = find_node(key);
        return iterator(node, nil_, this);
    }
    
    const_iterator find(const Key& key) const {
        Node* node = find_node(key);
        return const_iterator(node, nil_, this);
    }
    
    size_type count(const Key& key) const {
        return find_node(key) != nil_ ? 1 : 0;
    }
    
    bool contains(const Key& key) const {
        return find_node(key) != nil_;
    }
};

#endif // MAP_HPP

