#ifndef __STDIO__H
#define __STDIO__H

#include <stdarg.h>
#include <string.h>
#include <kernel/ext2.h>
#include <kernel/vga_console.h>

    int printf(const char *__restrict, ...);
    int sprintf(const char *__restict, ...);
    int putchar(int c);
    int puts(const char *str);
    // read n bytes from file -> return amount read
    size_t read(const char *path, void *buff, size_t n);

#endif
