/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

static void *traverse_list_to_malloc(size_t, size_t, size_t);
static int call_new_srbk();
static void *front_coalesce(sf_free_header *);
static void *back_coalesce(sf_free_header *);
static void remove_from_list(sf_free_header*, size_t);
static void place(sf_free_header *, size_t, size_t ,size_t);
static void place_request(sf_free_header *, size_t, size_t);
static void *place_next(sf_free_header*, size_t);
static int check_list_ptr(sf_free_header*);
static int check_list_size(size_t);
static void add_to_seg_free_list(sf_free_header *, size_t);
static void check_usrptr_validation(void *ptr);



void *sf_malloc(size_t size) {
	// size: requested by user; asize: rounded up size for allocator use (including header&footer)
	int asize;
	static int SRBK = 0;

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

	int startListNum = check_list_size(asize);
//	printf("ASIZE: %d   ", asize);
//	printf("Start Malloc Size:%d\n", startListNum);

	do {

		sf_free_header *toMallocPtr = traverse_list_to_malloc(startListNum, size, asize);

		if (toMallocPtr != NULL)
			return toMallocPtr;

		// can't find free space to allocate
		// 3. Not found in list, call sf_sbrk
		if (SRBK < 4){
			if (call_new_srbk() == -1)
				return NULL;
			}
		SRBK++;

	} while (SRBK <= 4);

	// if above request still cannot be fulfilled, return NULL and sf_errno = ENOMEM

	sf_errno = ENOMEM;
	return NULL;

}

void *sf_realloc(void *ptr, size_t size) {

	check_usrptr_validation(ptr);

	if (size == 0){
		sf_free(ptr);
		return NULL;
	}

	int asize;
	if (size <= BASESIZE)
		asize = 2*BASESIZE;		/* at least 16 + header(8) + footer(8) = 32 */
	else
		asize = BASESIZE * ((size + BASESIZE + BASESIZE -1)/ BASESIZE);	/*Round up to payload % 16 = 0 + header, footer*/

	sf_free_header *thisHeader = ptr - ROWSIZE;
	int blockSize = thisHeader->header.block_size << 4;

	if (blockSize < asize){

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
		sf_free_header *largeMalloc = sf_malloc(size);

		if (largeMalloc == NULL)	return NULL;

		memcpy(largeMalloc, ptr, size);
		sf_free(ptr);

		return largeMalloc;

	}
	else{

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

		if (blockSize < asize + MINBLKSIZE){
			// No splinter
			place_request(thisHeader, blockSize, size);

			// padding if it is not exact
			thisHeader->header.padded = !(blockSize == asize);
			sf_footer *thisFooter = (void*)(thisHeader + blockSize -ROWSIZE);
			thisFooter->padded = !(blockSize == asize);

/*			printf("No splinter allocated one\n");
			sf_blockprint(thisHeader);
			sf_snapshot();
*/
			return ptr;
		}
		else {
			// SPLIT
			place(thisHeader, asize, (size+16) != asize, 1);
			place_request(thisHeader, asize, size);
			sf_free_header *splitFreeHeader = place_next(thisHeader, blockSize - asize);

			place_request(splitFreeHeader, blockSize - asize, 0);
			sf_free_header *backCoalescePtr = back_coalesce(splitFreeHeader);
			add_to_seg_free_list(backCoalescePtr, check_list_ptr(backCoalescePtr));


/*			printf("allocated one\n");
			sf_blockprint(thisHeader);
			printf("Split new one\n");
			sf_blockprint(backCoalescePtr);
			sf_snapshot();
*/
			return ptr;


		}


	}


	return NULL;
}

