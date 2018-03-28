#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>
#include <stdint.h>

// copy n words from source into dest
uint16_t *memcpy_uint16_t(uint16_t *dest, uint16_t *source, size_t n);

#endif
