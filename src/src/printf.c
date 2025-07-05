#include "../include/printf.h"
#include "../include/stdio.h"
#include "../include/display.h"
#include "../include/vga.h"
#include "../include/string.h"
#include "../include/stdbuff.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Auxiliary Functions

static void uint_to_hex_string(uintptr_t value, char *buffer) {
	const char hex_chars[]="0123456789ABCDEF";
	int i=0;

	if (value == 0) {
		buffer[0]='0';
		buffer[1]='\0';
		return;
	}
	char tmp_buffer[sizeof(uintptr_t)*2+1];
	int k=0;

	while (value > 0) {
		tmp_buffer[k++] = hex_chars[value % 16];
		value /= 16;
	}

	while (k > 0) {
		buffer[i++]=tmp_buffer[k--];
	}
	buffer[i]='\0';
}

static void int_to_dec_string(int value, char *buffer) {
	int i=0;
	if (value==0) {
		buffer[0]='0';
		buffer[1]='\0';
		return;
	}
	bool negative=false;
	if (value < 0) {
		negative=true;
		value = -value;
	}
	
	char tmp_buffer[24];

	int k=0;

	while (value > 0) {
		tmp_buffer[k++] = (value % 10) + '0';
		value /=  10;
	}
	if (negative) tmp_buffer[k++]='-';
	
	while (k > 0) {
		buffer[i++]=tmp_buffer[k--]
	}

	buffer[i]='\0';
}

static void kprintf_putc(char c) {
	if (!stdout_interface.active) return;

	switch (c) {
		case '\n': {
			size_t current_y = stdout_interface.get_cur_y();
			size_t next_y = current_y + 1;

			size_t max_y = stdout_interface.get_max_y();
			if (next_y > max_y) {
				stdout_interface.scroll();
				next_y = max_y - 1;
			}
			stdout_interface.setcur(0, next_y);
			return;
		}
		case '\r': {
			stdout_interface.setcur(0, stdout_interface.get_cur_y());
			return;
			   }
		case '\t': {
			size_t current_x=stdout_interface.get_cur_x();
			size_t next_x=8-(current_x%8);
			for (char i=0;i<next_x;i++) {
				kprintf_putc(' ');
			}
			return;
			   }
		default: {
			//i need to make this	
			 }
	}
}
