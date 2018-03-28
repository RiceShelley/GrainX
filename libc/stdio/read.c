#include <stdio.h>

size_t read(const char *path, void *buff, size_t n)
{
    // Use buff on the heap to minimize damage from unexpected overflows
    char *f_buff = malloc(1024 * 12);
    size_t rtn = ext2_read_file((char *) path, f_buff);
    if (rtn == 0) {
        printf("failed to read from file: '%s'\n", path);
        free(f_buff);
        return 0;
    }
    // if n bytes is greater than bytes read -> cpy amt read, null terminate, and return len
    if (n > rtn) {
        memcpy((const void *)buff, f_buff, rtn);
        ((char *) buff)[rtn + 1] = 0;
        free(f_buff);
        return rtn;
    }
    // if n bytes is less than bytes read -> return n - 1 bytes read and null terminate
    memcpy((const void *) buff, f_buff, n);
    ((char *) buff)[n] = 0;
    free(f_buff);
    return n;
}