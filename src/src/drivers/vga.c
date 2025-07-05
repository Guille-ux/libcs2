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
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5 
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_AC_INDEX        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_INSTAT_READ     0x3DA 

uint8_t g_misc_output_reg_mode03 = 0x00; 
uint8_t g_seq_regs_mode03[5];
uint8_t G_SEQ_REG_COUNT_MODE03 = 5;
uint8_t g_crtc_regs_mode03[25]; 
uint8_t G_CRTC_REG_COUNT_MODE03 = 25;
uint8_t g_gfx_regs_mode03[9]; 
uint8_t G_GFX_REG_COUNT_MODE03 = 9;
uint8_t g_attr_regs_mode03[21]; 
uint8_t G_ATTR_REG_COUNT_MODE03 = 21;


void vga_start() {
    uint16_t i;
    uint8_t temp_cr11;     
    vga_disable();

    
    outb(VGA_MISC_WRITE, g_misc_output_reg_mode03);
    io_wait();

    
    outb(VGA_SEQ_INDEX, 0x00); io_wait();
    outb(VGA_SEQ_DATA, 0x01); io_wait();

    
    for (i = 0; i < G_SEQ_REG_COUNT_MODE03; i++) {
        outb(VGA_SEQ_INDEX, i); io_wait();
        outb(VGA_SEQ_DATA, g_seq_regs_mode03[i]); io_wait();
    }
    
    outb(VGA_SEQ_INDEX, 0x00); io_wait();
    outb(VGA_SEQ_DATA, 0x03); io_wait();
    io_wait();

    outb(VGA_CRTC_INDEX, 0x11); io_wait();
    temp_cr11 = inb(VGA_CRTC_DATA); io_wait(); 
    outb(VGA_CRTC_DATA, temp_cr11 & 0x7F); io_wait(); 

    for (i = 0; i < G_CRTC_REG_COUNT_MODE03; i++) {
        outb(VGA_CRTC_INDEX, i); io_wait();
        outb(VGA_CRTC_DATA, g_crtc_regs_mode03[i]); io_wait();
    }
    
    outb(VGA_CRTC_INDEX, 0x11); io_wait();
    outb(VGA_CRTC_DATA, temp_cr11); io_wait();
    io_wait(); 

    
    for (i = 0; i < G_GFX_REG_COUNT_MODE03; i++) {
        outb(VGA_GC_INDEX, i); io_wait();
        outb(VGA_GC_DATA, g_gfx_regs_mode03[i]); io_wait();
    }
    io_wait(); 

    
    for (i = 0; i < G_ATTR_REG_COUNT_MODE03; i++) {
        inb(VGA_INSTAT_READ); io_wait(); 
        outb(VGA_AC_INDEX, i); io_wait(); 
        outb(VGA_AC_INDEX, g_attr_regs_mode03[i]); io_wait(); 
    }
    
    inb(VGA_INSTAT_READ); io_wait(); 
    outb(VGA_AC_INDEX, 0x20); io_wait(); 
    io_wait(); 

    
    vga_enable();

    
    vga_clear(stdcolor);
    vga_setcur(0, 0);
    
    vga_print_string(0, 0, "VGA : [ ", 0x0F);
    vga_print_string(8, 0, "OK", 0x02);
    vga_print_string(11, 0, "]", 0x0F);

}

void vga_clear(MultiColor color) {
	char chr=' ';
	for (uint16_t x=0;x<width;x++) {
		for (uint16_t y=0;y<height;y++) {
			vga_putchar(x, y, chr, color.as.text_mode);
		}
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
