I used chatgpt in order to understand the assignment better and i asked it to give me pseudo code on how to do some of the functions with step by step breakdown. 
To implement vmalloc and vmfree with the specified requirements, you can follow the step-by-step pseudocode below. The focus is on maintaining metadata for the heap, aligning allocations to 16-byte boundaries, and using a "best fit" allocation strategy.

Data Structures
We need a simple data structure to represent blocks of memory in the heap:

Heap Block Metadata:
size: Size of the block (including metadata).
is_free: Boolean indicating whether the block is free or allocated.
next: Pointer to the next block in the heap (linked list of blocks).
1. vmalloc(size_t size)
Step-by-step pseudocode:

Validate Input:

If size <= 0, return NULL.
Align Size:

Add the size of metadata to size (e.g., aligned_size = size + sizeof(metadata)).
Align aligned_size to 16 bytes:
pseudo
Copy code
aligned_size = (aligned_size + 15) & ~15
Search for Best Fit Block:

Traverse the heap to find the smallest free block that is large enough to accommodate aligned_size.
Keep track of the best-fit block (smallest block that satisfies the size requirement).
If no suitable block is found, return NULL.
Split the Block:

If the best-fit block has excess space (greater than aligned_size + sizeof(metadata)), split it:
Create a new block in the leftover space.
Update the size of the original block.
Mark the Block as Allocated:

Set is_free = false for the chosen block.
Return the pointer to the memory region after the metadata.
Pseudocode for vmalloc:

pseudo
Copy code
function vmalloc(size):
    if size <= 0:
        return NULL

    aligned_size = (size + sizeof(metadata) + 15) & ~15
    best_fit = NULL
    best_fit_size = INFINITY

    block = heap_start
    while block != NULL:
        if block.is_free and block.size >= aligned_size and block.size < best_fit_size:
            best_fit = block
            best_fit_size = block.size
        block = block.next

    if best_fit == NULL:
        return NULL

    if best_fit.size >= aligned_size + sizeof(metadata) + 16:
        new_block = split_block(best_fit, aligned_size)
        new_block.next = best_fit.next
        best_fit.next = new_block

    best_fit.is_free = false
    return pointer_to_user_memory(best_fit)
2. vmfree(void* ptr)
Step-by-step pseudocode:

Validate Input:

If ptr == NULL, return immediately.
Locate the Block:

Calculate the starting address of the block metadata from ptr.
Mark the Block as Free:

Set is_free = true.
Coalesce Adjacent Free Blocks:

If the previous block is free, merge it with the current block.
If the next block is free, merge it with the current block.
Pseudocode for vmfree:

pseudo
Copy code
function vmfree(ptr):
    if ptr == NULL:
        return

    block = metadata_from_pointer(ptr)

    if block.is_free:
        return

    block.is_free = true

    # Coalesce with next block
    if block.next != NULL and block.next.is_free:
        block.size += block.next.size
        block.next = block.next.next

    # Coalesce with previous block
    previous = find_previous_block(block)
    if previous != NULL and previous.is_free:
        previous.size += block.size
        previous.next = block.next
3. Supporting Functions
Split Block:

pseudo
Copy code
function split_block(block, size):
    new_block_address = block_address(block) + size
    new_block = metadata_at(new_block_address)
    new_block.size = block.size - size
    new_block.is_free = true
    new_block.next = block.next
    block.size = size
    block.next = new_block
    return new_block
Metadata from Pointer:

pseudo
Copy code
function metadata_from_pointer(ptr):
    return pointer_at(ptr - sizeof(metadata))
Pointer to User Memory:

pseudo
Copy code
function pointer_to_user_memory(block):
    return block_address(block) + sizeof(metadata)
Find Previous Block:

pseudo
Copy code
function find_previous_block(block):
    current = heap_start
    while current != NULL and current.next != block:
        current = current.next
    return current
Notes
Ensure all allocations are aligned to 16 bytes by rounding up the requested size.
Metadata should be stored at the beginning of each block, and user memory starts immediately after metadata.
Use a linked list to keep track of all blocks in the heap.
Coalescing ensures that free blocks are merged to prevent fragmentation.
This pseudocode provides the foundation for implementing vmalloc and vmfree. Let me know if you need further details or assistance!

