#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "types.h"
#include "syscalls.h"

// Memory allocation configuration
#define PAGE_SIZE 4096          // sbrk allocates pages of 4096 bytes
#define MIN_BLOCK_SIZE 32       // Minimum allocation unit (for metadata)
#define ALIGNMENT 8             // Align all allocations to 8 bytes

// Metadata structure for each allocated block
typedef struct block_metadata {
    uint32_t size;                  // Size of the block (excluding metadata)
    uint32_t is_free;               // 0 = allocated, 1 = free
    struct block_metadata *next;    // Next block in the linked list
    struct block_metadata *prev;    // Previous block in the linked list
} block_metadata_t;

// Global heap management
static block_metadata_t *heap_start = NULL;  // First block in heap
static void *heap_end = NULL;                // End of allocated heap

// Forward declarations
static void *extend_heap(uint32_t size);
static void *align_pointer(void *ptr, uint32_t alignment);
static block_metadata_t *find_free_block(uint32_t size);
static void merge_blocks(void);
static void split_block(block_metadata_t *block, uint32_t size);

// Align a pointer to the given alignment boundary
static void *align_pointer(void *ptr, uint32_t alignment) {
    uint32_t addr = (uint32_t)ptr;
    uint32_t remainder = addr % alignment;
    if (remainder != 0) {
        addr += (alignment - remainder);
    }
    return (void *)addr;
}

// Request more memory from the kernel
static void *extend_heap(uint32_t size) {
    // Calculate how many pages we need
    uint32_t total_size = size + sizeof(block_metadata_t);
    uint32_t pages_needed = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Request pages from kernel
    void *new_block = sbrk(1);
    
    if (new_block == NULL || (int)new_block == -1) {
        printf("malloc: sbrk() failed\n");
        return NULL;
    }
    
    for(int remaining = pages_needed-1; remaining > 0; remaining--){
        void * ret = sbrk(1); // For now sbrk works only with pages = 1
        if(ret == NULL){
            pages_needed-= remaining; // Remaining were not allocated
            break;
        } 
    }
    

    
    // Update heap_end if this is the first allocation
    if (heap_end == NULL) {
        heap_end = new_block + (pages_needed * PAGE_SIZE);
    } else {
        // Update heap_end if we extended beyond it
        void *new_end = new_block + (pages_needed * PAGE_SIZE);
        if (new_end > heap_end) {
            heap_end = new_end;
        }
    }
    
    return new_block;
}

