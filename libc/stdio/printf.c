#include <stdio.h>

int printf(const char* restrict format, ...) 
{
    va_list param;
    va_start(param, format);

    int written = 0;
    
    while (*format != '\0') {

        if (format[0] != '%' || format[1] == '%') {

            if (format[0] == '%')
                format++;

            size_t amt = 1;
            while (format[amt] && format[amt] != '%')
                amt++;

            vga_term_write(format, amt);

            format += amt;
            written += amt;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c') {
            format++;
            char c = (char) va_arg(param, int);
            vga_term_write((const char *) &c, sizeof(c));
            written++;
        } else if (*format == 's') {
            format++;
            const char* str = va_arg(param, const char*);
            size_t len = strlen(str);
            vga_term_write(str, len);
            written += len;
        } else if (*format == 'd') {
            format++;
            uint32_t num = (uint32_t) va_arg(param, uint32_t);
            char buff[15];
            u32_to_str(num, buff, 14);
            size_t len = strlen(buff);
            vga_term_write(buff, len);
            written += len;
        } else if (*format == 'x') {
			format++;
			uint32_t num = (uint32_t) va_arg(param, uint32_t);
			size_t buff_len = 20;
			char buff[buff_len];
			size_t i = 0;
			// convert to hex
			while (num > .9 && i < buff_len) {
				uint32_t r = num % 16;
				char c = 0;
				if (r < 10)
					c = r + '0';
				else
					c = (r - 10) + 'A';
				buff[i] = c;
				i++;
				num = num / 16;
			}
			// How to handle when function is passed 0 
			if (i == 0) {
				buff[i] = '0';
				i++;
			}
			buff[i] = 0;
			str_reverse(buff, i);
			size_t real_len = strlen(buff);
			vga_term_write(buff, real_len);
			written += real_len;
		} else {
            format = format_begun_at;
            size_t len = strlen(format);
            vga_term_write(format, len);
            written += len;
            format += len;
        }
    }
    va_end(param);
    vga_term_update();
    return written;
}
