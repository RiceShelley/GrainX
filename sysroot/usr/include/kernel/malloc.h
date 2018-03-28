#ifndef __MALLOC__H
#define __MALLOC__H

#include <stdint.h>
#include <stddef.h>

#include <string.h>

#define HEAP_SIZE 1024 * 25

void mm_init(uint32_t kernel_end);

uint32_t heap_addr();

void *malloc(size_t size);

// prints 512 bytes of heap to terminal
void print_heap(uint32_t block);

// returns percentage of heap used
uint8_t heap_used();

void free(void *ptr);

#endif
