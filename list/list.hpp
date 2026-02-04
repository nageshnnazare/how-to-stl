#ifndef LIST_HPP
#define LIST_HPP

#include <cstddef>
#include <utility>
#include <initializer_list>

/**
 * @brief Doubly linked list implementation
 * O(1) insert/erase with iterator, no random access
 */
template<typename T>
class List {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
        Node(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}
    };
    
    Node* head_;
    Node* tail_;
    size_t size_;
    
public:
    using value_type = T;
    using size_type = std::size_t;
    
    class iterator {
    private:
        Node* node_;
        friend class List;
        iterator(Node* n) : node_(n) {}
    public:
        iterator() : node_(nullptr) {}
        T& operator*() { return node_->data; }
        const T& operator*() const { return node_->data; }
        T* operator->() { return &node_->data; }
        const T* operator->() const { return &node_->data; }
        
        iterator& operator++() { node_ = node_->next; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
        iterator& operator--() { node_ = node_->prev; return *this; }
        iterator operator--(int) { iterator tmp = *this; --(*this); return tmp; }
        
        bool operator==(const iterator& other) const { return node_ == other.node_; }
        bool operator!=(const iterator& other) const { return node_ != other.node_; }
    };
    
    List() : head_(nullptr), tail_(nullptr), size_(0) {}
    
    List(std::initializer_list<T> init) : List() {
        for (const auto& val : init) push_back(val);
    }
    
    ~List() { clear(); }
    
    List(const List& other) : List() {
        for (const auto& val : other) push_back(val);
    }
    
    List(List&& other) noexcept 
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = other.tail_ = nullptr;
        other.size_ = 0;
    }
    
    List& operator=(const List& other) {
        if (this != &other) {
            clear();
            for (const auto& val : other) push_back(val);
        }
        return *this;
    }
    
    List& operator=(List&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            other.head_ = other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    iterator begin() { return iterator(head_); }
    iterator end() { return iterator(nullptr); }
    const iterator begin() const { return iterator(head_); }
    const iterator end() const { return iterator(nullptr); }
    
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    
    T& front() { return head_->data; }
    const T& front() const { return head_->data; }
    T& back() { return tail_->data; }
    const T& back() const { return tail_->data; }
    
    void push_front(const T& value) {
        Node* new_node = new Node(value);
        if (empty()) {
            head_ = tail_ = new_node;
        } else {
            new_node->next = head_;
            head_->prev = new_node;
            head_ = new_node;
        }
        ++size_;
    }
    
    void push_back(const T& value) {
        Node* new_node = new Node(value);
        if (empty()) {
            head_ = tail_ = new_node;
        } else {
            new_node->prev = tail_;
            tail_->next = new_node;
            tail_ = new_node;
        }
        ++size_;
    }
    
    void pop_front() {
        if (empty()) return;
        Node* old = head_;
        head_ = head_->next;
        if (head_) head_->prev = nullptr;
        else tail_ = nullptr;
        delete old;
        --size_;
    }
    
    void pop_back() {
        if (empty()) return;
        Node* old = tail_;
        tail_ = tail_->prev;
        if (tail_) tail_->next = nullptr;
        else head_ = nullptr;
        delete old;
        --size_;
    }
    
    void clear() {
        while (!empty()) pop_front();
    }
};

#endif // LIST_HPP
