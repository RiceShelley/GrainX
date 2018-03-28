#include <stdio.h>

int putchar(int c) 
{
    vga_term_putc(c);
    vga_term_update();
    return 0;
}