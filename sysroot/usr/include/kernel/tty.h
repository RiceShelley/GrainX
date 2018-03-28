#ifndef __TTY__H
#define __TTY__H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/ps2.h>
#include <kernel/vga_console.h>
#include <string.h>

const char *tty_get_user_input(char *buff, size_t buff_len);

#endif
