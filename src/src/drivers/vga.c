#include "../../headers/vga.h"
#include "../../include/display.h"
#include "../../include/vga.h"
#include "../../include/io.h"
#include "../../include/string.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

uint16_t height=25;
uint16_t width=80;
uint16_t global_x=0;
uint16_t global_y=0;
uint8_t current_mode=0x03;


uint8_t g_misc_output_reg_mode03 = 0x67; // 0x67 = color, 25MHz clock, neg HSYNC, neg VSYNC, IOAS=0x3D4

//  SR01 a SR04
uint8_t g_seq_regs_mode03[] = {
    0x01, // SR01: Clocking Mode (8 dots/character, screen off)
    0x03, // SR02: Map Mask (all planes enabled)
    0x00, // SR03: Character Map Select (default font)
    0x02  // SR04: Memory Mode (text mode, sequential addressing)
};
uint8_t G_SEQ_REG_COUNT_MODE03 = sizeof(g_seq_regs_mode03);


//  CRT (CRTC)
uint8_t g_crtc_regs_mode03[] = {
    0x5F, // CR00: Horizontal Total
    0x4F, // CR01: End Horizontal Display
    0x50, // CR02: Start Horizontal Blanking
    0x82, // CR03: End Horizontal Blanking (bit 7 for CR00-07)
    0x54, // CR04: Start Horizontal Retrace
    0x80, // CR05: End Horizontal Retrace
    0x0B, // CR06: Vertical Total
    0x3E, // CR07: Overflow (for vertical regs)
    0x00, // CR08: Preset Row Scan
    0x00, // CR09: Maximum Scan Line (16 scanlines per character row)
    0x00, // CR0A: Cursor Start
    0x0F, // CR0B: Cursor End
    0x00, // CR0C: Start Address High
    0x00, // CR0D: Start Address Low
    0x00, // CR0E: Cursor Location High
    0x00, // CR0F: Cursor Location Low
    0x9C, // CR10: Vertical Retrace Start
    0x8E, // CR11: Vertical Retrace End (bit 7 for unlock/lock)
    0x8F, // CR12: Vertical Display End
    0x28, // CR13: Offset (memory offset between rows)
    0x1F, // CR14: Underline Location
    0x96, // CR15: Start Vertical Blanking
    0xB9, // CR16: End Vertical Blanking
    0xE3  // CR17: Mode Control (word mode, byte mode, etc.)
};
uint8_t G_CRTC_REG_COUNT_MODE03 = sizeof(g_crtc_regs_mode03);


// (Graphics Controller)
uint8_t g_gfx_regs_mode03[] = {
    0x00, // GR00: Set/Reset
    0x00, // GR01: Enable Set/Reset
    0x00, // GR02: Color Compare
    0x00, // GR03: Data Rotate
    0x00, // GR04: Read Map Select (reads from plane 0 by default)
    0x00, // GR05: Mode Register (alpha-numeric mode, even though it's 0x03)
    0x0E, // GR06: Miscellaneous Register (map 0xA0000-0xBFFFF, A16-A17 disable, etc.)
    0x00, // GR07: Color Don't Care
    0xFF  // GR08: Bit Mask
};
uint8_t G_GFX_REG_COUNT_MODE03 = sizeof(g_gfx_regs_mode03);


uint8_t g_attr_regs_mode03[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, // Paleta de colores 0-7
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // Paleta de colores 8-15
    0x01, // AC10: Mode Control (text mode, attribute enable)
    0x00, // AC11: Overscan Color (border color)
    0x0F, // AC12: Color Plane Enable (enable all 4 planes)
    0x00, // AC13: Horizontal Pixel Panning
    0x00  // AC14: Color Select
};
uint8_t G_ATTR_REG_COUNT_MODE03 = sizeof(g_attr_regs_mode03);

