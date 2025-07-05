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


#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_CRTC_INDEX      0x3D4 // O tu macro VGA_CRTC_INDEX_PORT + VGA_CRTC_INDEX_BASE_A
#define VGA_CRTC_DATA       0x3D5 // O tu macro VGA_CRTC_READ_WRITE + VGA_CRTC_INDEX_BASE_A
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_AC_INDEX        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_INSTAT_READ     0x3DA // O tu macro VGA_ACR_RESET



uint8_t g_misc_output_reg_mode03 = 0x67; // 0x67 = color, 25MHz clock, neg HSYNC, neg VSYNC, IOAS=0x3D4

//  SR01 a SR04
uint8_t g_seq_regs_mode03[] = {
    0x01, // SR01: Clocking Mode (8 dots/character, screen off)
    0x03, // SR02: Map Mask (all planes enabled)
    0x00, // SR03: Character Map Select (default font)
    0x06  // SR04: Memory Mode (text mode, sequential addressing)
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

void vga_start() { 
    uint16_t i;
    uint8_t temp;

    
    outb(VGA_MISC_WRITE, g_misc_output_reg_mode03); io_wait();
    io_wait();

    
    outb(VGA_SEQ_INDEX_PORT, 0x00); io_wait(); 
    outb(VGA_SEQ_READ_WRITE_INDEX, 0x01); io_wait();
    for (i = 0; i < G_SEQ_REG_COUNT_MODE03; i++) {
       
        outb(VGA_SEQ_INDEX_PORT, i + 1); io_wait();
        outb(VGA_SEQ_READ_WRITE_INDEX, g_seq_regs_mode03[i]); io_wait();
    }

    outb(VGA_SEQ_INDEX_PORT, 0x00); io_wait();
    outb(VGA_SEQ_READ_WRITE_INDEX, 0x03); io_wait(); 

    io_wait(); 
    io_wait();
    io_wait();

    
    outb(VGA_CRTC_INDEX_PORT + VGA_CRTC_INDEX_BASE_A, 0x11); io_wait();
    temp = inb(VGA_CRTC_READ_WRITE + VGA_CRTC_INDEX_BASE_A); io_wait();
    outb(VGA_CRTC_READ_WRITE + VGA_CRTC_INDEX_BASE_A, temp & 0x7F); io_wait(); 

    for (i = 0; i < G_CRTC_REG_COUNT_MODE03; i++) { 
        outb(VGA_CRTC_INDEX_PORT + VGA_CRTC_INDEX_BASE_A, i); io_wait();
        outb(VGA_CRTC_READ_WRITE + VGA_CRTC_INDEX_BASE_A, g_crtc_regs_mode03[i]); io_wait();
    }

    outb(VGA_CRTC_INDEX_PORT + VGA_CRTC_INDEX_BASE_A, 0x11); io_wait();
    outb(VGA_CRTC_READ_WRITE + VGA_CRTC_INDEX_BASE_A, temp); io_wait();  
    io_wait();
    io_wait();

    
    for (i = 0; i < G_GFX_REG_COUNT_MODE03; i++) {
        outb(VGA_GCR_INDEX_PORT, i); io_wait();
        outb(VGA_GCR_READ_WRITE_INDEX, g_gfx_regs_mode03[i]); io_wait();
    }

    io_wait(); 
    io_wait();

    
    inb(VGA_ACR_RESET); 
    io_wait();

    for (i = 0; i < G_ATTR_REG_COUNT_MODE03; i++) {
        outb(VGA_ACR_INDEX_WRITE_PORT, i | 0x20); io_wait(); 
        outb(VGA_ACR_INDEX_WRITE_PORT, g_attr_regs_mode03[i]); io_wait(); 
    }

    inb(VGA_ACR_RESET); 
    io_wait();
    outb(VGA_ACR_INDEX_WRITE_PORT, 0x20); io_wait();
   
    io_wait();
    io_wait();

    
    vga_clear(stdcolor);
    vga_setcur(0, 0);
    vga_print_string(0, 0, "VGA : [ OK ]", 0x0F);
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
	volatile vga_char *vga_ptr = (vga_char*)0xB8000 + x + y*width;

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
void vga_scroll() {
	vga_char vacio = {' ', stdcolor.as.text_mode};
	memcpy((void *)VGA_TEXT_MODE_BEGIN, (const void *)(VGA_TEXT_MODE_BEGIN + width), sizeof(vga_char)*width*(height-1));
	vga_fill_line(height-1, vacio);

	global_x = 0;
	global_y = height-1;
	vga_setcur(global_x, global_y);
}

void vga_init() {
	dump_vga_registers_minimal_read_only();
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
/*
 void dump_vga_registers() {
    uint16_t i;
    char hex_str[3];
    char line_buffer[80];
    uint16_t current_y = 0;

    
    void print_and_advance(const char* prefix, uint8_t value) {
        byte_to_hex_string(value, hex_str);
        strcpy(line_buffer, prefix);
        strcat(line_buffer, hex_str);
        
        vga_print_string(0, current_y, line_buffer, VGA_COLOR_LIGHT_GREY | (VGA_COLOR_BLACK << 4));
        current_y++;
        if (current_y >= height) {
            vga_scroll(); 
            current_y = height -1; 
        }
    }

    
    uint8_t val = inb(VGA_MISC_READ); 
    print_and_advance("MISC_OUTPUT: ", val);

    
    print_and_advance("--- SECUENCIADOR ---", 0); 
    for (i = 0; i < 5; i++) { 
        outb(VGA_SEQ_INDEX, i); io_wait();
        val = inb(VGA_SEQ_DATA); io_wait();
        char prefix_buf[8]; 
        strcpy(prefix_buf, "SR");
        byte_to_hex_string(i, hex_str);
        strcat(prefix_buf, hex_str);
        strcat(prefix_buf, ": ");
        print_and_advance(prefix_buf, val);
    }

    
    print_and_advance("--- CRTC ---", 0); // Encabezado
   
    outb(VGA_CRTC_INDEX, 0x11); io_wait();
    uint8_t cr11_val = inb(VGA_CRTC_DATA); io_wait();
    outb(VGA_CRTC_DATA, cr11_val & 0x7F); io_wait();
    for (i = 0; i < 25; i++) { 
									
        outb(VGA_CRTC_INDEX, i); io_wait();
        val = inb(VGA_CRTC_DATA); io_wait();
        char prefix_buf[8]; 
        strcpy(prefix_buf, "CR");
        byte_to_hex_string(i, hex_str);
        strcat(prefix_buf, hex_str);
        strcat(prefix_buf, ": ");
        print_and_advance(prefix_buf, val);
    }
    
    outb(VGA_CRTC_INDEX, 0x11); io_wait();
    outb(VGA_CRTC_DATA, cr11_val); io_wait(); 

    
    print_and_advance("--- GRAFICOS ---", 0); 
    for (i = 0; i < 9; i++) { 
        outb(VGA_GC_INDEX, i); io_wait();
        val = inb(VGA_GC_DATA); io_wait();
        char prefix_buf[8]; 
        strcpy(prefix_buf, "GR");
        byte_to_hex_string(i, hex_str);
        strcat(prefix_buf, hex_str);
        strcat(prefix_buf, ": ");
        print_and_advance(prefix_buf, val);
    }

    
    print_and_advance("--- ATRIBUTOS ---", 0); 
    for (i = 0; i < 21; i++) { 
        inb(VGA_INSTAT_READ); io_wait(); 
        outb(VGA_AC_INDEX, i); io_wait(); 
        val = inb(VGA_AC_READ); io_wait(); 
        char prefix_buf[8]; 
        strcpy(prefix_buf, "AC");
        byte_to_hex_string(i, hex_str);
        strcat(prefix_buf, hex_str);
        strcat(prefix_buf, ": ");
        print_and_advance(prefix_buf, val);
    }
    
    inb(VGA_INSTAT_READ); io_wait(); 
    outb(VGA_AC_INDEX, 0x20); io_wait(); 

    
}
 */
void dump_vga_registers_minimal_read_only() {
    uint16_t i;
    
    volatile uint8_t val;

   
    
    g_misc_output_reg_mode03 = inb(VGA_MISC_READ);


    for (i = 0; i < 5; i++) { 
        outb(VGA_SEQ_INDEX, i); io_wait();
        g_seq_regs_mode03[i] = inb(VGA_SEQ_DATA); io_wait();
    }


    outb(VGA_CRTC_INDEX, 0x11); io_wait();
    uint8_t cr11_val = inb(VGA_CRTC_DATA); io_wait();
    outb(VGA_CRTC_DATA, cr11_val & 0x7F); io_wait(); 

    for (i = 0; i < 25; i++) { 
        outb(VGA_CRTC_INDEX, i); io_wait();
        g_crtc_regs_mode03[i] = inb(VGA_CRTC_DATA); io_wait();
    }
   
    outb(VGA_CRTC_INDEX, 0x11); io_wait();
    outb(VGA_CRTC_DATA, cr11_val); io_wait();

    
    for (i = 0; i < 9; i++) { 
        outb(VGA_GC_INDEX, i); io_wait();
        g_gfx_regs_mode03[i] = inb(VGA_GC_DATA); io_wait();
    }

    
    for (i = 0; i < 21; i++) { 
        inb(VGA_INSTAT_READ); io_wait(); 
        outb(VGA_AC_INDEX, i); io_wait(); 
        g_attr_regs_mode03[i] = inb(VGA_AC_READ); io_wait(); 
    }

    inb(VGA_INSTAT_READ); io_wait();
    outb(VGA_AC_INDEX, 0x20); io_wait();

}

void vga_disable() {
	volatile uint8_t tmp;

	outb(VGA_SEQ_INDEX_PORT, 0x01); // select clocking mode register
	
	tmp = inb(VGA_SEQ_READ_WRITE_INDEX);

	outb(VGA_SEQ_READ_WRITE_INDEX, tmp | 0x20);
}

void vga_enable() {
	volatile uint8_t tmp;

	outb(VGA_SEQ_INDEX_PORT, 0x01);

	tmp=inb(VGA_SEQ_READ_WRITE_INDEX);
	
	outb(VGA_SEQ_READ_WRITE_INDEX, tmp & ~0x20);
}