. Understanding and Initial Setup
Run the Starter Code: Make sure you understand how the provided init_heap() and vminfo() functions work. Test them to confirm that the heap initializes correctly and you can inspect its state.
Understand the Block Header Layout:
Each block header contains:
Size of the block.
Status bit (0 for free, 1 for allocated).
Previous block status bit (0 if previous is free, 1 if busy).
2. Implementing vmalloc
a. Allocation Size Calculation
Tips:

Add 8 bytes for the header to the requested size.
Round up to the nearest multiple of 16:
c
Copy code
size = (requested_size + 8 + 15) & ~15;
Ensure you correctly account for both payload and metadata size in all calculations.
b. Best-Fit Allocation Policy
Tips:

Traverse the Heap: Loop through the heap and find the smallest free block that is large enough for the aligned size.
Track the Best Fit:
Initialize a pointer (best_fit) to NULL and a variable (best_fit_size) to MAX_VALUE.
Update best_fit whenever you find a smaller free block that satisfies the size requirement.
Helper Function:
Write a helper function to check if a block is free and large enough:
c
Copy code
bool is_block_free_and_fits(header* block, size_t size) {
    return block->is_free && block->size >= size;
}
c. Splitting Blocks
Tips:

Check for Splitting:
If the block is much larger than needed (e.g., block->size > size + 16), split it.
Create a New Block:
Update the original block's size to the requested size.
Create a new block in the leftover space.
Write a header for the new block, mark it as free, and link it to the next block.
Update Metadata:
Set the new block’s size and free status (is_free = 1).
Update the previous bit of the block that comes after the newly split block.
d. Returning the Address
Tips:

Return the Payload Pointer:
The payload starts immediately after the block header. Use pointer arithmetic to calculate this:
c
Copy code
return (void*)((char*)best_fit + sizeof(header));
3. Implementing vmfree
a. Mark Block as Free
Tips:

Locate the header from the given pointer:
c
Copy code
header* block = (header*)((char*)ptr - sizeof(header));
Check if the block is already free to avoid unnecessary work.
Update the status bit (block->is_free = 1).
b. Coalesce Adjacent Free Blocks
Tips:

Coalesce Forward:
If the next block is free, merge it into the current block:
c
Copy code
block->size += block->next->size;
block->next = block->next->next;
Coalesce Backward:
Use the footer of the previous block to locate its header:
c
Copy code
header* prev_block = (header*)((char*)block - prev_block->size);
if (prev_block->is_free) {
    prev_block->size += block->size;
    prev_block->next = block->next;
}
4. Adding Block Footers
a. Writing Footers
Tips:

Footers store the block size for free blocks. Write the footer when freeing or splitting blocks.
Place the footer at the last 8 bytes of the free block:
c
Copy code
footer* block_footer = (footer*)((char*)block + block->size - sizeof(footer));
block_footer->size = block->size;
Update footers whenever a block is split or coalesced.
b. Updating Metadata
Tips:

When freeing a block, ensure the previous bit of the next block is updated:
c
Copy code
if (next_block != NULL) {
    next_block->previous_free = 1;
}
When coalescing:
Update the header and footer of the new combined block.
5. Testing
Testing vmalloc
Request various sizes and verify:
Allocated size matches the aligned size.
Returned pointers are 16-byte aligned.
Best-fit policy is used (inspect heap state after allocations).
Test splitting:
Allocate a small size from a larger block and check the new block is correctly created and free.
Testing vmfree
Free single blocks and verify they are marked as free.
Test coalescing:
Free adjacent blocks and verify they are combined into one.
Check headers and footers for correctness after coalescing.
Helper Functions
Implement functions to traverse and inspect the heap for debugging:
c
Copy code
void print_heap() {
    header* block = heap_start;
    while (block != NULL) {
        printf("Block: size=%zu, free=%d\n", block->size, block->is_free);
        block = block->next;
    }
}
By following these tips and incrementally developing and testing each component, you’ll systematically build a robust implementation of vmalloc and vmfree. Let me know if you'd like further help with any specific part!






