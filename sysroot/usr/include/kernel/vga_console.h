#ifndef __VGA_CONSOLE__H
#define __VGA_CONSOLE__H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <mem.h>
#include <kernel/io_access.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Virtual term buffer is 4 times bigger so we can keep a history
#define VIRT_WIDTH 80
#define VIRT_HEIGHT 100

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

size_t t_row;
size_t t_col;
uint8_t term_color;
uint16_t *term_buff;
// Varible tells where to start printing the virtual terminal to physical
size_t virt_term_start_line;

uint16_t virtual_term_buff[VIRT_WIDTH * (VIRT_HEIGHT + 2)];

void vga_term_init(void);

void vga_term_set_color(enum vga_color fg, enum vga_color bg);

void vga_term_clear();

void vga_term_putc_at(char c, uint8_t color, size_t x, size_t y);

void vga_term_putc(char c);

void vga_term_update_curse_pos();

void vga_term_pos(size_t val);

void vga_term_write(const char *data, size_t size);

void vga_term_update(void);

void vga_term_write_str(const char *data);

#endif
