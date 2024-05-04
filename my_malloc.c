/*
 * Author: Hammad ur Rehman
 */

/* we need this for uintptr_t */
#include <stdint.h>
/* we need this for memcpy/memset */
#include <string.h>
/* we need this to print out stuff*/
#include <stdio.h>
/* we need this for the metadata_t struct and my_malloc_err enum definitions */
#include "my_malloc.h"

/* Function Headers
 * Here is a place to put all of your function headers
 * Remember to declare them as static
 */

/* Our freelist structure - our freelist is represented as a singly linked list
 * the freelist is sorted by address;
 */
metadata_t *address_list;

/* Set on every invocation of my_malloc()/my_free()/my_realloc()/
 * my_calloc() to indicate success or the type of failure. See
 * the definition of the my_malloc_err enum in my_malloc.h for details.
 * Similar to errno(3).
 */
enum my_malloc_err my_malloc_errno;



// -------------------- PART 1: Helper functions --------------------

/* The following prototypes represent useful helper functions that you may want
 * to use when writing your malloc functions. We recommend reading over their
 * documentation and prototypes; having a good idea of the kinds of helpers
 * you can use will make it easier to plan your code.
 *
 * None of these functions will be graded individually. However, implementing
 * and using these will make programming easier. We have provided ungraded test
 * cases these functions that you may check your implementations against.
 */


/* HELPER FUNCTION: find_right
 * Given a pointer to a free block, this function searches the freelist for another block to the right of the provided block.
 * If there is a free block that is directly next to the provided block on its right side,
 * then return a pointer to the start of the right-side block.
 * Otherwise, return null.
 */
metadata_t *find_right(metadata_t *freed_block) {
    metadata_t* curr = address_list;

    //While curr isn't null and havent crossed the freed_block yet
    while (curr && ((uintptr_t)freed_block > (uintptr_t) curr)) {
        curr = curr->next;
    }

    //When we break out of the while loop, freed_block should be to the left of curr
    if (((uintptr_t)freed_block + freed_block->size + TOTAL_METADATA_SIZE) == (uintptr_t)curr) {
        return curr;
    }

    return NULL;
        
}

/* HELPER FUNCTION: find_left
 * This function is the same as find_right, but for the other side of the newly freed block.
 * This function will be useful for my_free(), but it is also useful for my_malloc(), since whenever you sbrk a new block,
 * you need to merge it with the block at the back of the freelist if the blocks are next to each other in memory.
 */

metadata_t *find_left(metadata_t *freed_block) {
    metadata_t *curr = address_list;
    

    //While curr isn't null and is still to the left of freed_block pointer
    while (curr && ((uintptr_t) freed_block > (uintptr_t) curr)) {
        //Immediately to the right of curr
        uintptr_t right_of_curr = ((uintptr_t) curr) + TOTAL_METADATA_SIZE + curr->size;
        if (right_of_curr == (uintptr_t) freed_block) {
            return curr; // if `right_of_curr` is the freed block, then the `curr` block is directly left of `freed_block`
        }
        curr = curr->next;
    }
    
    return NULL;
}

/* HELPER FUNCTION: merge
 * This function should take two pointers to blocks and merge them together.
 * The most important step is to increase the total size of the left block to include the size of the right block.
 * You should also copy the right block's next pointer to the left block's next pointer. If both blocks are initially in the freelist, this will remove the right block from the list.
 * This function will be useful for both my_malloc() (when you have to merge sbrk'd blocks) and my_free().
 */
 void merge(metadata_t *left, metadata_t *right) {
    //All merge should do is merge blocks that are already in the list

     //Update the size of left
     left->size = left->size + TOTAL_METADATA_SIZE + right->size;
     
     //Adjust the pointers
     left->next = right->next;


 }

/* HELPER FUNCTION: split_block
 * This function should take a pointer to a large block and a requested size, split the block in two, and return a pointer to the new block (the right part of the split).
 * Remember that you must make the right side have the user-requested size when splitting. The left side of the split should have the remaining data.
 * We recommend doing the following steps:
 * 1. Compute the total amount of memory that the new block will take up (both metadata and user data).
 * 2. Using the new block's total size with the address and size of the old block, compute the address of the start of the new block.
 * 3. Shrink the size of the old/left block to account for the lost size. This block should stay in the freelist.
 * 4. Set the size of the new/right block and return it. This block should not go in the freelist.
 * This function will be useful for my_malloc(), particularly when the best-fit block is big enough to be split.
 */
 metadata_t *split_block(metadata_t *block, size_t size) {
    //First find the total size of the new block:
     size_t total_size = size + TOTAL_METADATA_SIZE;
    //Pointer tobe returned; cast to uint8_t* for arithematic and then cast abck to metadata_t*
     metadata_t* right_half = (metadata_t *)((uintptr_t)block + TOTAL_METADATA_SIZE + (block->size - total_size));
    //Shrink the older block
     block->size = block->size - total_size;
     right_half->next = NULL;
     right_half->size = size;

     return right_half;
 }

