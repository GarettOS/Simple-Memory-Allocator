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

// This function allocates a specified amount of bytes and will return a pointer to where that memory is
void *malloc(size_t size) {
	if (size == 0) {
		return NULL;
	}

	void *block; // Where the allocated bytes will be held
	block = sbrk(size); // the heap is increased
	if (block == (void*)-1) { // if increasing the memory was unsuccessful
		return NULL;
	}

	return;
}

void free(void *ptr) {
	if (ptr == NULL) {
		return;
	}

	
}

int main() { 
	void *block = malloc(8);
	free(block);
    return 0;
}
