#include <math.h>

uint32_t sqrt(uint32_t val, uint32_t pow)
{
	uint32_t base = val;
	for (uint32_t i = 0; i < pow - 1; i++) {
		val *= base;
	}
	return val;
}