/* HELPER FUNCTION: add_to_addr_list
 * This function should add a block to freelist.
 * Remember that the freelist must be sorted by address. You can compare the addresses of blocks by comparing the metadata_t pointers like numbers (do not dereference them).
 * Don't forget about the case where the freelist is empty. Remember what you learned from Homework 9.
 * This function will be useful for my_malloc() (mainly for adding in sbrk blocks) and my_free().
 */
 void add_to_addr_list(metadata_t *block) {

    //All this function should be responsible for is to add the block to the list appropriately without merging

    //Find the location it should be placed 
     metadata_t* curr = address_list;
    
     //If the freelist is empty
     if (address_list == NULL) {
         address_list = block;
         address_list->next = NULL;
         return;
     }

     while (curr->next && (uintptr_t)(curr->next) < (uintptr_t)block) {
         //iterate up till block is between current and curr->next
         curr = curr->next;
     }

     //If we found a slot in between the list for block to be inserted
     if (curr->next) {
         metadata_t* temp = curr->next;
         curr->next = block;
         block->next = temp;
     }
     else { //If we didn't, add block to the end
         curr->next = block;
         block->next = NULL;
     }

     //see if left merge is possible
     if (((uintptr_t)curr + TOTAL_METADATA_SIZE + curr->size) == (uintptr_t)block) {
         merge(curr, block);
         //See if right merge is possible while left merge already happened
         if (curr->next && ((uintptr_t)block + TOTAL_METADATA_SIZE + block->size) == (uintptr_t)curr->next) {
             merge(curr, curr->next);
         }
     } //See if right merge is possible while left merge wasn't
     else if (curr->next && ((uintptr_t)block + TOTAL_METADATA_SIZE + block->size) == (uintptr_t)curr->next) {
         merge(block, curr->next);
     }
}

/* HELPER FUNCTION: remove_from_addr_list
 * This function should remove a block from the freelist.
 * Simply search through the freelist, looking for a node whose address matches the provided block's address.
 * This function will be useful for my_malloc(), particularly when the best-fit block is not big enough to be split.
 */
 void remove_from_addr_list(metadata_t *block) {
    metadata_t *curr = address_list;
    if (!curr) {
        return;
    } else if (curr == block) {
        address_list = curr->next;
    }

    metadata_t *next;
    while ((next = curr->next) && (uintptr_t) block > (uintptr_t) next) {
        curr = next;
    }
    if (next == block) {
        curr->next = next->next;
    }
}
/* HELPER FUNCTION: find_best_fit
 * This function should find and return a pointer to the best-fit block. See the PDF for the best-fit criteria.
 * Remember that if you find the perfectly sized block, you should return it immediately.
 * You should not return an imperfectly sized block until you have searched the entire list for a potential perfect block.
 * Note that you should NOT be splitting the best-fit block in this function (do this within my_malloc() instead).
 */
 metadata_t *find_best_fit(size_t size) {
     //Few things to keep in mind:
     //1. curr->size is the size of the userdata only
     metadata_t *curr = address_list;
     metadata_t* first_biggest = NULL;
     size_t first_biggest_size = 0;

     if (address_list == NULL) {
         return NULL;
     }

     while (curr) {
         //Immediately to the right of curr
         if (curr->size == size) {
             return curr;
         }
         else if (curr->size > size) {
             //If this curr has less of a size than the last curr, make this priority
             if (first_biggest_size == 0) {
                 first_biggest = curr;
                 first_biggest_size = curr->size;
             }
             else if (curr->size < first_biggest_size) {
                first_biggest = curr;
                first_biggest_size = curr->size;
             }
         }
         curr = curr->next;
     }

     return first_biggest; //Will either be a block big enough or null
 }




// ------------------------- PART 2: Malloc functions -------------------------


/* MALLOC

 */
