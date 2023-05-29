/*
 * 
 * 
 * THIS WAS FOLLOWING THE GUIDE AT https://arjunsreedharan.org/
 * 
 * Not all of this code is my own. This project was done for learning purposes
 * 
 * 
 * 
 */

#include <stdio.h>
#include <pthread.h>

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
header_t *head, *tail;
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
void *malloc(size_t size) {
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

	// If we get here then there wasn't memory we could use so we have to extend our memory by the header + the requested size.
	block = sbrk(size + sizeof(header_t)); // this extends the memory we have and returns the pointer to the start of it

	// check if the block space was extended successfully
	if (block == (void*) -1) {
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}

	// Set all the values for this header since we allocated it
	header = block;
	header->s.size = size;
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

void free(void *block) {
	// Get header of current block
	header_t *blockHeader = ((header_t *)block-1); // go back by 1 header_t amount

	// Get header of tail block
	header_t *tailHeader = ((header_t *)tail-1); // the 1 is a multiplier for the memory header_t takes

	// Lock
	pthread_mutex_lock(&global_malloc_lock);
	
	// Determine if block to be freed is at the end of the heap by comparing pointers
	if (blockHeader == tailHeader) {
		// Release to the OS
		sbrk(0 - sizeof(header_t) - blockHeader->s.size);

		if (block == (void*) -1) {
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
	
int main() {
	pthread_mutex_init(&global_malloc_lock, NULL);
    void *block1 = malloc(8);
    void *block2 = malloc(16);
    void *block3 = malloc(32);

    // Perform operations on the allocated blocks
    return 0;
}
