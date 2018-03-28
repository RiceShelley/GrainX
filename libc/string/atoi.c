#include <string.h>

uint32_t atoi(char *str, size_t len)
{
	uint32_t rtn = 0;
	uint8_t pow = 0;
	str--;
	for (size_t i = len; i > 0; i--) {
		pow++;
		uint32_t num = str[i] - '0';
		if (pow == 1)
			rtn += num;
		else
			rtn += num * sqrt(10, pow - 1);	
	}
	return rtn;
}