// Find a free block that can fit the requested size
static block_metadata_t *find_free_block(uint32_t size) {
    block_metadata_t *current = heap_start;
    
    while (current != NULL) {
        // Check if block is free and large enough
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Split a block if it's larger than needed
static void split_block(block_metadata_t *block, uint32_t size) {
    // Only split if there's enough space for the metadata of the new block
    if (block->size <= size + sizeof(block_metadata_t)) {
        return;  // Not enough space to split
    }
    
    // Create new block metadata after the current data
    block_metadata_t *new_block = (block_metadata_t *)((char *)block + sizeof(block_metadata_t) + size);
    
    // Copy size from old block's remaining space
    new_block->size = block->size - size - sizeof(block_metadata_t);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    
    // Update original block
    if (block->next != NULL) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
}

// Merge adjacent free blocks
static void merge_blocks(void) {
    if (heap_start == NULL) {
        return;
    }
    
    block_metadata_t *current = heap_start;
    
    while (current != NULL && current->next != NULL) {
        block_metadata_t *next = current->next;
        
        // If both blocks are free, merge them
        if (current->is_free && next->is_free) {
            current->size += sizeof(block_metadata_t) + next->size;
            current->next = next->next;
            
            if (next->next != NULL) {
                next->next->prev = current;
            }
            
            // Don't advance current, check if we can merge with the next block
            continue;
        }
        
        current = current->next;
    }
}

// Allocate memory
void *malloc(size_t size) {
    // Sanity checks
    if (size == 0) {
        return NULL;
    }
    
    // Align size to ALIGNMENT bytes
    uint32_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    // Ensure minimum block size
    if (aligned_size < MIN_BLOCK_SIZE) {
        aligned_size = MIN_BLOCK_SIZE;
    }
    
    block_metadata_t *block = NULL;
    
    // First allocation
    if (heap_start == NULL) {
        // Request initial heap space
        void *heap_space = extend_heap(aligned_size);
        if (heap_space == NULL) {
            return NULL;
        }
        
        heap_start = (block_metadata_t *)heap_space;
        heap_start->size = aligned_size;
        heap_start->is_free = 0;
        heap_start->next = NULL;
        heap_start->prev = NULL;
        
        // Check if there's remaining space in the allocated pages
        uint32_t total_size = aligned_size + sizeof(block_metadata_t);
        uint32_t pages_allocated = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
        uint32_t actual_allocated = pages_allocated * PAGE_SIZE;
        
        if (actual_allocated > total_size) {
            // Create a free block for the remaining space
            uint32_t remaining_size = actual_allocated - total_size;
            
            if (remaining_size >= sizeof(block_metadata_t)) {
                block_metadata_t *free_block = (block_metadata_t *)((char *)heap_space + total_size);
                free_block->size = remaining_size - sizeof(block_metadata_t);
                free_block->is_free = 1;
                free_block->next = NULL;
                free_block->prev = heap_start;
                
                heap_start->next = free_block;
            }
        }
        
        return (void *)((char *)heap_start + sizeof(block_metadata_t));
    }
    
    // Try to find a free block
    block = find_free_block(aligned_size);
    
    if (block != NULL) {
        // Found a free block, use it
        block->is_free = 0;
        
        // Split the block if it's much larger than needed
        split_block(block, aligned_size);
        
        return (void *)((char *)block + sizeof(block_metadata_t));
    }
    
    // No suitable free block found, extend heap
    void *heap_space = extend_heap(aligned_size);
    if (heap_space == NULL) {
        return NULL;
    }
    
    // Create metadata for new block
    block = (block_metadata_t *)heap_space;
    block->size = aligned_size;
    block->is_free = 0;
    block->next = NULL;
    block->prev = NULL;
    
    // Add to linked list
    if (heap_start != NULL) {
        block_metadata_t *current = heap_start;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = block;
        block->prev = current;
    } else {
        heap_start = block;
    }
    
    // Check if there's remaining space in the allocated pages
    uint32_t total_size = aligned_size + sizeof(block_metadata_t);
    uint32_t pages_allocated = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t actual_allocated = pages_allocated * PAGE_SIZE;
    
    if (actual_allocated > total_size) {
        // Create a free block for the remaining space
        uint32_t remaining_size = actual_allocated - total_size;
        
        if (remaining_size >= sizeof(block_metadata_t)) {
            block_metadata_t *free_block = (block_metadata_t *)((char *)heap_space + total_size);
            free_block->size = remaining_size - sizeof(block_metadata_t);
            free_block->is_free = 1;
            free_block->next = block->next;
            free_block->prev = block;
            
            block->next = free_block;
            if (free_block->next != NULL) {
                free_block->next->prev = free_block;
            }
        }
    }
    
    return (void *)((char *)block + sizeof(block_metadata_t));
}

// Free memory
void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // Get metadata by going back one block_metadata_t size
    block_metadata_t *block = (block_metadata_t *)ptr - 1;
    
    // Sanity check: verify we're not freeing the same block twice
    if (block->is_free) {
        printf("malloc: double free detected at %p\n", ptr);
        return;
    }
    
    // Mark as free
    block->is_free = 1;
    
    // Try to merge with adjacent blocks
    merge_blocks();
}

// Reallocate memory
void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        // If ptr is NULL, realloc behaves like malloc
        return malloc(size);
    }
    
    if (size == 0) {
        // If size is 0, realloc behaves like free
        free(ptr);
        return NULL;
    }
    
    // Get metadata
    block_metadata_t *block = (block_metadata_t *)ptr - 1;
    
    // Align new size
    uint32_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    if (aligned_size < MIN_BLOCK_SIZE) {
        aligned_size = MIN_BLOCK_SIZE;
    }
    
    // If block is already large enough, reuse it
    if (block->size >= aligned_size) {
        // If much larger, we could split, but for simplicity, just reuse
        return ptr;
    }
    
    // Need larger block - allocate new, copy, free old
    void *new_ptr = malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    // Copy old data to new block
    memcpy((uint8_t *)new_ptr, (const uint8_t *)ptr, block->size);
    
    // Free old block
    free(ptr);
    
    return new_ptr;
}

// Allocate and zero-initialize memory
void *calloc(size_t count, size_t size) {
    size_t total_size = count * size;
    
    void *ptr = malloc(total_size);
    if (ptr == NULL) {
        return NULL;
    }
    
    // Zero-initialize the memory
    memset((uint8_t *)ptr, 0, total_size);
    
    return ptr;
}

// Debug: Print heap statistics
void malloc_stats(void) {
    if (heap_start == NULL) {
        printf("Heap not initialized\n");
        return;
    }
    
    printf("=== Malloc Statistics ===\n");
    printf("Heap start: %p\n", heap_start);
    printf("Heap end: %p\n", heap_end);
    printf("\nBlocks:\n");
    
    block_metadata_t *current = heap_start;
    int block_num = 0;
    uint32_t total_allocated = 0;
    uint32_t total_free = 0;
    
    while (current != NULL) {
        const char *status = current->is_free ? "FREE" : "USED";
        printf("  Block %d: %p size=%u status=%s\n", 
               block_num, current, current->size, status);
        
        if (current->is_free) {
            total_free += current->size;
        } else {
            total_allocated += current->size;
        }
        
        current = current->next;
        block_num++;
    }
    
    printf("\nTotal allocated: %u bytes\n", total_allocated);
    printf("Total free: %u bytes\n", total_free);
}
