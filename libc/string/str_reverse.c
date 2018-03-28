#include <string.h>

const char *str_reverse(char *str, size_t len)
{
        uint8_t pos = 0;
        while (pos < (len / 2)) {
                char s = str[pos];
                char e = str[(len - 1) - pos];
                str[pos] = e;
                str[(len - 1) - pos] = s;
                pos++;
        }
        return (const char *) str;
}
