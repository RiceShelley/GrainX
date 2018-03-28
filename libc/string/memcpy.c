#include <string.h>

/*const char *memcpy(char *dest, const char *source, size_t n)
{
	for (size_t i = 0; i < n; i++)
		dest[i] = source[i];
	return (const char *) dest;
}*/
void *memcpy(const void* dest, const void* src, size_t count)
{
	char* dst8 = (char*)dest;
	char* src8 = (char*)src;

	if (count & 1) {
		dst8[0] = src8[0];
		dst8 += 1;
		src8 += 1;
	}

	count /= 2;
	while (count--) {
		dst8[0] = src8[0];
		dst8[1] = src8[1];

		dst8 += 2;
		src8 += 2;
	}
	return (void*)dest;
}
