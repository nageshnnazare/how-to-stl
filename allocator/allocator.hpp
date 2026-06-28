#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <cstddef>      // for size_t
#include <cstdlib>      // for malloc, free
#include <new>          // for std::bad_alloc, alignof
#include <stdexcept>    // (reserved for future throws)
#include <cstring>      // (reserved for memset guards in extensions)

// ============================================================================
//  Custom Allocators — four hand-rolled allocation strategies
// ============================================================================
//
// WHAT THIS HEADER IS
// -------------------
// General-purpose malloc is a jack of all trades: it must handle arbitrary
// sizes, lifetimes, and fragmentation patterns. That generality costs time
// (metadata lookups, locking, coalescing) on every call. This header shows
// four *specialized* allocators that trade flexibility for speed by matching
// the allocator to a known allocation pattern.
//
// THE FOUR STRATEGIES (at a glance)
// ----------------------------------
//
//   LinearAllocator     bump pointer forward; bulk reset only
//   PoolAllocator       fixed-size blocks on a free list; O(1) alloc/dealloc
//   StackAllocator      LIFO bump + markers; unwind scopes in O(1)
//   FreeListAllocator   variable sizes; first-fit + coalescing (like malloc)
//
// WHY NOT JUST USE malloc?
// ------------------------
// malloc must search free lists, update global bookkeeping, and sometimes
// syscall into the OS. A pool allocator that pops a pre-carved block off a
// linked list is a handful of pointer operations — no search, no syscall.
//
//     malloc:     caller ──▶ [global heap metadata] ──▶ maybe OS ──▶ ptr
//     PoolAllocator: caller ──▶ pop free_list_ ──▶ ptr        (O(1))
//
// All four classes are NON-copyable and NOT thread-safe unless you add your
// own synchronization (see arena_allocator for per-thread bump allocators).
// ============================================================================

// ============================================================================
//  LinearAllocator — bump / arena (sequential, bulk-free)
// ============================================================================
//
// WHAT IT IS
// ----------
// A single contiguous buffer with one moving "bump" pointer (offset_).
// allocate() rounds up for alignment, checks space, returns a slice, and
// advances offset_. deallocate() is a deliberate no-op; reset() sets
// offset_ = 0 and reclaims everything at once.
//
// THE THREE FIELDS
// ----------------
//
//     buffer_  -> base of the malloc'd slab (or nullptr on failure)
//     size_    -> total bytes in the slab
//     offset_  -> next free byte (bytes [0, offset_) are "in use")
//
// MEMORY LAYOUT (offset_ = 120, size_ = 256)
// -------------------------------------------
//
//     LinearAllocator (stack)              Heap slab (size_ bytes)
//     ┌───────────────────┐               ┌──────── used ────────┬─ free ─┐
//     │ buffer_    ●──────┼──────────────▶│ A │pad│ B │pad│ C │...│        │
//     │ size_      = 256  │               └──────────────────────┴────────┘
//     │ offset_    = 120  │                 0                    120    256
//     └───────────────────┘                                    bump ──▶
//
// allocate(32) with align=16:
//     (1) pad offset_ up to next 16-byte boundary
//     (2) if offset_ + 32 <= size_, return buffer_+offset_, offset_ += 32+pad
//     (3) else return nullptr (arena exhausted)
//
// reset():
//     offset_ = 0   — O(1); no per-object destructors (caller must ~T() first)
//
// PERFECT FOR: per-frame temps, parser scratch, compile-time arenas.
// ============================================================================
class LinearAllocator {
private:
    char* buffer_;      // base pointer of the contiguous slab (malloc'd once)
    size_t size_;       // total capacity of the slab in bytes
    size_t offset_;     // bump pointer: bytes [0, offset_) are considered allocated

public:
    /**
     * @brief Construct a linear allocator with a fixed byte capacity.
     *
     * Allocates one raw slab via malloc. Throws std::bad_alloc on failure.
     */
    LinearAllocator(size_t size)
        : buffer_(static_cast<char*>(std::malloc(size)))
        , size_(size)
        , offset_(0)
    {
        if (!buffer_) throw std::bad_alloc();
    }

