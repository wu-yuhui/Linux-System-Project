/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>


// My Implementation of sf_errno messages

#define EINVAL	22	// invalid argument
#define ENOMEM	12 	// out of memory

// 10.08.17

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *sf_malloc(size_t size) {
	
	// 1. Check if size == 0 || size > 4 pages
	//    Return NULL and sf_errno = EINVAL
	if (size == 0 || size > 4*PAGE_SZ){
		sf_errno = EINVAL;
		return NULL;	
	}

	// 2. Find space to allocate

	int startListNum = 0; 
	
	while (startListNum <4){	// Which list to start with
		if (size >= seg_free_list[startListNum].min-16 && size <= seg_free_list[startListNum].max-16)
			break;
		startListNum++; 
	}
	
	/**** error-handle of Myself ****/
	if (startListNum >= 4)	printf("Cannot find correct size in list. ERROR!!\n!");
	/********************************/

	for (;startListNum < 4; startListNum++){	// Find if there is free space in list, if no, next list
		sf_free_header *ptr = seg_free_list[startListNum].head;

		while (ptr != NULL){
			uint64_t blockSize = ptr->header.block_size << 1;
 
			if (blockSize >= size + 16){
				// Check if splitting will cause splinter
				if (blockSize < size + 48){	// Cause splinter, include all size
					/*					
					1. set header alloc = 1, set header block_size = blocksize;
					2. if block_size is not exactly, padded = 1;
					3. do above to footer, and requested_size = size;
					4. return (ptr+HEADER_SIZE)
					*/
				//	*ptr = 1;	ptr->header.block_size = blockSize >> 4;
				//	if (blockSize ) 						
						
				
				}
				else {				// No splinter, split


						

				}

			}
			else	ptr = ptr -> next;
		}
	}

	// 3. Not found in list, call sf_sbrk 
		// if above request still cannot be fulfilled, return NULL and sf_errno = ENOMEM

	double *newSbrkPtr = sf_sbrk();
	if (newSbrkPtr == NULL){
		sf_errno = ENOMEM;
		return NULL;
	} 
	
	return NULL;	

}

void *sf_realloc(void *ptr, size_t size) {

	/*
	1. Call sf_malloc to obtain a larger block
	2. Call memcpy to copy the data in the block given by the user to the block
	   returned by sf_malloc
	3. Call sf_free on the block given by the user (coalescing if necessary)
	   Return the block given to you by sf_malloc to the user


	If sf_malloc returns NULL, sf_realloc must also return NULL. Note that
	you do not need to set sf_errno in sf_realloc because sf_malloc should
	take care of this.
	*/




	/*
	1. Splitting the returned block results in a splinter. In this case, do not
	   split the block. Leave the splinter in the block, update the header and footer
	   fields if necessary, and return the block back to the user. Note that you must
	   not attempt to coalesce the splinter.

	2. The block can be split without creating a splinter. In this case, split the
	   block and update the block size fields in both headers. Free the remaining block
	   (i.e. coalesce if possible and insert the block into the head of the correct
	   free list). Return a pointer to the payload of the smaller block to the user
	*/


	return NULL;
}

void sf_free(void *ptr) {

	// 1. Check if pointer is invalid

	//The pointer is NULL
	//The header of the block is before heap_start or block ends after heap_end
	//The alloc bit in the header or footer is 0
	//The requested_size, block_size, and padded bits do not make sense when put together. 
	//For example, if requested_size + 16 != block_size, you know that the padded bit must be 1.
	//The padded and alloc bits in the header and footer are inconsistent.

	// 2. Add new block to list
		// Coalesce if needed
		// Always first remove the to be coalesced free block out of list and insert it at top always

	return;
}
