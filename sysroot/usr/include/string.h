#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <stdint.h>
#include <math.h>

// Get length of string terminated with '0'
size_t strlen(const char *str);

const char *str_reverse(char *str, size_t len);

const char *u32_to_str(uint32_t val, char *buff, size_t len);

char *memset(char *str, char c, size_t len);

const char *u8_to_str(unsigned char val, char *buff, size_t len);

int str_find_first(const char *str, size_t len, char c);

// compare n bytes in first to second returns 1 if equal
int8_t memcmp(const char *first, const char *second, size_t len);

// copy n bytes from source into dest
void *memcpy(const void* dest, const void* src, size_t count);

// split string
size_t strsplit(char *str, char delim);

// compare string
size_t strcmp(const char *str1, const char *str2);

// convert string to uint32_t
uint32_t atoi(char *str, size_t len);

#endif