    ~LinearAllocator() {
        std::free(buffer_);
    }

    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    /**
     * @brief Bump-allocate `size` bytes with the given alignment.
     *
     * Step-by-step (see layout diagram above):
     *     (1) Compute padding so (offset_ + padding) % alignment == 0
     *     (2) If aligned_offset + size > size_, return nullptr
     *     (3) Return buffer_ + aligned_offset; advance offset_
     *
     * WHY alignment padding: CPUs and SIMD instructions require certain
     * addresses (e.g. 16 for SSE). Skipping bytes is cheaper than unaligned
     * loads that fault or run slower.
     *
     *     before:  [...used...|xx|----free----]
     *                         ^offset_
     *     after:   [...used...|pad| NEW |--free--]
     *                                   ^offset_
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t padding = 0;
        size_t aligned_offset = offset_;

        if (alignment > 0 && (offset_ % alignment) != 0) {
            padding = alignment - (offset_ % alignment);
            aligned_offset = offset_ + padding;
        }

        if (aligned_offset + size > size_) {
            return nullptr;
        }

        void* ptr = buffer_ + aligned_offset;
        offset_ = aligned_offset + size;
        return ptr;
    }

    /**
     * @brief Individual deallocation is intentionally unsupported.
     *
     * Linear allocators cannot know which hole to leave without a free list.
     * Call reset() when the whole batch of allocations dies together.
     */
    void deallocate(void*) {
        // no-op by design
    }

    /**
     * @brief Reclaim the entire arena in O(1).
     *
     *     offset_: 120 ──▶ 0
     *
     * Does NOT run destructors — destroy live T objects before reset().
     */
    void reset() {
        offset_ = 0;
    }

    /** @brief Bytes currently bumped past (high-water mark). */
    size_t used() const { return offset_; }

    /** @brief Bytes still available before the next allocate() fails. */
    size_t available() const { return size_ - offset_; }

    /** @brief Total slab capacity passed to the constructor. */
    size_t capacity() const { return size_; }
};

// ============================================================================
//  PoolAllocator — fixed-size blocks on a free list
// ============================================================================
//
// WHAT IT IS
// ----------
// Pre-carves num_blocks_ slices of block_size_ bytes from one malloc slab.
// Free blocks are threaded into a singly-linked list via the first
// sizeof(FreeNode) bytes of each block (overlaid when the block is free).
//
// THE FOUR FIELDS
// ---------------
//
//     buffer_      -> the backing slab (block_size_ * num_blocks_ bytes)
//     block_size_  -> every allocation is exactly this many bytes
//     num_blocks_  -> fixed capacity (pool exhausts when free_list_ == nullptr)
//     free_list_   -> head of the intrusive free list
//
// POOL LAYOUT (4 blocks, block_size_ = 16)
// ----------------------------------------
//
//     buffer_ slab:
//     ┌──────────┬──────────┬──────────┬──────────┐
//     │ block 0  │ block 1  │ block 2  │ block 3  │
//     └──────────┴──────────┴──────────┴──────────┘
//
//     free_list_ ──▶ block 2 ──▶ block 0 ──▶ nullptr
//                  (in use: block 1, block 3)
//
// allocate() — O(1) pop:
//     free_list_ ──▶ [node]──▶ [node]──▶ null
//                    │ pop
//                    ▼
//                  return node; free_list_ = node->next
//
// deallocate(ptr) — O(1) push:
//     node->next = free_list_; free_list_ = node
//
// WHY a free list instead of a bitmap? Pushing/popping a linked list is
// O(1) with no scanning; the "metadata" lives inside the free block itself.
// ============================================================================
class PoolAllocator {
private:
    struct FreeNode {
        FreeNode* next;     // when block is FREE, first bytes hold list link
    };

