#include <kernel/ps2.h>

static const char *scancode[] = {"<esc>", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                "-", "=", "<backspace>", "<tab>", "q", "w", "e", "r", "t", "y",
                "u", "i", "o", "p", "[", "]", "<enter>", "<left_ctrl>", "a", "s",
                "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "<left_shift>",
                "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "<right_shift>",
                "(keypad *)", "<left_alt>", " ", "<capslock>", "<F1>", "<F2>", "<F3>",
                "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<numlock>",
                "<scroll_lock>", "(keypad 7)", "(keypad 8)", "(keypad 9)", "(keypad -)",
                "(keypad 4)", "(keypad 5)", "(keypad 6)", "(keypad +)", "(keypad 1)",
                "(keypad 2)", "(keypad 3)", "(keypad 0)", "(keypad .)", "<F11>", "<F12>"};

static bool shift_down = false;

unsigned char get_scancode()
{
	unsigned char c = 0;
	do {
		if (inb(0x60) != c) {
			c = inb(0x60);
			// test for shift
			if (c == 0x36 || c == 0x2A)
				shift_down = true;
			if (c == 0xB6 || c == 0xAA)
				shift_down = false;
			if (c > 0)
				return c;
		}
	} while (1);
}

bool is_scancode_pressed(unsigned char c) {
	return ((c & 128) != 128);
}

const char *scancode_to_str(unsigned char c) 
{
	return (const char *) scancode[c - 1];
}

bool is_shift_down()
{
	return shift_down;
}

const char *ps2_key_in()
{
        // user should only be able to hold down 128 keys at a time
        static unsigned char keys_down[128];
	static bool keys_down_init = false;

	if (!keys_down_init) {
		for (int i = 0; i < 128; i++)
			keys_down[i] = 0;
		keys_down_init = true;
	}

        while (true) { 
		int foo = 0;
                unsigned char c = get_scancode();
                if (is_scancode_pressed(c)) {
			for (int i = 0; i < 128; i++) {
				if (keys_down[i] == c) {
					foo = -1;
					break;
				}
			}
	
			if (foo == -1) {
				continue;
			}

			for (int i = 0; i < 128; i++) {
				if (keys_down[i] == 0) { 
					keys_down[i] = c;
					break;
				}
			}
               		return scancode_to_str(c);
                } else {
                        for (int i = 0; i < 128; i++) {
                                if (keys_down[i] == (c - 0x80)) 
                                        keys_down[i] = 0;
                        }
                }
        }

}
