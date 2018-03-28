#include <kernel/malloc.h>

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;

void mm_init(uint32_t kernel_end) 
{
    last_alloc = kernel_end + 0x10000;
    heap_begin = last_alloc;
    heap_end = heap_begin + HEAP_SIZE;
    char *ptr = last_alloc;
    memset(ptr, '$', HEAP_SIZE);
    memcpy(&ptr[HEAP_SIZE], "END_OF_HEAP", strlen("END_OF_HEAP"));
    memcpy(ptr - strlen("START_OF_HEAP"), "START_OF_HEAP", strlen("START_OF_HEAP"));
}

void print_heap(uint32_t block)
{
	if (block < 0 && block > HEAP_SIZE)
		return;

	uint8_t *h_ptr = heap_begin;
	h_ptr = &h_ptr[256 * block];

	for (uint32_t row = 0; row < 16; row++) {
		for (uint32_t col = 0; col < 16; col++) {
			uint8_t b = h_ptr[(row * 16) + col];
			if (b > 0xF)
				printf("%x ", b);
			else
				printf("0%x ", b);
		}
		printf("\n");
	}
}

uint8_t heap_used()
{
    char *ptr = heap_begin;
    double used = 0;
    for (uint32_t i = 0; i < HEAP_SIZE; i++) {
        if (ptr[i] != '$')
            used++;
    }
    printf("%d bytes used.\n", (uint32_t) used);
    return (uint8_t) ((used / ((double) HEAP_SIZE)) * 100.0);
}

uint32_t heap_addr() 
{
    return last_alloc;
}

void *find_block(size_t size) 
{
    size_t data_size = size;
	size += 6 + sizeof(size_t);
    char *ptr = last_alloc;
    for (size_t i = 0; i < size; i++) {
        if (ptr[i] == '$') {
            continue;
		}
        i = -1;
        ptr++;
    }
	// Move ptr 3 bytes fwd to prevent buffer overflows
	ptr += 3;
    memcpy(ptr, &data_size, sizeof(size_t));
    ptr += sizeof(size_t);
	memset(ptr, 0, data_size);
		for (uint32_t i = 0; i < (uint32_t) data_size; i++) {
				if (ptr[i] != 0) {
					printf("what tf\n");
				}
			}
    return (void *) ptr;
}

void *malloc(size_t size) 
{
    return (void *) find_block(size);
}

void free(void *ptr)
{
    // find size of alloc
    ptr -= sizeof(size_t);
    size_t len = 0;
    memcpy(&len, ptr, sizeof(size_t));
    len += sizeof(size_t);
    // delete data
    memset(ptr, '$', len);
}
