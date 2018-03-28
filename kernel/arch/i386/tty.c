#include <kernel/tty.h>

const char *tty_get_user_input(char *buff, size_t buff_len)
{
	for (size_t i = 0; i < (buff_len - 1); i++) {
		const char *key = ps2_key_in();

		if ((char) key[0] != '<' && (char) key[0] != '(') {
			char c = key[0];
			if (is_shift_down()) {
				switch (c) {
				case '1':
					c = '!';
					break;
				case '2':
					c = '@';
					break;
				case '3':
					c = '#';
					break;
				case '4':
					c = '$';
					break;
				case '5':
					c = '%';
					break;
				case '6':
					c = '^';
					break;
				case '7':
					c = '&';
					break;
				case '8':
					c = '*';
					break;
				case '9':
					c = '(';
					break;
				case '0':
					c = ')';
					break;
				case '-':
					c = '_';
					break;
				case '=':
					c = '+';
					break;
				case '`':
					c = '~';
					break;
				case '[':
					c = '{';
					break;
				case ']':
					c = '}';
					break;
				case '\\':
					c = '|';
					break;
				case ';':
					c = ':';
					break;
				case '\'':
					c = '"';
					break;
				case ',':
					c = '<';
					break;
				case '.':
					c = '>';
					break;
				case '/':
					c = '?';
					break;
				default:
					if (c >= 'a' && c <= 'z')
						c -= 'a' - 'A';
				}
			}
			char m[] = {c, '\0'};
			vga_term_write_str((const char *) m);
			buff[i] = c;
		} else if (memcmp(key, "<enter>", strlen("<enter>"))) {
			buff[i] = '\0';
			return buff;
		} else if (memcmp(key, "<backspace>", strlen("<backspace>"))) {
			if (i == 0) {
				i--;
				continue;
			}
			i -= 2;
			vga_term_write_str("\b \b");
		// scroll term up
		} else if (memcmp(key, "<F2>", strlen("<F2>"))) {
			vga_term_pos(-2);
			i--;
		// scroll term down
		} else if (memcmp(key, "<F3>", strlen("<F3>"))) {
			vga_term_pos(2);	
			i--;
		} else {
			i--;
		}
	}
	buff[buff_len - 1] = '\0';
	return buff;
}