void *my_malloc(size_t size) {
    my_malloc_errno = NO_ERROR;


    //Making sure that the data is neither too small or too last
    if (size <= 0) {
        my_malloc_errno = NO_ERROR;
        return NULL;
    }

    if (size > SBRK_SIZE - TOTAL_METADATA_SIZE) {
        my_malloc_errno = SINGLE_REQUEST_TOO_LARGE;
        return NULL;
    }

    metadata_t* best_fit = NULL;

    if (address_list != NULL) {
        //Search for a best-fit block. 
        best_fit = find_best_fit(size);
    }

    //cases where best fit is found:
    if (best_fit) {
        if (best_fit->size == size || (best_fit->size - size) < MIN_BLOCK_SIZE) {
            remove_from_addr_list(best_fit);
            best_fit->next = NULL;
            return (metadata_t*)((uintptr_t)best_fit + TOTAL_METADATA_SIZE);
        }
        else {
            metadata_t* right_block = split_block(best_fit, size);
            return (metadata_t*)((uintptr_t)right_block + TOTAL_METADATA_SIZE);
        }
    }

    if (best_fit == NULL) { //If a block was not found or first time calling sbrk:
        // Call sbrk to get a new block.
        metadata_t *sbrk_pointer = my_sbrk(SBRK_SIZE);

        // If sbrk fails (which means it returns -1), return NULL.
        if ((long) sbrk_pointer == -1) {
            my_malloc_errno = OUT_OF_MEMORY;
            return NULL; //sbrk failed
        }
        // If sbrk succeeds, add the new block to the freelist. 
        else {
            sbrk_pointer->size = SBRK_SIZE - TOTAL_METADATA_SIZE;
            sbrk_pointer->next = NULL;
            add_to_addr_list(sbrk_pointer);

            // Go to step 2.
            best_fit = find_best_fit(size);

            if (best_fit) {
                if (best_fit->size == size || (best_fit->size - size) < MIN_BLOCK_SIZE) {
                    remove_from_addr_list(best_fit);
                    best_fit->next = NULL;
                    return (metadata_t*)((uintptr_t)best_fit + TOTAL_METADATA_SIZE);
                }
                else {
                    metadata_t* right_block = split_block(best_fit, size);
                    return (metadata_t*)((uintptr_t)right_block + TOTAL_METADATA_SIZE);
                }
            }
        }
    }
    return NULL;
}

/* FREE

 */
void my_free(void *ptr) {
    my_malloc_errno = NO_ERROR;

    if (ptr == NULL) {
        return;
    }

    // Reminder for how to do free:
    // 1. Since ptr points to the start of the user block, obtain a pointer to the metadata for the freed block.
    metadata_t* ptr_to_md = (metadata_t *) ((uintptr_t)ptr - TOTAL_METADATA_SIZE);

    //2. Place thew freed block in the address_list
    add_to_addr_list(ptr_to_md);
}

/* REALLOC

 */
void *my_realloc(void *ptr, size_t size) {
    my_malloc_errno = NO_ERROR;

    //If ptr is NULL, then only call my_malloc(size). 
    if (ptr == NULL) {
        return my_malloc(size);
    }
    
    //If the size is 0, then only call my_free(ptr).
    if (size <= 0) {
        my_free(ptr);
        return NULL;
    }

//    if (size > SBRK_SIZE - TOTAL_METADATA_SIZE) {
//        my_malloc_errno = SINGLE_REQUEST_TOO_LARGE;
//        return NULL;
//    }

    // 2. Call my_malloc to allocate the requested number of bytes. If this fails, immediately return NULL and do not free the old allocation.
    void* new_block = my_malloc(size);
    if (new_block == NULL) {
        return NULL;
    }

    // 3. Copy the data from the old allocation to the new allocation. We recommend using memcpy to do this. Be careful not to read or write out-of-bounds!
    metadata_t* old_block = (metadata_t*)((uintptr_t)ptr - TOTAL_METADATA_SIZE);
    if (old_block->size < size) {
        memcpy(new_block, ptr, old_block->size);
    }
    else {
        memcpy(new_block, ptr, size);
    }
    // 4. Free the old allocation and return the new allocation.
    my_free(ptr);
    return new_block;
}

/* CALLOC

 */
void *my_calloc(size_t nmemb, size_t size) {
    my_malloc_errno = NO_ERROR;
    
    size_t total_size = nmemb * size;

    void* new_block = my_malloc(total_size);

    if (new_block == NULL) {
        return NULL;
    }

    new_block = memset(new_block, 0, total_size);


    return new_block;
}
