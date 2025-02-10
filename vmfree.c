#include "vm.h"
#include "vmlib.h"

/**
 * The vmfree() function frees the memory space pointed to by ptr,
 * which must have been returned by a previous call to vmmalloc().
 * Otherwise, or if free(ptr) has already been called before,
 * undefined behavior occurs.
 * If ptr is NULL, no operation is performed.
 */
void vmfree(void *ptr)
{
if (ptr == NULL) {
        return;
    }

    
    struct block_header *current = (struct block_header *)((char *)ptr - sizeof(struct block_header));

    
    if (!(current->size_status & VM_BUSY)) {
        
        return;
    }

    size_t current_size = BLKSZ(current);

    
    current->size_status &= ~VM_BUSY;

    
    struct block_footer *current_footer = (struct block_footer *)((char *)current + current_size - sizeof(struct block_footer));
    current_footer->size = current_size;

    
    struct block_header *next_block = (struct block_header *)((char *)current + current_size);
    if (next_block->size_status != VM_ENDMARK && !(next_block->size_status & VM_BUSY)) {
        size_t next_size = BLKSZ(next_block);

        
        current_size += next_size;

        
        current->size_status = current_size | (current->size_status & VM_PREVBUSY);

       
        struct block_footer *next_footer = (struct block_footer *)((char *)current + current_size - sizeof(struct block_footer));
        next_footer->size = current_size;

        
        next_block = (struct block_header *)((char *)current + current_size);
    }

    
    if (!(current->size_status & VM_PREVBUSY)) {
        
        struct block_footer *prev_footer = (struct block_footer *)((char *)current - sizeof(struct block_footer));
        size_t prev_size = prev_footer->size;

        
        struct block_header *prev_block = (struct block_header *)((char *)current - prev_size);

     
        size_t total_size = prev_size + current_size;

      
        prev_block->size_status = total_size | (prev_block->size_status & VM_PREVBUSY);

 
        struct block_footer *current_footer = (struct block_footer *)((char *)prev_block + total_size - sizeof(struct block_footer));
        current_footer->size = total_size;

      
        current = prev_block;
        current_size = total_size;
    }

    
    next_block = (struct block_header *)((char *)current + current_size);
    if (next_block->size_status != VM_ENDMARK) {
        next_block->size_status &= ~VM_PREVBUSY;
    }
}