void sf_free(void *ptr) {

	// 1. Check if pointer is invalid
	check_usrptr_validation(ptr);

	// 2. Add new block to list
		// Coalesce if needed
		// Always first remove the to be coalesced free block out of list and insert it at top always

	sf_header *thisHeader = ptr - ROWSIZE;
	sf_footer *thisFooter = ptr - ROWSIZE*2 + (thisHeader->block_size << 4);

	sf_free_header *thisFreeHeader = (sf_free_header *)thisHeader;
	thisHeader->allocated = 0;
	thisFooter->allocated = 0;
	thisHeader->padded = 0;
	thisFooter->padded = 0;
	thisFooter->requested_size = 0;
	//sf_blockprint(thisFreeHeader);
	//sf_blockprint(thisHeader);
	//sf_snapshot();

	sf_free_header *backCoalescePtr = back_coalesce(thisFreeHeader);

	add_to_seg_free_list(backCoalescePtr, check_list_ptr(backCoalescePtr));

/*	printf("New Freed Memory\n");
	sf_blockprint(backCoalescePtr);
	sf_snapshot();
*/


	return;
}

static void *traverse_list_to_malloc(size_t listNum, size_t size, size_t asize){

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
		sf_free_header *ptr = seg_free_list[i].head;

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
					remove_from_list(ptr, i);
					place(ptr, blockSize, 1, 1);
					place_request(ptr, blockSize, size);

/*					printf("No splinter allocated one\n");
					sf_blockprint(ptr);
					sf_snapshot();
*/
					void *payloadPtr = (void*)ptr+ROWSIZE;

					return payloadPtr;

				}
				else {				// No splinter, split

					remove_from_list(ptr, i);
					place(ptr, asize, (size+16) != asize, 1);
					place_request(ptr, asize, size);
					sf_free_header *splitFreeHeader = place_next(ptr, blockSize - asize);
					add_to_seg_free_list(splitFreeHeader, check_list_ptr(splitFreeHeader));


/*					printf("allocated one\n");
					sf_blockprint(ptr);
					printf("Split new one\n");
					sf_blockprint(splitFreeHeader);
					sf_snapshot();
*/
					void *payloadPtr = (void*)ptr+ROWSIZE;

					return payloadPtr;
				}

			}
			else	ptr = ptr -> next;
		}

	}

	return NULL;


}

static int call_new_srbk(){

	sf_free_header *sbrkPtr = sf_sbrk();

	if (sbrkPtr == NULL){
		sf_errno = ENOMEM;
		return -1;
	}

	sf_free_header *heap_end = get_heap_end();

	// Place header, footer for srbk block. Then front coalesce, and place again.
	place(sbrkPtr, (void*)heap_end - (void*)sbrkPtr, 0, 0);
	sf_free_header *frontCoalescePtr = front_coalesce(sbrkPtr);
	place(frontCoalescePtr, (void*)heap_end - (void*)frontCoalescePtr, 0, 0);


	//	sf_blockprint(frontCoalescsPtr);

	// Put in free list;
	add_to_seg_free_list(frontCoalescePtr, check_list_ptr(frontCoalescePtr));


//	printf("New Srbk\n");
//	sf_blockprint(frontCoalescePtr);
//	sf_snapshot();

	return 0;

}


static void *front_coalesce(sf_free_header *ptr){
	// TODO
	// Front coalesce after sbrk if needed
	void *front = (void*)ptr;
	sf_footer *prevFooter = front-ROWSIZE;

	if (prevFooter->allocated == 0){
	/* previous footer allocated = 0, then update own footer and previous header to new size */
		int blockSize = prevFooter->block_size << 4;

		//sf_header *prevHeader = front-blockSize;
		sf_free_header *toCoalesce = front-blockSize;

		remove_from_list(toCoalesce, check_list_ptr(toCoalesce));

		/*
		prevHeader->block_size += blockSize >> 4;
		sf_footer *myFooter = front + ptr->header.block_size - ROWSIZE;
		myFooter->block_size = prevHeader->block_size;
		front -= blockSize;
		*/

		place(toCoalesce, toCoalesce->header.block_size + ptr->header.block_size, 0, 0);

		return toCoalesce;
	}
	return front;
}

