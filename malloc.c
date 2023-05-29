/*
 * 
 * 
 * THIS WAS FOLLOWING THE GUIDE AT https://arjunsreedharan.org/
 * 
 * The Malloc and Get_free_block are not mine. This project was done for learning purposes
 * 
 * 
 * 
 */

#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

typedef char ALIGN[16];

union header {
	struct {
		size_t size;
		unsigned is_free;
		union header *next;
	} s;
	ALIGN stub;
};

typedef union header header_t;
header_t *head, *tail = NULL;
pthread_mutex_t global_malloc_lock;

// Search for a free block of memory in our allocated memory
header_t *get_free_block(size_t size) {
	header_t *curr = head;

	// Traverse the linked list and search for a free memory block
	while(curr != NULL) {
		if (curr->s.is_free == 1 && curr->s.size >= size) { // check if it is free and has enough space
			return curr;
		} else {
			curr = curr->s.next;
		}
	}
	return NULL;
}

// This function allocates a specified amount of bytes and will return a pointer to where that memory is
void *my_malloc(size_t size) {
	size_t total_size;
	void *block;
	header_t *header;

	// Check if the requested size is 0
	if (size == 0) {
		return NULL;
	}

	// For a valid size, we should lock the memory allocation
	pthread_mutex_lock(&global_malloc_lock);
	header = get_free_block(size); // search for a free block in our current allocated memory

	if (header) {
		header->s.is_free = 0; // set this block to not free anymore
		pthread_mutex_unlock(&global_malloc_lock);
		return ((void*) header+1); // return header+1 because we want it to start at the memory block which is right after the header in memory so pointer is now +1 further
	}

	total_size = size + sizeof(header_t); 
	block = sbrk(total_size);

	if (block == (void*) -1) {
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}

	header = block;
	header->s.size = size; // now this should work without causing a segfault

	header->s.is_free = 0;
	header->s.next = NULL;

	// Now set this up to be linked together with other memory blocks
	if (!head) { // If theres no blocks yet
		head = header;
	}

	// If there is blocks already append this to the end of the linked list
	if (tail) {
		tail->s.next = header;
	}

	// This is now the new tail
	tail = header;

	pthread_mutex_unlock(&global_malloc_lock);

	return ((void*) header+1);
}

void my_free(void *block) {
	
    if (!block) {
        return;
    }

	// Get header of current block
	header_t *blockHeader = ((header_t *)block-1); // go back by 1 header_t amount

	// Get header of tail block
	header_t *tailHeader = ((header_t *)tail-1); // the 1 is a multiplier for the memory header_t takes

	// Lock
	pthread_mutex_lock(&global_malloc_lock);
	
	// Determine if block to be freed is at the end of the heap by comparing pointers
	if (blockHeader == tailHeader) {
		// Release to the OS
		void *result = sbrk(0-sizeof(header_t)-blockHeader->s.size);

		if (result == (void*) -1) {
			pthread_mutex_unlock(&global_malloc_lock);	
			return;
		}

		// Update the linked list and remove the tail
		header_t *temp = head;
		while(temp->s.next != tail) {
			temp = temp->s.next;
		}
		temp->s.next = NULL;
		tail = temp;

		// Unlock
		pthread_mutex_unlock(&global_malloc_lock);	
		return;
	}

	// If the block to be freed isn't at the end of the head, then mark it as free
	blockHeader->s.is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);	
	return;
}

void *my_calloc(size_t num, size_t nsize) {
	void *block;
	if (num || nsize == 0) {
		return NULL;
	}
	block = my_malloc(num*nsize);
	if(block == 0) {
		return NULL;
	}
	memset(block, 0, num * nsize);
	return block;
}

void *my_realloc(void *block, size_t size) {
    if (!block) {
        return my_malloc(size);
    }
    if (size == 0) {
        my_free(block);
        return NULL;
    }

    pthread_mutex_lock(&global_malloc_lock);
	
    header_t *blockHeader = ((header_t*)block - 1);

    if (size <= blockHeader->s.size) {
        pthread_mutex_unlock(&global_malloc_lock);
        return block;
    }

    void *new_block = my_malloc(size);
    if (!new_block) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL; 
    }

    size_t old_size = blockHeader->s.size;
    if (size < old_size) {
        old_size = size;
    }

    memcpy(new_block, block, old_size);
    my_free(block);
    pthread_mutex_unlock(&global_malloc_lock);
	
    return new_block;
}
