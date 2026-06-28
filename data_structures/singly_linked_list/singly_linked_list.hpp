#ifndef DS_SINGLY_LINKED_LIST_HPP
#define DS_SINGLY_LINKED_LIST_HPP

// ============================================================================
//  SinglyLinkedList<T> -- a textbook singly-linked list
// ============================================================================
//
// THE IDEA
// --------
// Each element lives in a *node* on the heap. A node stores a value and a
// pointer to the next node. The list keeps a `head_` pointer to the first
// node (or nullptr when empty). There is no random access: to reach index i
// you walk i links from the head.
//
// PICTURE
//
//     head
//       |
//       v
//     +---+---+     +---+---+     +---+----+
//     | a | * | --> | b | * | --> | c |null|
//     +---+---+     +---+---+     +---+----+
//
// COMPLEXITY
//     push_front / pop_front ... O(1)
//     push_back ................ O(N)   (walk to tail; use a tail ptr for O(1))
//     front / size / empty ..... O(1)
//     find / remove ............ O(N)
//     reverse .................. O(N)
//     iteration ................ O(N)
// ============================================================================

#include <cstddef>      // std::size_t

namespace ds {

template <typename T>
class SinglyLinkedList {
    struct Node {
        T value;
        Node* next;
        explicit Node(const T& v) : value(v), next(nullptr) {}
    };

public:
    // ---- construction / destruction ----------------------------------------

    SinglyLinkedList() : head_(nullptr), size_(0) {}

    // Deep copy: walk the other list, node by node.
    SinglyLinkedList(const SinglyLinkedList& other) : head_(nullptr), size_(0) {
        for (const T& v : other) push_back(v);
    }

    SinglyLinkedList& operator=(const SinglyLinkedList& other) {
        if (this != &other) {
            clear();
            for (const T& v : other) push_back(v);
        }
        return *this;
    }

    ~SinglyLinkedList() { clear(); }

    // ---- size / access -----------------------------------------------------

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T& front() { return head_->value; }
    const T& front() const { return head_->value; }

    // ---- modifiers ---------------------------------------------------------

    // New node becomes the head.
    void push_front(const T& value) {
        Node* n = new Node(value);
        n->next = head_;
        head_ = n;
        ++size_;
    }

    // Walk to the tail, then append.
    void push_back(const T& value) {
        Node* n = new Node(value);
        if (!head_) {
            head_ = n;
        } else {
            Node* cur = head_;
            while (cur->next) cur = cur->next;
            cur->next = n;
        }
        ++size_;
    }

    // Remove the head node.
    void pop_front() {
        if (!head_) return;
        Node* old = head_;
        head_ = head_->next;
        delete old;
        --size_;
    }

    // Linear scan; returns true if any node holds `value`.
    bool find(const T& value) const {
        for (const T& v : *this)
            if (v == value) return true;
        return false;
    }

    // Remove the first node whose value matches (if any).
    void remove(const T& value) {
        if (!head_) return;
        if (head_->value == value) {
            pop_front();
            return;
        }
        Node* cur = head_;
        while (cur->next) {
            if (cur->next->value == value) {
                Node* doomed = cur->next;
                cur->next = doomed->next;
                delete doomed;
                --size_;
                return;
            }
            cur = cur->next;
        }
    }

    // Reverse links in place by rewiring next pointers.
    //
    //   before:  head -> a -> b -> c -> nullptr
    //   after:   head -> c -> b -> a -> nullptr
    void reverse() {
        Node* prev = nullptr;
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next;
            cur->next = prev;
            prev = cur;
            cur = nxt;
        }
        head_ = prev;
    }

    // Free every node and reset to empty.
    void clear() {
        while (head_) {
            Node* nxt = head_->next;
            delete head_;
            head_ = nxt;
        }
        size_ = 0;
    }

    // ---- iteration ---------------------------------------------------------

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
    std::size_t size_;
};

}  // namespace ds

#endif  // DS_SINGLY_LINKED_LIST_HPP