static void *back_coalesce(sf_free_header *ptr){
	// TODO
	void *back = (void*)ptr;
	int thisBlockSize = ptr->header.block_size << 4;
	sf_free_header *nextHeader = back + thisBlockSize;

	if (nextHeader->header.allocated == 0){
		int nextblockSize = nextHeader->header.block_size << 4;

		remove_from_list(nextHeader, check_list_ptr(nextHeader));

		place(back, nextblockSize + thisBlockSize, 0, 0);

		return back;
	}
	return back;

}




static void remove_from_list(sf_free_header* ptr, size_t listNum){
	//	Remove allocated area from segregated free list

	/* If only one element lleft in list, all NULL */
	if (ptr -> next == NULL && ptr -> prev == NULL){
		seg_free_list[listNum].head = NULL;
	}
	else {
		if (ptr -> next != NULL)
			ptr -> next -> prev = ptr -> prev;

		if (ptr -> prev != NULL)
			ptr -> prev -> next = ptr -> next;
		else	/* Take care list head to new head */
			seg_free_list[listNum].head = ptr -> next;
	}

	return;

}


static void place(sf_free_header *start, size_t size, size_t padded, size_t allocated){

	void *voidstart = start;

	sf_header *header = voidstart;
	header->allocated = allocated;
	header->padded = padded;
	header->block_size = size >> 4;

	sf_footer *footer = voidstart + size - ROWSIZE;
	footer->allocated = allocated;
	footer->padded = padded;
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

static int check_list_size(size_t size){

	int	listNum = 0;

	while (listNum < 4){	// Which list to start with

		if (size >= seg_free_list[listNum].min && size <= seg_free_list[listNum].max)
			return listNum;
		listNum++;
	}
	return -1;
}


static int check_list_ptr(sf_free_header *ptr){
	size_t size = ptr->header.block_size << 4;

	return check_list_size(size);

}

static void add_to_seg_free_list(sf_free_header *newFree, size_t startListNum){

	if (seg_free_list[startListNum].head == NULL){
		newFree -> next = NULL;
		newFree -> prev = NULL;
		seg_free_list[startListNum].head = newFree;
	}
	else {
		newFree -> next = seg_free_list[startListNum].head;
		newFree -> prev = NULL;
		seg_free_list[startListNum].head -> prev = newFree;
		seg_free_list[startListNum].head = newFree;
	}

//	printf("Seg list working\n" );

}

static void check_usrptr_validation(void *ptr){
	//The pointer is NULL
	if (ptr == NULL)	abort();

	//The header of the block is before heap_start or block ends after heap_end
	sf_header *thisHeader = ptr - ROWSIZE;
	sf_footer *thisFooter = ptr - ROWSIZE*2 + (thisHeader->block_size << 4);


	if (thisHeader < (sf_header*)get_heap_start() || thisFooter > (sf_footer*)get_heap_end())
		abort();

	//The alloc bit in the header or footer is 0
	if (thisHeader->allocated == 0 || thisFooter->allocated == 0)
		abort();

	//The requested_size, block_size, and padded bits do not make sense when put together.
	//For example, if requested_size + 16 != block_size, you know that the padded bit must be 1.

	if ((thisHeader->block_size << 4) - thisFooter->requested_size > 16 && !thisHeader->padded)	abort();
	if ((thisFooter->block_size << 4) - thisFooter->requested_size > 16 && !thisFooter->padded)	abort();
	if ((thisHeader->block_size << 4) - thisFooter->requested_size <= 16 && thisHeader->padded)	abort();
	if ((thisFooter->block_size << 4) - thisFooter->requested_size <= 16 && thisFooter->padded)	abort();

	//The padded and alloc bits in the header and footer are inconsistent.
	if (thisHeader->block_size != thisFooter->block_size || thisHeader->padded != thisFooter-> padded)
		abort();

	return;

}