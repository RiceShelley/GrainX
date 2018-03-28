#include <string.h>

const char *u32_to_str(uint32_t val, char *buff, size_t len)
{
        size_t i = 0;
        while (val > .9 && i < len) {
                uint8_t r = val % 10;
                char c = r + '0';
                buff[i] = c;
                i++;
                val = val / 10;
        }

        // This is what happens when the funtion is passed 0 in val
        if (i == 0) {
                buff[i] = '0';
                i++;
        }

        buff[i] = 0;
        //return (const char *) buff;
        return str_reverse(buff, i);
}

const char *u8_to_str(unsigned char val, char *buff, size_t len)
{
        size_t i = 0;
        while (val > .9 && i < len) {
                uint8_t r = val % 10;
                char c = r + '0';
                buff[i] = c;
                i++;
                val = val / 10;
        }

        // This is what happens when the funtion is passed 0 in val
        if (i == 0) {
                buff[i] = '0';
                i++;
        }

        buff[i] = 0;
        //return (const char *) buff;
        return str_reverse(buff, i);
}
