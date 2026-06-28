#ifndef DS_TRIE_HPP
#define DS_TRIE_HPP

// ============================================================================
//  Trie -- prefix tree for lowercase English words
// ============================================================================
//
// THE IDEA
// --------
// Each edge is labeled with a character; a path from the root spells a prefix.
// A node marked `is_end` means the path from the root to here is a complete word.
//
// CHILDREN: fixed array children[26]
// ----------------------------------
// We map 'a'..'z' to indices 0..25 with `c - 'a'`. Fast and cache-friendly for
// lowercase ASCII; wastes space when the alphabet is sparse (a map<char,Node*>
// would be leaner for mixed scripts).
//
// EXAMPLE (words: "cat", "car", "dog", "do")
//
//     root --c-->* --a-->* --t(end)
//            |            \--r(end)
//            |
//            +--d-->* --o(end) --g(end)
//
// COMPLEXITY (W = word length, N = number of words, A = alphabet size = 26)
//     insert / contains / starts_with ... O(W)
//     erase ............................ O(W)
//     words_with_prefix ................ O(W + matches)
//     word_count ....................... O(1)
// ============================================================================

#include <cstddef>      // std::size_t
#include <string>       // std::string
#include <vector>       // std::vector

namespace ds {

class Trie {
public:
    Trie() : root_(new Node()), word_count_(0) {}

    Trie(const Trie& other) : root_(new Node()), word_count_(0) {
        copy_from(other.root_);
        word_count_ = other.word_count_;
    }

    Trie& operator=(Trie other) {
        swap(other);
        return *this;
    }

    ~Trie() { destroy(root_); }

    std::size_t word_count() const { return word_count_; }
    bool empty() const { return word_count_ == 0; }

    void clear() {
        destroy(root_);
        root_ = new Node();
        word_count_ = 0;
    }

    // Insert a lowercase word (letters a-z only).
    void insert(const std::string& word) {
        Node* cur = root_;
        for (char c : word) {
            int idx = index(c);
            if (!cur->children[idx])
                cur->children[idx] = new Node();
            cur = cur->children[idx];
        }
        if (!cur->is_end) ++word_count_;
        cur->is_end = true;
    }

    bool contains(const std::string& word) const {
        const Node* node = find_node(word);
        return node && node->is_end;
    }

    // True if any inserted word has `prefix` as a prefix.
    bool starts_with(const std::string& prefix) const {
        return find_node(prefix) != nullptr;
    }

    // Remove a word; returns false if it was not present.
    bool erase(const std::string& word) {
        if (!contains(word)) return false;
        erase_rec(root_, word, 0);
        --word_count_;
        return true;
    }

    // All complete words that begin with `prefix`.
    std::vector<std::string> words_with_prefix(const std::string& prefix) const {
        std::vector<std::string> out;
        const Node* node = find_node(prefix);
        if (!node) return out;
        collect_words(node, prefix, out);
        return out;
    }

    void swap(Trie& other) noexcept {
        std::swap(root_, other.root_);
        std::swap(word_count_, other.word_count_);
    }

private:
    static const int ALPHA = 26;

    struct Node {
        Node* children[ALPHA];
        bool is_end;

        Node() : is_end(false) {
            for (int i = 0; i < ALPHA; ++i) children[i] = nullptr;
        }
    };

    Node* root_;
    std::size_t word_count_;

    static int index(char c) { return c - 'a'; }

    void destroy(Node* n) {
        if (!n) return;
        for (int i = 0; i < ALPHA; ++i)
            destroy(n->children[i]);
        delete n;
    }

    void copy_from(const Node* src) {
        std::string path;
        copy_walk(src, path);
    }

    void copy_walk(const Node* src, std::string& path) {
        if (src->is_end) insert(path);
        for (int i = 0; i < ALPHA; ++i) {
            if (src->children[i]) {
                path.push_back(static_cast<char>('a' + i));
                copy_walk(src->children[i], path);
                path.pop_back();
            }
        }
    }

    const Node* find_node(const std::string& s) const {
        const Node* cur = root_;
        for (char c : s) {
            int idx = index(c);
            if (!cur->children[idx]) return nullptr;
            cur = cur->children[idx];
        }
        return cur;
    }

    static void collect_words(const Node* n, const std::string& prefix,
                              std::vector<std::string>& out) {
        if (n->is_end) out.push_back(prefix);
        for (int i = 0; i < ALPHA; ++i) {
            if (n->children[i])
                collect_words(n->children[i], prefix + static_cast<char>('a' + i), out);
        }
    }

    // Post-order prune: delete childless non-end nodes on the way back up.
    bool erase_rec(Node* n, const std::string& word, std::size_t depth) {
        if (depth == word.size()) {
            n->is_end = false;
            return no_children(n) && !n->is_end;
        }
        int idx = index(word[depth]);
        Node* child = n->children[idx];
        if (!child) return false;
        bool remove_child = erase_rec(child, word, depth + 1);
        if (remove_child) {
            delete child;
            n->children[idx] = nullptr;
        }
        return no_children(n) && !n->is_end;
    }

    static bool no_children(const Node* n) {
        for (int i = 0; i < ALPHA; ++i)
            if (n->children[i]) return false;
        return true;
    }
};

}  // namespace ds

#endif  // DS_TRIE_HPP
