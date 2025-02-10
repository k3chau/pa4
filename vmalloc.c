#include "vm.h"
#include "vmlib.h"

void *vmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t total_size = ROUND_UP(size + sizeof(struct block_header), BLKALIGN);
    if (total_size < BLKALIGN) {
        total_size = BLKALIGN;
    }

    struct block_header *current = heapstart;
    struct block_header *best_fit = NULL;
    size_t best_fit_size = SIZE_MAX;

    // Step 1: Find the best-fit block
    while (current->size_status != VM_ENDMARK) {
        size_t current_size = BLKSZ(current);
        int is_busy = current->size_status & VM_BUSY;

        if (!is_busy && current_size >= total_size) {
            if (current_size < best_fit_size) {
                best_fit = current;
                best_fit_size = current_size;
            }
        }

        current = (struct block_header *)((char *)current + current_size);
    }

    
    if (best_fit == NULL) {
        return NULL;
    }

    size_t remaining_size = best_fit_size - total_size;

    if (remaining_size >= BLKALIGN) {
        
        best_fit->size_status = total_size | (best_fit->size_status & VM_PREVBUSY) | VM_BUSY;

        struct block_header *next_block = (struct block_header *)((char *)best_fit + total_size);
        next_block->size_status = remaining_size | VM_PREVBUSY;
        next_block->size_status &= ~VM_BUSY;

        struct block_footer *next_footer = (struct block_footer *)((char *)next_block + remaining_size - sizeof(struct block_footer));
        next_footer->size = remaining_size;
    } else {
        
        best_fit->size_status |= VM_BUSY;

        struct block_header *next_block = (struct block_header *)((char *)best_fit + best_fit_size);
        if (next_block->size_status != VM_ENDMARK) {
            next_block->size_status |= VM_PREVBUSY;
        }
    }

    return (void *)((char *)best_fit + sizeof(struct block_header));
}
