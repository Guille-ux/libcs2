#include "../include/keymaps.h"
#include "../include/io.h"
#include "../include/keyboard.h"
#include "../include/printf.h"
// atención, solo para teclados integrados



const uint8_t _kbd_us_normal_map[256] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, // 0x00-0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's', // 0x10-0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', // 0x20-0x2F
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, // 0x30-0x3F
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40-0x4F (NumPad)
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x50-0x5F
    
};


const uint8_t _kbd_us_shift_map[256] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0, // 0x00-0x0F
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0, 'A', 'S', // 0x10-0x1F
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V', // 0x20-0x2F
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, // 0x30-0x3F
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', // 0x40-0x4F (NumPad)
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x50-0x5F
    // ...
};


const uint8_t _kbd_us_caps_map[256] = {
    
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0, 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    
};


const uint8_t _kbd_us_num_map[256] = {};


// Definición del layout en_US
kb_t layout_en_US = {
    .normal_map = _kbd_us_normal_map,
    .shift_map = _kbd_us_shift_map,
    .caps_map = _kbd_us_caps_map,
    .num_map = _kbd_us_num_map,
};





void set_kb_layout(kb_t *kb_layout) {
	keyboard_layout = kb_layout;
}

i_keyboard_t keyboard_interface;
char final_character;
bool mouse=false;
bool shift_pressed=false;
bool caps_lock=false;
bool num_lock=false;
bool win=false;
bool ctrl=false;
bool alt=false;
char kb_prefix;
uint8_t scancode;

void ps2_init() {
	outb(PS2_STATUS, PS2_DISABLE_FIRST);
	io_wait();
	outb(PS2_STATUS, PS2_DISABLE_SECOND);
	io_wait();

	// cleaning...
	while (inb(PS2_STATUS) & 0x01) {
		inb(PS2_DATA);
	}

	outb(PS2_STATUS, PS2_READ_CONFIG_BYTE);
	io_wait();
	uint8_t config = inb(PS2_DATA);

	// Enable interrupts of first port, who needs mouse?
	config |= 0x01;
	if (mouse) {
		config |= 0x02;
	} else {
		config &= ~0x02;
	}

	// disabling traduction...
	config &= ~0x40;

	// writing config...
	
	outb(PS2_STATUS, PS2_WRITE_CONFIG_BYTE);
	io_wait();
	outb(PS2_DATA, config);
	io_wait();

	outb(PS2_STATUS, PS2_SELF_TEST);
	io_wait();
	if (inb(PS2_DATA) == PS2_TEST_PASSED) {
		kprintf("-> PS2 SELF TEST [PASSED]\n");
	} else {
		kprintf("-> PS2 SELF TEST [FAILED]\n");
	}

	outb(PS2_STATUS, PS2_ENABLE_FIRST); io_wait();
	if (mouse) outb(PS2_STATUS, PS2_ENABLE_SECOND); io_wait();

	// reset keyboard
	outb(PS2_DATA, PS2_RESET);

	if (inb(PS2_DATA) == PS2_ACK) {
		kprintf("-> PS2 ACK [OK]\n");
	} else {
		kprintf("-> PS2 ACK [ERR]\n");
	}

	// selecting scancode set
	outb(PS2_DATA, PS2_SCANCODE_SET); io_wait();
	outb(PS2_DATA, 0x02); io_wait();// selecting scancode set number 2

	keyboard_interface.backspace = PS2_BACKSPACE;
	keyboard_interface.handle = ps2_handle;
	keyboard_interface.data_port = PS2_DATA;
	keyboard_interface.status_port = PS2_STATUS;
}

void keyboard_handler() {
	keyboard_interface.handle();
}

void ps2_handle() {
	scancode=inb(PS2_DATA);
	// DEBUG : kprintf("-> PS2 \n\t -> %d  Pressed!\n", (int)scancode);
	kb_common_handler();
}
void kb_common_handler() {
	uint16_t full_scancode = scancode;
	bool is_break=false;

	if (scancode==0xE0) {
		kb_prefix = 0xE0;
		return;
	} if (scancode==0xF0) {
		kb_prefix = 0xF0;
		return;
	}

	if (kb_prefix==0xE0) {
		full_scancode = (uint16_t)0xE000 | scancode;
	} else if (kb_prefix==0xF0) {
		is_break = true;

		// no se porque, pero se supone que lo tengo que hacer
		full_scancode = scancode;
	}
	// reiniciar prefijo
	kb_prefix = 0;

	switch (full_scancode) {
		case KC_LSHIFT: shift_pressed = !is_break; break;
		case KC_RSHIFT: shift_pressed = !is_break; break;
		case KC_LCTRL: ctrl = !is_break; break;
		case KC_CAPSLOCK: if (!is_break) caps_lock = !caps_lock; break;
		case KC_NUMLOCK: if (!is_break) num_lock = !num_lock;
		default: break;
	}

	if (!is_break) {
		final_character = '\0';
		if (shift_pressed) {
			final_character = keyboard_layout->shift_map[scancode];
		} else {
			final_character = keyboard_layout->normal_map[scancode];
		}

		if (caps_lock && final_character >= 'a' && final_character <= 'z') { 
			final_character -= 32; // convertir a mayuscula
		} else if (caps_lock && final_character >= 'A' && final_character <= 'Z') {
			final_character += 32;
		}
	}

}
