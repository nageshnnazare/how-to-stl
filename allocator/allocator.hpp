#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <cstring>

/**
 * @brief Custom Memory Allocator with various allocation strategies
 * 
 * Demonstrates different memory allocation techniques:
 * - Linear/Bump allocator (fast sequential allocation)
 * - Pool allocator (fixed-size blocks)
 * - Stack allocator (LIFO allocation/deallocation)
 * - Free list allocator (general purpose with coalescing)
 */

// ============================================================================
// 1. LINEAR ALLOCATOR (Bump Allocator)
// ============================================================================
/**
 * Extremely fast allocator that bumps a pointer forward.
 * No individual deallocation - must reset entire allocator.
 * Perfect for: frame allocations, temporary data, parsers
 */
class LinearAllocator {
private:
    char* buffer_;      // Start of memory block
    size_t size_;       // Total size
    size_t offset_;     // Current allocation offset
    
public:
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
    
    // No copying
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;
    
    /**
     * Allocate memory with alignment
     * O(1) complexity - just bump the pointer!
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        // Align current offset
        size_t padding = 0;
        size_t aligned_offset = offset_;
        
        if (alignment > 0 && (offset_ % alignment) != 0) {
            padding = alignment - (offset_ % alignment);
            aligned_offset = offset_ + padding;
        }
        
        // Check if we have space
        if (aligned_offset + size > size_) {
            return nullptr;  // Out of memory
        }
        
        void* ptr = buffer_ + aligned_offset;
        offset_ = aligned_offset + size;
        return ptr;
    }
    
    /**
     * Individual deallocation not supported
     * Use reset() to reclaim all memory
     */
    void deallocate(void*) {
        // No-op - linear allocators don't support individual deallocation
    }
    
    /**
     * Reset allocator - reclaim all memory
     * O(1) - just reset offset!
     */
    void reset() {
        offset_ = 0;
    }
    
    size_t used() const { return offset_; }
    size_t available() const { return size_ - offset_; }
    size_t capacity() const { return size_; }
};

// ============================================================================
// 2. POOL ALLOCATOR
// ============================================================================
/**
 * Fixed-size block allocator using free list.
 * All allocations are the same size.
 * Perfect for: object pools, entity systems, frequent alloc/dealloc
 */
class PoolAllocator {
private:
    struct FreeNode {
        FreeNode* next;
    };
    
    char* buffer_;          // Memory block
    size_t block_size_;     // Size of each block
    size_t num_blocks_;     // Total number of blocks
    FreeNode* free_list_;   // Head of free list
    
public:
    PoolAllocator(size_t block_size, size_t num_blocks)
        : buffer_(nullptr)
        , block_size_(block_size < sizeof(FreeNode) ? sizeof(FreeNode) : block_size)
        , num_blocks_(num_blocks)
        , free_list_(nullptr)
    {
        // Allocate memory for all blocks
        size_t total_size = block_size_ * num_blocks_;
        buffer_ = static_cast<char*>(std::malloc(total_size));
        if (!buffer_) throw std::bad_alloc();
        
        // Initialize free list - link all blocks
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
     * Allocate a block from the pool
     * O(1) - just pop from free list
     */
    void* allocate() {
        if (!free_list_) {
            return nullptr;  // Pool exhausted
        }
        
        // Pop from free list
        FreeNode* node = free_list_;
        free_list_ = node->next;
        return node;
    }
    
    /**
     * Return a block to the pool
     * O(1) - just push to free list
     */
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        // Push to free list
        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = free_list_;
        free_list_ = node;
    }
    
    size_t block_size() const { return block_size_; }
    size_t num_blocks() const { return num_blocks_; }
};

// ============================================================================
// 3. STACK ALLOCATOR
// ============================================================================
/**
 * LIFO allocator with markers for bulk deallocation.
 * Must deallocate in reverse order of allocation.
 * Perfect for: scope-based allocation, recursive algorithms
 */
class StackAllocator {
private:
    struct Header {
        size_t size;
        size_t padding;
    };
    
    char* buffer_;
    size_t size_;
    size_t offset_;
    
public:
    using Marker = size_t;
    
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
     * Allocate memory with header
     * O(1) complexity
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t header_size = sizeof(Header);
        
        // Align for header
        size_t header_padding = 0;
        size_t header_offset = offset_;
        
