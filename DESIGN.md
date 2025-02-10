Design Question # 1:
#include "vmlib.h"

int main() {
    vminit(128); 

    
    void *a = vmalloc(40); 
    void *b = vmalloc(24); 
    void *c = vmalloc(32); 

    
    vmfree(a); 
    vmfree(c); 

 
    void *d = vmalloc(48);

    vminfo(); 

    vmdestroy();
    return 0;
}
The heap contains two free blocks of 32 and 40 bytes but the 48 vmalloc calls fails because no single block is large enough. There is no free block large enough for a allocation even though the free memory is large enough.
Question #2:
int main() {
    vminit(200); 

    
    void *a = vmalloc(50); 
    void *b = vmalloc(30);
    void *c = vmalloc(70);

    
    vmfree(b); 


    void *d = vmalloc(25); 

    vminfo(); 

    vmdestroy();
    return 0;
}
Best fit can reduce fragmentation because the allocator uses the 30 byte block that was freed for the vmalloc(25) which leaves very little leftover memory space.
With first fit, the allocator can choose something like the 70 byte block if it was free and cause more inefficiency in use of memory. This could lead to allocation failure in further malloc requests later.



UPDATED DESIGN QUESTION:
#include <stdio.h>
#include "vmlib.h"

int main() {
    vminit(2000);  

   
    void *a = vmalloc(200);  // Block A: 200 bytes
    void *b = vmalloc(300);  // Block B: 300 bytes
    void *c = vmalloc(400);  // Block C: 400 bytes
    void *d = vmalloc(500);  // Block D: 500 bytes

    printf("Initial allocation successful.\n");


    vmfree(b);  
    vmfree(d);  

    printf("Freed blocks B and D.\n");

    
    void *e = vmalloc(700);  // This allocation will succeed in the current design.

    if (e != NULL) {
        printf("Allocation of 700 bytes succeeded!\n");
    } else {
        printf("Allocation of 700 bytes failed!\n");
    }

    vmdestroy();
    return 0;
}

If we dont coalesce with previous blocks, memory can be fragemented even though a large enough block of contiguous free memory exists. Coalescing only the next block prevents the merging of multiple smaller free blocks into a single large free block. 
In the current desing, Block C separates Block B and Block D so coalescing is not possible directly across B and D. 
However, if we coalesce both previous blocks and next blocks we can create a large free block of 700 bytes after combining smaller free blocks that are adjacent when we free.
In coalescing only with the next block the free Block B(300 bytes) does not merge with Block D (500 bytes)
So no free block of size 700 exists and allocation fails. This updated design cannot merge free blocks that are separated by busy blocks causes fragmentation. Although there is enough total free memory(800 bytes), it is not contiguous so the request for a 700 byte block fails. In the current design coalescing both previous blocks and the next blocks makes it so that the freed blocks can merge into a bigger block which reduces fragmentation and the allocations succeed.
