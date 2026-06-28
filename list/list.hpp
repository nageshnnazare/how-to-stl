#ifndef LIST_HPP
#define LIST_HPP

#include <cstddef>
#include <utility>
#include <initializer_list>

// ============================================================================
//  List<T> -- a hand-rolled doubly-linked list (no random access)
// ============================================================================
//
// WHAT IT IS
// ----------
// A List is a sequence container where each element lives in its own heap-
// allocated Node wired to its neighbors with prev/next pointers. Insertion and
// removal at known positions (via iterator) is O(1) because only pointers are
// rewired — no shifting like vector. There is NO random access: reaching
// element i requires walking i steps from the head.
//
// THE THREE FIELDS
// ----------------
//
//     head_  -> first Node in the chain (nullptr when empty)
//     tail_  -> last Node in the chain (nullptr when empty)
//     size_  -> number of Nodes currently in the list
//
// There is NO sentinel/dummy head node in this implementation. end() is
// represented by a nullptr iterator — not a special sentinel Node.
//
// MEMORY LAYOUT (size_ = 3, values A, B, C)
// -------------------------------------------
//
//     List object (stack)          Heap Nodes (each separately allocated)
//     ┌─────────────────┐
//     │ head_  ●────────┼──┐       ┌──────────┐    ┌──────────┐    ┌──────────┐
//     │ tail_  ●────────┼──┼──┐    │ prev: ∅  │◀───│ prev: ●──┼───▶│ prev: ●──┼──┐
//     │ size_  = 3      │  │  │    │ data: A  │    │ data: B  │    │ data: C  │  │
//     └─────────────────┘  │  └───▶│ next: ●──┼───▶│ next: ●──┼───▶│ next: ∅  │◀─┘
//                          │       └──────────┘    └──────────┘    └──────────┘
//                          └───────────────────────────────────────────────┘
//     end() iterator holds node_ == nullptr (one past tail_->next)
//
// PUSH_BACK — append a new Node at the tail
// ---------------------------------------
//
//     before:  head ─▶ [A] ◀─▶ [B] ◀── tail
//     new Node [C]:
//         tail_->next = C;  C->prev = tail_;  tail_ = C;
//     after:   head ─▶ [A] ◀─▶ [B] ◀─▶ [C] ◀── tail
//
// PUSH_FRONT — mirror at the head side
// ------------------------------------
//
//     before:  head ─▶ [B] ◀── tail
//     new Node [A]:
//         head_->prev = A;  A->next = head_;  head_ = A;
//     after:   head ─▶ [A] ◀─▶ [B] ◀── tail
//
// CACHE BEHAVIOR vs VECTOR
// ------------------------
// Nodes are scattered on the heap — poor spatial locality. Traversing a list
// chases pointers and misses CPU cache lines; vector's contiguous block is
// much faster for sequential scans.
//
// Key characteristics:
// - O(1) push/pop at front and back
// - O(n) access by index (must walk the chain)
// - Stable addresses: Node addresses don't change when neighbors are added
// - Extra memory: 2 pointers per element (prev + next)
// ============================================================================

template<typename T>
class List {
private:
    /**
     * @brief One heap-allocated link in the doubly-linked chain.
     *
     *     ┌──────────────┐
     *     │ prev  ●──────┼──▶ previous Node (or nullptr at head)
     *     │ data  (T)    │
     *     │ next  ●──────┼──▶ next Node (or nullptr at tail)
     *     └──────────────┘
     */
    struct Node {
        T data;           // stored element value
        Node* prev;       // link toward head (nullptr at front)
        Node* next;       // link toward tail (nullptr at back)

        Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
        Node(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}
    };

    Node* head_;      // pointer to first node, or nullptr if empty
    Node* tail_;      // pointer to last node, or nullptr if empty
    size_t size_;     // number of nodes in the list

public:
    using value_type = T;
    using size_type = std::size_t;

    /**
     * @brief Bidirectional iterator holding a Node* (nullptr == end).
     */
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

    /**
     * @brief Default constructor — empty list (head_ = tail_ = nullptr).
     */
    List() : head_(nullptr), tail_(nullptr), size_(0) {}

    List(std::initializer_list<T> init) : List() {
        for (const auto& val : init) push_back(val);
    }

    /**
     * @brief Destructor — walk from head, delete each Node.
     */
    ~List() { clear(); }

    List(const List& other) : List() {
        for (const auto& val : other) push_back(val);
    }

    /**
     * @brief Move constructor — steal head_/tail_/size_ in O(1).
     */
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

    /**
     * @brief Prepend — allocate Node, wire before current head.
     *
     *   empty:     head_ = tail_ = new_node
     *   non-empty: new_node->next = head_; head_->prev = new_node; head_ = new_node
     */
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

    /**
     * @brief Append — allocate Node, wire after current tail.
     *
     *   empty:     head_ = tail_ = new_node
     *   non-empty: tail_->next = new_node; new_node->prev = tail_; tail_ = new_node
     */
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

    /**
     * @brief Remove head — unlink, delete Node, fix tail_ if list becomes empty.
     */
    void pop_front() {
        if (empty()) return;
        Node* old = head_;
        head_ = head_->next;
        if (head_) head_->prev = nullptr;
        else tail_ = nullptr;
        delete old;
        --size_;
    }

    /**
     * @brief Remove tail — unlink, delete Node, fix head_ if list becomes empty.
     */
    void pop_back() {
        if (empty()) return;
        Node* old = tail_;
        tail_ = tail_->prev;
        if (tail_) tail_->next = nullptr;
        else head_ = nullptr;
        delete old;
        --size_;
    }

    /**
     * @brief Destroy all nodes — repeated pop_front until empty.
     */
    void clear() {
        while (!empty()) pop_front();
    }
};

#endif // LIST_HPP