        if ((offset_ % alignof(Header)) != 0) {
            header_padding = alignof(Header) - (offset_ % alignof(Header));
            header_offset = offset_ + header_padding;
        }
        
        // Align for data
        size_t data_offset = header_offset + header_size;
        size_t data_padding = 0;
        
        if (alignment > 0 && (data_offset % alignment) != 0) {
            data_padding = alignment - (data_offset % alignment);
            data_offset += data_padding;
        }
        
        size_t total_padding = header_padding + data_padding;
        size_t required = header_offset + header_size + total_padding + size - offset_;
        
        if (offset_ + required > size_) {
            return nullptr;  // Out of memory
        }
        
        // Store header
        Header* header = reinterpret_cast<Header*>(buffer_ + header_offset);
        header->size = size;
        header->padding = total_padding;
        
        void* data_ptr = buffer_ + data_offset;
        offset_ = data_offset + size;
        
        return data_ptr;
    }
    
    /**
     * Deallocate - must be in LIFO order!
     * O(1) complexity
     */
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        // Get header (need to read padding first)
        // Note: This is tricky - we need the header to get padding, but need padding to find header
        // For proper implementation, would need to store header pointer or use different strategy
        // For now, simplified: just mark as deallocated but don't actually move offset
        // (Full LIFO deallocation should use markers, not individual deallocate)
        (void)ptr;  // Simplified for demo
    }
    
    /**
     * Get current position marker
     */
    Marker get_marker() const {
        return offset_;
    }
    
    /**
     * Free to marker (bulk deallocation)
     * O(1) - just reset offset
     */
    void free_to_marker(Marker marker) {
        if (marker <= size_) {
            offset_ = marker;
        }
    }
    
    void reset() {
        offset_ = 0;
    }
    
    size_t used() const { return offset_; }
    size_t available() const { return size_ - offset_; }
};

// ============================================================================
// 4. FREE LIST ALLOCATOR
// ============================================================================
/**
 * General-purpose allocator with free list and coalescing.
 * Supports arbitrary allocation sizes and deallocation order.
 * Perfect for: general purpose allocation, long-lived objects
 */
class FreeListAllocator {
private:
    struct Header {
        size_t size;
        Header* next;
    };
    
    char* buffer_;
    size_t size_;
    Header* free_list_;
    
    /**
     * Coalesce adjacent free blocks
     */
    void coalesce() {
        if (!free_list_) return;
        
        Header* current = free_list_;
        while (current && current->next) {
            // Check if adjacent
            char* current_end = reinterpret_cast<char*>(current) + sizeof(Header) + current->size;
            char* next_start = reinterpret_cast<char*>(current->next);
            
            if (current_end == next_start) {
                // Merge
                current->size += sizeof(Header) + current->next->size;
                current->next = current->next->next;
            } else {
                current = current->next;
            }
        }
    }
    
public:
    FreeListAllocator(size_t size)
        : buffer_(static_cast<char*>(std::malloc(size)))
        , size_(size)
        , free_list_(nullptr)
    {
        if (!buffer_) throw std::bad_alloc();
        
        // Initialize with one large free block
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
     * Allocate using first-fit strategy
     * O(n) where n = number of free blocks
     */
    void* allocate(size_t size) {
        Header* prev = nullptr;
        Header* current = free_list_;
        
        // First-fit search
        while (current) {
            if (current->size >= size) {
                // Found a suitable block
                
                // If block is much larger, split it
                if (current->size >= size + sizeof(Header) + 16) {
                    // Split the block
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
                    // Use entire block
                    if (prev) {
                        prev->next = current->next;
                    } else {
                        free_list_ = current->next;
                    }
                }
                
                // Return data pointer (after header)
                return reinterpret_cast<char*>(current) + sizeof(Header);
            }
            
            prev = current;
            current = current->next;
        }
        
        return nullptr;  // No suitable block found
    }
    
    /**
     * Deallocate and add back to free list
     * O(n) where n = number of free blocks
     */
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        // Get header
        Header* block = reinterpret_cast<Header*>(
            static_cast<char*>(ptr) - sizeof(Header)
        );
        
        // Insert into free list (sorted by address for coalescing)
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
        
        // Coalesce adjacent blocks
        coalesce();
    }
    
    size_t capacity() const { return size_; }
};

#endif // ALLOCATOR_HPP

