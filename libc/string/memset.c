#include <string.h>

char *memset(char *str, char c, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		str[i] = c;
	}
	return str;
}