    char* buffer_;          // contiguous slab holding all fixed-size blocks
    size_t block_size_;     // bytes per block (clamped to >= sizeof(FreeNode))
    size_t num_blocks_;     // total block count (fixed at construction)
    FreeNode* free_list_;   // head of intrusive singly-linked free list

public:
    /**
     * @brief Create a pool of `num_blocks` fixed-size blocks.
     *
     * Constructor walks the slab back-to-front, pushing each block onto
     * free_list_. Cost is O(num_blocks) once; every later alloc is O(1).
     */
    PoolAllocator(size_t block_size, size_t num_blocks)
        : buffer_(nullptr)
        , block_size_(block_size < sizeof(FreeNode) ? sizeof(FreeNode) : block_size)
        , num_blocks_(num_blocks)
        , free_list_(nullptr)
    {
        size_t total_size = block_size_ * num_blocks_;
        buffer_ = static_cast<char*>(std::malloc(total_size));
        if (!buffer_) throw std::bad_alloc();

        for (size_t i = 0; i < num_blocks_; ++i) {
            void* block = buffer_ + (i * block_size_);
            FreeNode* node = static_cast<FreeNode*>(block);
            node->next = free_list_;
            free_list_ = node;
        }
    }

    ~PoolAllocator() {
        std::free(buffer_);
    }

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    /**
     * @brief Pop one block off the free list; return nullptr if exhausted.
     *
     *     BEFORE:  free_list_ ──▶ [B2]──▶ [B0]──▶ null
     *     AFTER:   free_list_ ──▶ [B0]──▶ null     return B2
     *
     * Caller must placement-new objects into the raw block and call ~T()
     * before deallocate().
     */
    void* allocate() {
        if (!free_list_) {
            return nullptr;
        }

        FreeNode* node = free_list_;
        free_list_ = node->next;
        return node;
    }

    /**
     * @brief Push a block back onto the free list.
     *
     *     BEFORE:  free_list_ ──▶ [B0]──▶ null
     *     AFTER:   free_list_ ──▶ [ptr]──▶ [B0]──▶ null
     *
     * The block must have been allocated from THIS pool and already destroyed.
     */
    void deallocate(void* ptr) {
        if (!ptr) return;

        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = free_list_;
        free_list_ = node;
    }

    /** @brief Fixed byte width of every block in this pool. */
    size_t block_size() const { return block_size_; }

    /** @brief Total number of blocks carved from the slab. */
    size_t num_blocks() const { return num_blocks_; }
};

// ============================================================================
//  StackAllocator — LIFO bump allocator with scope markers
// ============================================================================
//
// WHAT IT IS
// ----------
// Like LinearAllocator, but each allocation stores a small Header (size +
// padding) immediately before the returned pointer. Markers snapshot offset_;
// free_to_marker(marker) rewinds the bump pointer, bulk-freeing everything
// allocated after that point — classic "stack frame" semantics.
//
// THE THREE FIELDS (+ Header)
// ---------------------------
//
//     buffer_  -> backing slab
//     size_    -> slab capacity
//     offset_  -> current bump position (also exposed as Marker)
//
// ALLOCATION LAYOUT (one object)
// ------------------------------
//
//     buffer_:  [....][Header|pad|  USER DATA  ]────free────▶
//                        ^returned pointer
//
//     Header { size, padding } lets free_to_marker rewind correctly past
//     alignment padding without leaking gaps.
//
// MARKER / free_to_marker
// -----------------------
//
//     marker = offset_ after alloc A
//     alloc B, alloc C  →  offset_ moves right
//     free_to_marker(marker)  →  offset_ = marker  (B and C logically freed)
//
// NOTE: individual deallocate(ptr) is a simplified stub; real LIFO per-pointer
// free needs walking headers backward. Use markers for production patterns.
// ============================================================================
class StackAllocator {
private:
    struct Header {
        size_t size;        // user-requested payload size
        size_t padding;     // alignment padding bytes before the Header
    };

    char* buffer_;          // backing slab
    size_t size_;           // total slab capacity
    size_t offset_;         // bump pointer; also what get_marker() snapshots

public:
    using Marker = size_t;  // opaque snapshot of offset_ at a scope boundary

    StackAllocator(size_t size)
        : buffer_(static_cast<char*>(std::malloc(size)))
        , size_(size)
        , offset_(0)
    {
        if (!buffer_) throw std::bad_alloc();
    }

