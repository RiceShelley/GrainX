#include <stdio.h>

int puts(const char *str)
{
    return printf((const char *) str);
}