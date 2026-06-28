#ifndef DS_DOUBLY_LINKED_LIST_HPP
#define DS_DOUBLY_LINKED_LIST_HPP

// ============================================================================
//  DoublyLinkedList<T> -- a textbook doubly-linked list
// ============================================================================
//
// THE IDEA
// --------
// Like a singly-linked list, but each node also stores a pointer to the
// *previous* node. The list keeps both `head_` and `tail_` so we can add or
// remove at either end in O(1).
//
// PICTURE
//
//     nullptr <--+---+---+
//                |prev| a |next| <--+---+---+
//                +---+---+          |prev| b |next| --> nullptr
//                                   +---+---+
//     head ----------------^  tail --^
//
// COMPLEXITY
//     push_front / push_back / pop_front / pop_back ... O(1)
//     front / back / size / empty ...................... O(1)
//     iteration ........................................ O(N)
// ============================================================================

#include <cstddef>      // std::size_t

namespace ds {

template <typename T>
class DoublyLinkedList {
    struct Node {
        T value;
        Node* prev;
        Node* next;
        explicit Node(const T& v) : value(v), prev(nullptr), next(nullptr) {}
    };

public:
    // ---- construction / destruction ----------------------------------------

    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}

    // Deep copy: walk the other list, node by node.
    DoublyLinkedList(const DoublyLinkedList& other)
        : head_(nullptr), tail_(nullptr), size_(0) {
        for (const T& v : other) push_back(v);
    }

    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {
            clear();
            for (const T& v : other) push_back(v);
        }
        return *this;
    }

    ~DoublyLinkedList() { clear(); }

    // ---- size / access -----------------------------------------------------

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T& front() { return head_->value; }
    const T& front() const { return head_->value; }
    T& back() { return tail_->value; }
    const T& back() const { return tail_->value; }

    // ---- modifiers ---------------------------------------------------------

    // Insert before head.
    void push_front(const T& value) {
        Node* n = new Node(value);
        if (!head_) {
            head_ = tail_ = n;
        } else {
            n->next = head_;
            head_->prev = n;
            head_ = n;
        }
        ++size_;
    }

    // Insert after tail.
    void push_back(const T& value) {
        Node* n = new Node(value);
        if (!tail_) {
            head_ = tail_ = n;
        } else {
            n->prev = tail_;
            tail_->next = n;
            tail_ = n;
        }
        ++size_;
    }

    // Remove the head node.
    void pop_front() {
        if (!head_) return;
        Node* old = head_;
        head_ = head_->next;
        if (head_) head_->prev = nullptr;
        else tail_ = nullptr;
        delete old;
        --size_;
    }

    // Remove the tail node.
    void pop_back() {
        if (!tail_) return;
        Node* old = tail_;
        tail_ = tail_->prev;
        if (tail_) tail_->next = nullptr;
        else head_ = nullptr;
        delete old;
        --size_;
    }

    // Free every node and reset to empty.
    void clear() {
        while (head_) {
            Node* nxt = head_->next;
            delete head_;
            head_ = nxt;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    // ---- iteration (forward) -----------------------------------------------

    class Iterator {
    public:
        Iterator() : node_(nullptr) {}
        explicit Iterator(Node* n) : node_(n) {}

        T& operator*() { return node_->value; }
        const T& operator*() const { return node_->value; }

        Iterator& operator++() { node_ = node_->next; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); ++(*this); return tmp; }

        bool operator==(const Iterator& o) const { return node_ == o.node_; }
        bool operator!=(const Iterator& o) const { return node_ != o.node_; }

    private:
        Node* node_;
    };

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
    Iterator begin() const { return Iterator(head_); }
    Iterator end() const { return Iterator(nullptr); }

private:
    Node* head_;
    Node* tail_;
    std::size_t size_;
};

}  // namespace ds

#endif  // DS_DOUBLY_LINKED_LIST_HPP