    ~StackAllocator() {
        std::free(buffer_);
    }

    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;

    /**
     * @brief Allocate with an inline Header for later marker-based rewind.
     *
     * Steps:
     *     (1) Align offset_ for Header placement
     *     (2) Write Header { size, total_padding }
     *     (3) Align data region; return pointer past Header
     *     (4) Advance offset_ past user data
     *
     *     [...][hdr pad|Header|data pad| PAYLOAD ]──▶ offset_
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t header_size = sizeof(Header);

        size_t header_padding = 0;
        size_t header_offset = offset_;

        if ((offset_ % alignof(Header)) != 0) {
            header_padding = alignof(Header) - (offset_ % alignof(Header));
            header_offset = offset_ + header_padding;
        }

        size_t data_offset = header_offset + header_size;
        size_t data_padding = 0;

        if (alignment > 0 && (data_offset % alignment) != 0) {
            data_padding = alignment - (data_offset % alignment);
            data_offset += data_padding;
        }

        size_t total_padding = header_padding + data_padding;
        size_t required = header_offset + header_size + total_padding + size - offset_;

        if (offset_ + required > size_) {
            return nullptr;
        }

        Header* header = reinterpret_cast<Header*>(buffer_ + header_offset);
        header->size = size;
        header->padding = total_padding;

        void* data_ptr = buffer_ + data_offset;
        offset_ = data_offset + size;

        return data_ptr;
    }

    /**
     * @brief Per-pointer deallocate is not fully implemented (demo stub).
     *
     * Prefer get_marker() + free_to_marker() for scope-based bulk free.
     */
    void deallocate(void* ptr) {
        (void)ptr;
    }

    /**
     * @brief Snapshot the current bump position.
     *
     *     Marker m = get_marker();   // "bookmark" top of stack
     *     ... allocate more ...
     *     free_to_marker(m);         // unwind to bookmark
     */
    Marker get_marker() const {
        return offset_;
    }

    /**
     * @brief Rewind the bump pointer to a prior marker — O(1) bulk free.
     *
     *     offset_: 200 ──▶ marker (e.g. 80)
     *
     * All allocations between marker and current offset are abandoned.
     * Run destructors first if the payloads hold live C++ objects.
     */
    void free_to_marker(Marker marker) {
        if (marker <= size_) {
            offset_ = marker;
        }
    }

    /** @brief Rewind to the beginning of the slab (full reset). */
    void reset() {
        offset_ = 0;
    }

    size_t used() const { return offset_; }
    size_t available() const { return size_ - offset_; }
};

// ============================================================================
//  FreeListAllocator — variable-size first-fit with coalescing
// ============================================================================
//
// WHAT IT IS
// ----------
// One malloc slab managed as a linked list of free Header nodes. Each Header
// records payload size and next pointer. allocate() first-fits a free block;
// deallocate() inserts sorted by address and merges physically adjacent holes.
//
// THE THREE FIELDS
// ----------------
//
//     buffer_     -> backing slab
//     size_       -> slab capacity
//     free_list_  -> head of free blocks (address-sorted after dealloc)
//
// BLOCK LAYOUT (in-use vs free)
// -----------------------------
//
//     FREE block in list:
//     ┌──────── Header ────────┬──── payload (size bytes) ────┐
//     │ size │ next ──────────▶│ (uninitialized / garbage OK) │
//     └────────────────────────┴──────────────────────────────┘
//              returned pointer is AFTER Header on allocate()
//
// COALESCING (why address-sorted insert)
// --------------------------------------
//
//     [ free A ][ free B ]  adjacent  →  merge into one larger free block
//
//     before:  A(size=100) ──▶ B(size=80)
//     after:   A(size=100+sizeof(Header)+80) ──▶ ...
//
// Reduces fragmentation so a future large allocate() can succeed.
// ============================================================================
class FreeListAllocator {
private:
    struct Header {
        size_t size;        // payload bytes available in this free block
        Header* next;       // next free block in address order
    };

