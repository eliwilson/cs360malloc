/* Eli Wilson - CS360
 * 11/2/2022
 * This program implements the malloc and free functions by creating buffers using sbrk()
 * and storing freed memory in a linked list
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

typedef struct node {
    /* Number of bytes in block */
    unsigned int size; 

    /* Link to next free node in free list */
    struct node *next; 
} Node;

/* Global free list head pointer */
Node *head = NULL;

// Prints out entire free list, used for debugging
void printFL(){
    Node *n = head;
    printf("--------start---------\n");
    while(n != NULL){
        printf("addr: %p, size: %d, next: %p\n", n, n->size, n->next);
        n = n->next;
    }
    printf("--------end---------\n");
}

void *free_list_begin(){
    return (void *)head;
}

void *free_list_next(void *node){
    Node *n = (Node *)node;
    return n->next;
}

// allocates a block, used for blocks > 8192 bytes
Node * nalloc(size_t size){
    Node *n = sbrk(size);
    n->size = size;
    return n;
}

// Frees block by adding it to linked list of freed memory
void my_free(void *ptr){
    Node *prev, *n, *p;
    p = (Node *)(ptr - 8);

    if(!head){
        head = p; //sets head if ptr is first free node [minus 8?]
        head->next = NULL;
        return;
    }

	// Inserts block into free list sorted by address
    n = head;
    prev = head;
    while(1){
        if(n > p){
            if(n == head){
                p->next = head;
                head = p;
            }else{
                prev->next = p;
                p->next = n;
            }
            break;
        }

        prev = n;
        if(n->next != NULL){
            n = n->next;
        }else{
            prev->next = p;
            p->next = NULL;
            break; // Reached end of free list
        }
        
    }
}

// Creates new block of memory if free list 
// does not contain a valid block
Node * newBlock(size_t size){
    Node *n1, *n2;

    if(size >= 8192){ // Large blocks use unbuffered system call
        return nalloc(size);
    }else{ // Small blocks make large system call and split into two blocks
        n1 = sbrk(8192);
        n1->size = size;
        n2 = (void *)n1 + size;
        n2->size = 8192 - size;
        n2->next = NULL;
        my_free((void *)n2 + 8);
		return n1;
    }
}

// Checks free list for valid block, else: calls newBlock
void *my_malloc(size_t size){
    int padding;
    unsigned int blockSize;
    size_t bytes;
    Node *n, *prev, *newNode, *nnext;

	// Adds padding and 8 bookkeeping bytes
    padding = (8 - (size % 8)) % 8;
    bytes = size + padding + 8;

    /* search for first free block which is bigger than 'total_bytes' */
    n = free_list_begin();
    prev = n;
    while(n != NULL){
        blockSize = n->size;
        if(blockSize >= bytes){
            if(blockSize - bytes > 8){

                //Takes node from front of first large enough block
                newNode = n;
                nnext = n->next;
                newNode->size = bytes;
                n = (void *)newNode + bytes;
                n->size = blockSize - bytes;
                n->next = nnext;
				
                if(newNode == head){
                    head = n;
                }else{
                    prev->next = n;
                }

                return (void *)newNode + 8;

            }else{ // block is exact size
				n->size = bytes; // set size

                if(n == head){
                    head = n->next;
                }else{
					prev->next = n->next; // remove from fl
                }

				return (void *)n + 8;
            }
        }

        prev = n;
        n = free_list_next(n);
    }
   
	// Creates new block if none available
    newNode = newBlock(bytes);
    return (void *)newNode + 8;

}

// Combines adjacent free list nodes
void coalesce_free_list(){
	Node *node;

	node = head;
	while(node != NULL){
		while((void*)node + node->size == (void*)node->next){ 
			node->size += node->next->size; 
			node->next = node->next->next; 
		}
		node = node->next;
	}
}
