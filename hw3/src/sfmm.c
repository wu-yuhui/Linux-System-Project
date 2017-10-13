/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define	ROWSIZE	8
#define	BASESIZE	16
#define MINBLKSIZE	32

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

/* User Defined Function */


static void *front_coalesce(sf_free_header *);
static void remove_from_list(sf_free_header*, size_t);
static void place(sf_free_header *, size_t, size_t ,size_t);
static void place_request(sf_free_header *, size_t, size_t);
static void *place_next(sf_free_header*, size_t size);
static int checkWhichList(sf_free_header*);
static void add_to_seg_free_list(sf_free_header *, size_t);



void *sf_malloc(size_t size) {

	// size: requested by user; asize: rounded up size for allocator use (including header&footer)
	int asize;

	// 1. Check if size == 0 || size > 4 pages

	if (size == 0 || size > 4*PAGE_SZ){
		sf_errno = EINVAL;
		return NULL;
	}

	if (size <= BASESIZE)
		asize = 2*BASESIZE;		/* at least 16 + header(8) + footer(8) = 32 */
	else
		asize = BASESIZE * ((size + BASESIZE + BASESIZE -1)/ BASESIZE);	/*Round up to payload % 16 = 0 + header, footer*/


	// 2. Find space to allocate

	int startListNum = 0;

	while (startListNum <4){	// Which list to start with
		if (asize >= seg_free_list[startListNum].min && asize <= seg_free_list[startListNum].max)
			break;
		startListNum++;
	}


	/**** error-handle of Myself ****/
	if (startListNum >= 4)	printf("Cannot find correct size in list. ERROR!!\n");
	/********************************/

	printf("%d\n", startListNum);


	for (;startListNum < 4; startListNum++){	// Find if there is free space in list, if no, next list
		sf_free_header *ptr = seg_free_list[startListNum].head;

		while (ptr != NULL){
			uint64_t blockSize = ptr->header.block_size << 4;

			printf("%p\n", &ptr->header);

			if (blockSize >= asize){
				// Check if splitting will cause splinter
				if (blockSize < asize + MINBLKSIZE){	// Cause splinter, include all size
					/*
					1. set header alloc = 1, set header block_size = blockSize;
					2. if block_size is not exactly, padded = 1;
					3. do above to footer, and requested_size = size;
					4. return (ptr+HEADER_SIZE)
					*/

					return NULL;

				}
				else {				// No splinter, split



					return NULL;
				}

			}
			else	ptr = ptr -> next;
		}
	}

	// 3. Not found in list, call sf_sbrk
		// if above request still cannot be fulfilled, return NULL and sf_errno = ENOMEM


	sf_free_header *sbrkPtr = sf_sbrk();

	if (sbrkPtr == NULL){
		sf_errno = ENOMEM;
		return NULL;
	}

	sf_free_header *heap_end = get_heap_end();

	// Place header, footer for srbk block. Then front coalesce, and place again.
	place(sbrkPtr, (void*)heap_end - (void*)sbrkPtr, 0, 0);
	sf_free_header *frontCoalescsPtr = front_coalesce(sbrkPtr);
	place(frontCoalescsPtr, (void*)heap_end - (void*)frontCoalescsPtr, 0, 0);

	sf_blockprint(frontCoalescsPtr);

	// put in free list;
	add_to_seg_free_list(frontCoalescsPtr, checkWhichList(frontCoalescsPtr));

	sf_blockprint(frontCoalescsPtr);
	sf_snapshot();


	// Malloc
	int listNum;
	if ((listNum = checkWhichList(frontCoalescsPtr)) == -1)
		/* Do something!!!  call sbrk again */
		printf("Not fit in list\n");

	/*
	1. Traverse list of starting point
	2. Check size
		-> Will make Splinter:  give all
		-> Won't 			 :  split
	3. Split funtion
		(1) Remove from list
		(2) Allocate what we wanted
		(3)	Allocate free for the other
		(4) Add free to segregate list
	*/

	for (int i = listNum; i < 4; i++){	// Find if there is free space in list, if no, next list
		sf_free_header *ptr = seg_free_list[startListNum].head;

		while (ptr != NULL){
			size_t blockSize = ptr->header.block_size << 4;

			if (blockSize >= asize){
				// Check if splitting will cause splinter
				if (blockSize < asize + MINBLKSIZE){	// Cause splinter, include all size
					/*
					1. set header alloc = 1, set header block_size = blockSize;
					2. if block_size is not exactly, padded = 1;
					3. do above to footer, and requested_size = size;
					4. return (ptr+HEADER_SIZE)
					*/
					remove_from_list(ptr, listNum);
					place(ptr, blockSize, 1, 1);
					place_request(ptr, blockSize, size);

					sf_blockprint(ptr);

					void *payloadPtr = (void*)ptr+ROWSIZE;

					return payloadPtr;

				}
				else {				// No splinter, split

					remove_from_list(ptr, listNum);
					place(ptr, asize, 0, 1);
					place_request(ptr, asize, size);
					sf_free_header * splitFreeHeader = place_next(ptr, blockSize - asize);
					add_to_seg_free_list(splitFreeHeader, checkWhichList(splitFreeHeader));


					printf("allocated one\n");
					sf_blockprint(ptr);
					printf("Split new one\n");
					sf_blockprint(splitFreeHeader);

					void *payloadPtr = (void*)ptr+ROWSIZE;

					return payloadPtr;
				}

			}
			else	ptr = ptr -> next;
		}

		if (i == 3)
			/* Cannot find, call sbrk */;

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



/* Not test it thoroughly yet */
static void *front_coalesce(sf_free_header *ptr){
	// TODO
	// Front coalesce after sbrk if needed
	void *front = (void*)ptr;
	sf_footer *prevFooter = front-ROWSIZE;

	if (prevFooter->allocated == 0){
	/* previous footer allocated = 0, then update own footer and previous header to new size */
		int blockSize = prevFooter->block_size << 4;

		sf_header *prevHeader = front-blockSize;
		prevHeader->block_size += blockSize >> 4;

		sf_footer *myFooter = front + ptr->header.block_size - ROWSIZE;
		myFooter->block_size = prevHeader->block_size;

		front -= blockSize;
	}
	return front;
}

static void remove_from_list(sf_free_header* ptr, size_t listNum){
	//	TODO
	//	Remove allocated area from segregated free list

	sf_free_header *findPtr = seg_free_list[listNum].head;

	while (findPtr != NULL){

			if (findPtr == ptr){
				// next pointer not NULL -> its previous gets my previous
				if (ptr -> next != NULL)
					ptr -> next -> prev = ptr -> prev;
				// previous pointer  if NULL -> list head doesn't have next
				if (ptr -> prev == seg_free_list[listNum].head)
					seg_free_list[listNum].head = ptr -> next;
				else
					ptr -> prev -> next = ptr -> next;
			}
			else	findPtr = findPtr -> next;
		}
}


static void place(sf_free_header *start, size_t size, size_t padded, size_t allocated){

	void *voidstart = start;

	sf_header *header = voidstart;
	header->allocated = 0;
	header->padded = 0;
	header->block_size = size >> 4;

	sf_footer *footer = voidstart + size - ROWSIZE;
	footer->allocated = 0;
	footer->padded = 0;
	footer->block_size = size >> 4;

}

static void place_request(sf_free_header *start, size_t size, size_t requested_size){

	void *voidstart = start;

	sf_footer *footer = voidstart + size - ROWSIZE;
	footer->requested_size = requested_size;
}

static void *place_next(sf_free_header* start, size_t free_size){

	void *voidstart = start;
	size_t startSize = start->header.block_size << 4;

	sf_free_header *nextHeader = voidstart + startSize;

	place(nextHeader, free_size, 0, 0);

	return nextHeader;

}

static int checkWhichList(sf_free_header *ptr){

	int	listNum = 0;
	size_t size = ptr->header.block_size << 4;

//	printf("size: %lu\n", size);

	while (listNum < 4){	// Which list to start with
		if (size >= seg_free_list[listNum].min && size <= seg_free_list[listNum].max)
			return listNum;
		listNum++;
	}
	return -1;
}

static void add_to_seg_free_list(sf_free_header *newFree, size_t startListNum){

	if (seg_free_list[startListNum].head == NULL)
		newFree -> prev = (void *)&seg_free_list[startListNum].head;
	else
		newFree -> prev = seg_free_list[startListNum].head-> prev;

	newFree -> next = seg_free_list[startListNum].head;

	if (seg_free_list[startListNum].head != NULL)
		seg_free_list[startListNum].head -> prev = newFree;

	seg_free_list[startListNum].head = newFree;


	printf("Seg list working\n" );

}