    char* buffer_;          // backing slab from malloc
    size_t size_;           // total slab capacity
    Header* free_list_;     // head of the free-block intrusive list

    /**
     * @brief Merge physically adjacent free blocks in the list.
     *
     * Walks free_list_; if end of block A touches start of block B,
     * absorb B into A's size and unlink B.
     *
     *     [ A: 100 bytes ][ B: 80 bytes ]  →  [ A: 100+hdr+80 bytes ]
     */
    void coalesce() {
        if (!free_list_) return;

        Header* current = free_list_;
        while (current && current->next) {
            char* current_end = reinterpret_cast<char*>(current) + sizeof(Header) + current->size;
            char* next_start = reinterpret_cast<char*>(current->next);

            if (current_end == next_start) {
                current->size += sizeof(Header) + current->next->size;
                current->next = current->next->next;
            } else {
                current = current->next;
            }
        }
    }

public:
    /**
     * @brief Initialize the slab as one giant free block.
     *
     *     free_list_ ──▶ [ Header | size = slab - sizeof(Header) ]
     */
    FreeListAllocator(size_t size)
        : buffer_(static_cast<char*>(std::malloc(size)))
        , size_(size)
        , free_list_(nullptr)
    {
        if (!buffer_) throw std::bad_alloc();

        free_list_ = reinterpret_cast<Header*>(buffer_);
        free_list_->size = size - sizeof(Header);
        free_list_->next = nullptr;
    }

    ~FreeListAllocator() {
        std::free(buffer_);
    }

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

    /**
     * @brief First-fit allocate: scan free list for the first block >= size.
     *
     * Steps:
     *     (1) Walk free_list_ until current->size >= requested size
     *     (2) If block is much larger, SPLIT trailing remainder as new free block
     *     (3) Unlink chosen block from list
     *     (4) Return pointer just past the Header
     *
     * Split when remainder can hold another Header + 16 bytes (minimum useful hole):
     *
     *     [ free 200 bytes ]  request 64
     *         → [ in-use 64 ] [ free remainder ~120 ]
     *
     * O(n) in number of free blocks — acceptable for teaching; production
     * heaps use segregated size classes or trees.
     */
    void* allocate(size_t size) {
        Header* prev = nullptr;
        Header* current = free_list_;

        while (current) {
            if (current->size >= size) {
                if (current->size >= size + sizeof(Header) + 16) {
                    Header* new_block = reinterpret_cast<Header*>(
                        reinterpret_cast<char*>(current) + sizeof(Header) + size
                    );
                    new_block->size = current->size - size - sizeof(Header);
                    new_block->next = current->next;

                    current->size = size;

                    if (prev) {
                        prev->next = new_block;
                    } else {
                        free_list_ = new_block;
                    }
                } else {
                    if (prev) {
                        prev->next = current->next;
                    } else {
                        free_list_ = current->next;
                    }
                }

                return reinterpret_cast<char*>(current) + sizeof(Header);
            }

            prev = current;
            current = current->next;
        }

        return nullptr;
    }

    /**
     * @brief Return a block to the free list and coalesce neighbors.
     *
     * Steps:
     *     (1) Walk back sizeof(Header) from user pointer to find Header
     *     (2) Insert into free_list_ sorted by address (enables merge)
     *     (3) coalesce() adjacent buddies
     *
     * Address order:
     *     ... ──▶ [low addr] ──▶ [returned block] ──▶ [high addr] ──▶ ...
     */
    void deallocate(void* ptr) {
        if (!ptr) return;

        Header* block = reinterpret_cast<Header*>(
            static_cast<char*>(ptr) - sizeof(Header)
        );

        if (!free_list_ || block < free_list_) {
            block->next = free_list_;
            free_list_ = block;
        } else {
            Header* current = free_list_;
            while (current->next && current->next < block) {
                current = current->next;
            }
            block->next = current->next;
            current->next = block;
        }

        coalesce();
    }

    /** @brief Total bytes in the backing slab. */
    size_t capacity() const { return size_; }
};

#endif // ALLOCATOR_HPP
