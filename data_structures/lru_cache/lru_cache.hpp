#ifndef DS_LRU_CACHE_HPP
#define DS_LRU_CACHE_HPP

// ============================================================================
//  LRUCache<K, V> -- a fixed-capacity cache that evicts the Least Recently Used
// ============================================================================
//
// THE IDEA
// --------
// A cache holds at most `capacity` key->value entries. When it is full and a
// NEW key arrives, we must throw something out. "LRU" means we evict whatever
// was *touched least recently* -- the entry nobody has read or written for the
// longest time. Every get() and put() counts as "touching" a key, which makes
// it the Most Recently Used again.
//
// THE TRICK: HASH MAP + DOUBLY LINKED LIST  (both O(1))
// -----------------------------------------------------
// We need two things fast:
//   1. find an entry by key            -> hash map (unordered_map)
//   2. reorder "recency" in O(1)       -> doubly linked list
//
// The list keeps nodes in recency order. The map points key -> that node, so we
// can splice a node to the front (MRU) without scanning.
//
//        MRU (front)                                   LRU (back / evict here)
//        ┌───────┐   ┌───────┐   ┌───────┐   ┌───────┐
//   head │ k=C   │<->│ k=A   │<->│ k=D   │<->│ k=B   │ tail
//        │ v=30  │   │ v=10  │   │ v=40  │   │ v=20  │
//        └───────┘   └───────┘   └───────┘   └───────┘
//             ▲           ▲           ▲           ▲
//        map: C ──────────┘  A ───────┘  D ───────┘  B (map: key -> node*)
//
//   get(A):   find node via map, UNLINK it, RELINK at front  -> A is now MRU
//   put(E):   full? drop tail (B, the LRU) from list AND map, insert E at front
//
// COMPLEXITY
//     get(key) ........ O(1) average   (hash lookup + 6 pointer writes)
//     put(key,value) .. O(1) average   (hash + splice; eviction is O(1))
//     contains ........ O(1) average
//
// WHY A DOUBLY linked list?  Eviction removes the tail and every touch unlinks a
// node from the middle; both need the node's predecessor in O(1), which only a
// doubly linked list gives. The map is what makes "find the node" O(1).
// ============================================================================

#include <cstddef>        // std::size_t
#include <stdexcept>      // std::out_of_range, std::invalid_argument
#include <unordered_map>  // key -> node lookup
#include <utility>        // std::move

namespace ds {

template <typename K, typename V>
class LRUCache {
public:
    // A cache with capacity 0 can never hold anything, which is almost always a
    // bug, so we reject it up front.
    explicit LRUCache(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) throw std::invalid_argument("LRUCache: capacity must be > 0");
    }

    // Rule of five: we own raw Node pointers, so copy/move need real work.
    // For a teaching cache we simply forbid copying and moving to keep the
    // ownership story obvious; clear() in the destructor frees every node.
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    ~LRUCache() { clear(); }

    std::size_t size() const { return map_.size(); }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return map_.empty(); }
    bool contains(const K& key) const { return map_.find(key) != map_.end(); }

    // Look up a key. On a hit the entry becomes Most Recently Used and we return
    // a pointer to its (mutable) value; on a miss we return nullptr. Returning a
    // pointer lets the caller distinguish "missing" from "present but == V()".
    V* get(const K& key) {
        auto it = map_.find(key);
        if (it == map_.end()) return nullptr;  // miss: recency unchanged
        move_to_front(it->second);             // hit: promote to MRU
        return &it->second->value;
    }

    // Insert or update. Updating an existing key refreshes its value and makes
    // it MRU. Inserting a new key into a full cache first evicts the LRU entry.
    void put(const K& key, const V& value) {
        auto it = map_.find(key);
        if (it != map_.end()) {                // existing key: update + promote
            it->second->value = value;
            move_to_front(it->second);
            return;
        }
        if (map_.size() == capacity_) evict_lru();  // make room

        Node* node = new Node(key, value);
        push_front(node);
        map_[key] = node;
    }

    // Remove a key if present; returns true if something was erased.
    bool erase(const K& key) {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        unlink(it->second);
        delete it->second;
        map_.erase(it);
        return true;
    }

    void clear() {
        Node* cur = head_;
        while (cur) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        head_ = tail_ = nullptr;
        map_.clear();
    }

    // ---- introspection helpers (handy for demos / tests) -------------------

    // Most / least recently used keys (throw if empty).
    const K& most_recent() const {
        if (!head_) throw std::out_of_range("LRUCache::most_recent on empty cache");
        return head_->key;
    }
    const K& least_recent() const {
        if (!tail_) throw std::out_of_range("LRUCache::least_recent on empty cache");
        return tail_->key;
    }

    // Visit keys from MRU (front) to LRU (back) without changing recency.
    template <typename Fn>
    void for_each_mru_to_lru(Fn fn) const {
        for (Node* cur = head_; cur; cur = cur->next) fn(cur->key, cur->value);
    }

private:
    struct Node {
        K key;
        V value;
        Node* prev;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };

    // --- doubly linked list primitives (front = MRU, back = LRU) ---

    void push_front(Node* node) {
        node->prev = nullptr;
        node->next = head_;
        if (head_) head_->prev = node;
        head_ = node;
        if (!tail_) tail_ = node;  // first node ever
    }

    // Detach a node from the list (does not delete or touch the map).
    void unlink(Node* node) {
        if (node->prev) node->prev->next = node->next;
        else head_ = node->next;            // node was the head
        if (node->next) node->next->prev = node->prev;
        else tail_ = node->prev;            // node was the tail
        node->prev = node->next = nullptr;
    }

    // Promote an already-present node to the front (MRU).
    void move_to_front(Node* node) {
        if (node == head_) return;  // already MRU
        unlink(node);
        push_front(node);
    }

    // Drop the least-recently-used entry (the tail) from both list and map.
    void evict_lru() {
        if (!tail_) return;
        Node* lru = tail_;
        map_.erase(lru->key);
        unlink(lru);
        delete lru;
    }

    std::size_t capacity_;
    Node* head_ = nullptr;  // MRU end
    Node* tail_ = nullptr;  // LRU end
    std::unordered_map<K, Node*> map_;
};

}  // namespace ds

#endif  // DS_LRU_CACHE_HPP
