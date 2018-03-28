#include <mem.h>

uint16_t *memcpy_uint16_t(uint16_t *dest, uint16_t *source, size_t n)
{
	for (size_t i = 0; i < n; i++) 
		dest[i] = source[i];
	return dest;
}
