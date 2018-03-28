#include <string.h>

int8_t memcmp(const char *first, const char *second, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (first[i] == second[i])
			continue;
		return 0;
	}
	return 1;
}
