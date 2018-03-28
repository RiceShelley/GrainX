#include <kernel/vga_console.h>

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void vga_term_init(void)
{
	virt_term_start_line = VIRT_HEIGHT - VGA_HEIGHT;
	
	t_row = virt_term_start_line;
	t_col = 0;
	term_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
	term_buff = (uint16_t *) 0xB8000;
	// Init pysical term buffer
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term_buff[index] = vga_entry(' ', term_color);
		}
	}
	// Init virtual term buffer
	for (size_t y = 0; y < VIRT_HEIGHT; y++) {
		for (size_t x = 0; x < VIRT_WIDTH; x++) {
			const size_t index = y * VIRT_WIDTH + x;
			virtual_term_buff[index] = vga_entry(' ', term_color);
		}
	}
	printf("VGA terminal initialized.\n\r");
}

void vga_term_set_color(enum vga_color fg, enum vga_color bg)
{
	term_color = vga_entry_color(fg, bg);
}

void vga_term_clear()
{
	for (uint8_t y = 0; y < VIRT_HEIGHT; y++) {
		for (uint8_t x = 0; x < VIRT_WIDTH; x++) {
			virtual_term_buff[y * VIRT_WIDTH + x] = vga_entry(' ', term_color);
		}
	}
	t_row = virt_term_start_line;
	t_col = 0;
	vga_term_update_curse_pos();
	vga_term_write_str("Term cleared.\r\n\0");
}

void vga_term_write_pysical()
{
	// Write section of virtual term to pysical
	uint16_t *virt_term_start = &virtual_term_buff[virt_term_start_line * VIRT_WIDTH];
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			term_buff[index] = virt_term_start[index];
		}
	}
	// Write in stat bar at top of pysical term
	char *str = " GrainX Ver0.1! term:     ";
	char buff[15];
	u32_to_str((uint32_t) virt_term_start_line, buff, 14);
	for (uint8_t i = 0; i < 3; i++)
		str[22 + i] = buff[0 + i];
	size_t str_len = strlen(str) + 1;
	uint8_t offset = VGA_WIDTH - (str_len + 1);
	term_buff[offset - 1] = vga_entry('*', VGA_COLOR_GREEN);
	term_buff[offset + str_len] = vga_entry('*', VGA_COLOR_GREEN);
	for (uint8_t i = 0; i < str_len; i++)
		term_buff[offset + i] = vga_entry(str[i], (VGA_COLOR_BLACK + i) % 7 + 1);
	
}

void vga_term_putc_at(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VIRT_WIDTH + x;
	virtual_term_buff[index] = vga_entry(c, color);
}

void scroll_virtual_term()
{
	// Move every line up one
	for (size_t y = 0; y < VIRT_HEIGHT - 1; y++) {
		memcpy(&virtual_term_buff[y * VGA_WIDTH], &virtual_term_buff[(y + 1) * VGA_WIDTH], sizeof(uint16_t) * VGA_WIDTH);
	}
	// Clear out last line
	for (size_t x = 0; x < VIRT_WIDTH; x++) {
		virtual_term_buff[(VIRT_HEIGHT - 1) * VIRT_WIDTH + x] = vga_entry(' ', term_color);	
	}
}

void vga_term_pos(size_t val)
{
	size_t i = virt_term_start_line + val;
	if (val == 0) 
		virt_term_start_line = VIRT_HEIGHT - VGA_HEIGHT;
	else if (i < (VIRT_HEIGHT - VGA_HEIGHT) + 1 && i > 0)
		virt_term_start_line += val;
	else
		vga_term_pos(0);
	vga_term_write_pysical();
}

void vga_term_putc(char c)
{
	switch (c)
	{
		case '\n':
			// If term is scrolled up scroll back down
			if (virt_term_start_line != VIRT_HEIGHT - VGA_HEIGHT)
				vga_term_pos(0);
			t_row++;
			t_col = 0;
			if (t_row == VIRT_HEIGHT) {
				t_col = 0;
				t_row -= 1;
				scroll_virtual_term();
			}
			return;
		case '\r':
			t_col = 0;
			return;
		case '\b':
			if (t_col)
				t_col--;
			return;
	}

	vga_term_putc_at(c, term_color, t_col, t_row);

	if (++t_col == VIRT_WIDTH) {
		t_col = 0;
		if (++t_row == VIRT_HEIGHT) {
			t_row -= 1;
			scroll_virtual_term();
		}
	}
}


/* 
* update cursor 
* 0x3D4 is an assumed x86 IO port for vga 
* currsor control and might fail on some pc's
*/

void vga_term_update_curse_pos()
{
	size_t pysical_t_row = t_row - virt_term_start_line;
	unsigned short pos = pysical_t_row * VGA_WIDTH + t_col;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

void vga_term_write(const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		vga_term_putc(data[i]);
}

void vga_term_update(void)
{
	vga_term_update_curse_pos();
	vga_term_write_pysical();
}

void vga_term_write_str(const char *data)
{
	vga_term_write(data, strlen(data));
	vga_term_update_curse_pos();
	vga_term_write_pysical();
}
