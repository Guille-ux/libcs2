#include "../../headers/vga.h"
#include "../../include/display.h"
#include "../../include/vga.h"
#include "../../include/io.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

uint16_t heigh=25;
uint16_t width=80;
uint8_t current_mode=0x03;

uint8_t vga_seq_regs_mode_03[] = {
    0x03, // SR01: Clocking Mode Register
    0x01, // SR02: Map Mask Register
    0x0F, // SR03: Character Map Select
    0x00, // SR04: Memory Mode Register
    0x0E  // SR05: Address Mode
};
uint8_t VGA_SEQ_REG_COUNT_MODE_03 = sizeof(vga_seq_regs_mode_03);

uint8_t vga_crtc_regs_mode_03[] = {
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, 
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00, 
    0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, 
    0xFF                                            
};
uint8_t VGA_CRTC_REG_COUNT_MODE_03 = sizeof(vga_crtc_regs_mode_03);

uint8_t vga_gfx_regs_mode_03[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F,
    0xFF                                           
};
uint8_t VGA_GFX_REG_COUNT_MODE_03 = sizeof(vga_gfx_regs_mode_03);

uint8_t vga_attr_regs_mode_03[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x01, 0x00, 0x0F, 0x00, 0x00                   
};
uint8_t VGA_ATTR_REG_COUNT_MODE_03 = sizeof(vga_attr_regs_mode_03);


uint8_t vga_misc_out_reg_mode_03 = 0xE3;

void vga_setmode(uint8_t mode) {
	uint16_t i;
	uint8_t temp;
	switch (mode) {
		case (0x03): {
			temp = inb(VGA_MISC_READ); // read the current state
			outb(VGA_MISC_WRITE, temp & ~0xCF); // 0xCF = 11001111b, ~0xCF = 00110000
			
			outb(VGA_SEQ_INDEX_PORT, 0x00); // select a register from sequencer
			outb(VGA_SEQ_READ_WRITE_INDEX, 0x01);



			outb(VGA_MISC_WRITE, vga_misc_out_reg_mode_03); // for mode 0x03
			for (i=0;i<VGA_SEQ_REG_COUNT_MODE_03;i++) {
				outb(VGA_SEQ_INDEX_PORT, i+1);
				outb(VGA_SEQ_READ_WRITE_INDEX, vga_seq_regs_mode_03[i]);
			}
			
			outb(VGA_SEQ_INDEX_PORT, 0x00);
			outb(VGA_SEQ_READ_WRITE_INDEX, 0x03);


			outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_INDEX_PORT, 0x11);
			temp = inb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_READ_WRITE);
			outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_READ_WRITE, temp & 0x7F);

			for (i=0;i<VGA_CRTC_REG_COUNT_MODE_03;i++) {
				outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_INDEX_PORT, i);
				outb(VGA_CRTC_INDEX_BASE_A + VGA_CRTC_READ_WRITE, vga_crtc_regs_mode_03[i]);
			}

			for (i=0;i<VGA_GFX_REG_COUNT_MODE_03;i++) {
				outb(VGA_GCR_INDEX_PORT, i);
				outb(VGA_GCR_READ_WRITE_INDEX, vga_gfx_regs_mode_03[i]);
			}

			inb(VGA_ACR_RESET);

			for (i=0;i<VGA_ATTR_REG_COUNT_MODE_03;i++) {
				outb(VGA_ACR_INDEX_WRITE_PORT, i);
				outb(VGA_ACR_INDEX_WRITE_PORT, vga_attr_regs_mode_03[i]);
				inb(VGA_ACR_RESET);
			}

			outb(VGA_ACR_INDEX_WRITE_PORT, 0x20); // enable video, idk
			outb(VGA_MISC_WRITE, vga_misc_out_reg_mode_03);

			     }
		default: return;
	}
}

void vga_clear() {
	if (current_mode==0x03) {
		char chr=' ';
		for (uint16_t x=0;x<width;x++) {
			for (uint16_t y;y<heigh;y++) {
				vga_putchar(x, y, chr, 0);
			}
		}
	} else {
		// for the future
	}
}

void vga_putchar(uint16_t x, uint16_t y, char c, uint8_t attr) {
	if (x<0 || x > width || y < 0 || y > heigh) return;
	vga_char *vga_ptr = (vga_char*)0xB8000 + x + y*width;

	vga_char chr;
	chr.character = c;
	chr.attr = attr;
	*vga_ptr = chr;
}

void vga_print_string(uint16_t x, uint16_t y, const char *str, uint8_t attr) {

}

void vga_setcur(uint16_t x, uint16_t y) {

}

void vga_putpix(uint16_t x, uint16_t y, rgb color) {

}


void vga_init(uint8_t mode) {
	vga_setmode(mode);	// actualmente solo necesito esto ¿no?
}

