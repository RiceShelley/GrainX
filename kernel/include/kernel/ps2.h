#ifndef __PS2__H
#define __PS2__H 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel/io_access.h>
#include <kernel/vga_console.h>
#include <string.h>

// Get scancode from ps2 controler
unsigned char get_scancode();

bool is_scancode_pressed(unsigned char c);

bool is_shift_down();

// Convert scancode value to string representation
const char *scancode_to_str(unsigned char c);

const char *ps2_key_in();

#endif
