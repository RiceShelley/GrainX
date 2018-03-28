#include <string.h>

int str_find_first(const char *str, size_t len, char c) 
{
	for (size_t i = 0; i < len; i++) {
		if (str[i] == c) {
			return (int) i;
		}
	}
	return -1;
}