void vga_setmode(uint8_t mode) {
	uint16_t i;
	uint8_t temp;
	switch (mode) {
		case (0x03): {
			outb(VGA_MISC_WRITE, g_misc_output_reg_mode03); io_wait();

            		outb(VGA_SEQ_INDEX_PORT, 0x00); io_wait();
            		outb(VGA_SEQ_READ_WRITE_INDEX, 0x01); io_wait(); 

            		for (i = 0; i < G_SEQ_REG_COUNT_MODE03; i++) {
                		outb(VGA_SEQ_INDEX_PORT, i + 1); io_wait(); 
                		outb(VGA_SEQ_READ_WRITE_INDEX, g_seq_regs_mode03[i]); io_wait();
            		}

            		outb(VGA_SEQ_INDEX_PORT, 0x00); io_wait();
            		outb(VGA_SEQ_READ_WRITE_INDEX, 0x03); io_wait(); 
            		outb(VGA_CRTC_INDEX_PORT + VGA_CRTC_INDEX_BASE_A, 0x11); io_wait();
            		temp = inb(VGA_CRTC_READ_WRITE + VGA_CRTC_INDEX_BASE_A); io_wait();
            		outb(VGA_CRTC_READ_WRITE+VGA_CRTC_INDEX_BASE_A, temp & 0x7F); io_wait(); 	      		      
			for (i = 0; i < G_CRTC_REG_COUNT_MODE03; i++) {
		                outb(VGA_CRTC_INDEX_PORT+VGA_CRTC_INDEX_BASE_A, i); io_wait();
		                outb(VGA_CRTC_READ_WRITE+VGA_CRTC_INDEX_BASE_A, g_crtc_regs_mode03[i]); io_wait();
            		}

		        for (i = 0; i < G_GFX_REG_COUNT_MODE03; i++) {
                		outb(VGA_GCR_INDEX_PORT, i); io_wait();
                		outb(VGA_GCR_READ_WRITE_INDEX, g_gfx_regs_mode03[i]); io_wait();
            		}
            		inb(VGA_ACR_RESET); 
            		io_wait(); 
            		for (i = 0; i < G_ATTR_REG_COUNT_MODE03; i++) {
                		outb(VGA_ACR_INDEX_WRITE_PORT, i); io_wait();      
                		outb(VGA_ACR_INDEX_WRITE_PORT, g_attr_regs_mode03[i]); io_wait();              
            		}

            		outb(VGA_ACR_INDEX_WRITE_PORT, 0x20); io_wait();
            
                       	vga_clear(stdcolor); 
            		vga_setcur(0, 0);
            		return;
			     }
		default: return;
	}
}

void vga_clear(MultiColor color) {
	if (current_mode==0x03) {
		char chr=' ';
		for (uint16_t x=0;x<width;x++) {
			for (uint16_t y=0;y<height;y++) {
				vga_putchar(x, y, chr, color.as.text_mode);
			}
		}
	} else {
		// for the future
	}
}

void vga_putchar(uint16_t x, uint16_t y, char c, uint8_t attr) {
	if (x > width || y > height) return;
	vga_char *vga_ptr = (vga_char*)0xB8000 + x + y*width;

	vga_char chr;
	chr.character = c;
	chr.attr = attr;
	*vga_ptr = chr;
}

void vga_print_string(uint16_t x, uint16_t y, const char *str, uint8_t attr) {
	global_x = x;
	global_y = y;

	for (size_t i=0;i<strlen(str);i++) {
		char c=str[i];
		if (c=='\n') {
			global_x=0;
			global_y++;
		} else if (c=='\r') {
			global_x=0;
		} else {
			vga_putchar(global_x, global_y, c, attr);
			global_x++;
		}

		if (global_x>=width) {
			global_x=0;
			global_y++;
		}
		if (global_y>=height) {
			vga_scroll();
			global_y=height-1;
			global_x=0;
		}
	}
	vga_setcur(global_x, global_y);
}

void vga_setcur(uint16_t x, uint16_t y) {
	if (x >= width) x = width - 1;
	if (y >= height) y = height - 1;

	uint16_t pos = x + y*width;

	outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_INDEX_PORT, 0x0E); // read headers/vga.h
	outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_READ_WRITE, (uint8_t)((pos >> 8) & 0xFF)); // for little endian
	outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_INDEX_PORT, 0x0F); // read headers/vga.h
	outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_READ_WRITE, (uint8_t)(pos & 0xFF));
}

void vga_putpix(uint16_t x, uint16_t y, rgb color) {
	// anything actually
}

void vga_scroll() {
	vga_char vacio = {' ', stdcolor.as.text_mode};
	memcpy((void *)VGA_TEXT_MODE_BEGIN, (const void *)(VGA_TEXT_MODE_BEGIN + width), sizeof(vga_char)*width*(height-1));
	vga_fill_line(height-1, vacio);

	global_x = 0;
	global_y = height-1;
	vga_setcur(global_x, global_y);
}

void vga_init(uint8_t mode) {
	vga_setmode(mode);	// actualmente solo necesito esto ¿no?
}

void vga_fill_line(uint16_t line, vga_char character) {
	if (current_mode==0x03) {
		for (uint16_t i=0;i<width;i++) {
			vga_putchar(i, line, character.character, character.attr);
		}
	} else {
		// for the future
	}
}

