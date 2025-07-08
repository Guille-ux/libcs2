#ifndef KEYMAPS_H
#define KEYMAPS_H

#include <stdint.h>

#define KC_LSHIFT       0x12
#define KC_RSHIFT       0x59
#define KC_LCTRL        0x14
#define KC_LALT         0x11
#define KC_RALT_EXT     0xE011
#define KC_LSUPER_EXT   0xE02F
#define KC_RSUPER_EXT   0xE027
#define KC_CAPSLOCK     0x58
#define KC_NUMLOCK      0x77
#define KC_SCROLLLOCK   0x7E
#define KC_BACKSPACE    0x66
#define KC_ENTER        0x5A
#define KC_TAB          0x0D
#define KC_ESC          0x76

typedef struct {
	const uint8_t *normal_map;
	const uint8_t *shift_map;
	const uint8_t *caps_map;
	const uint8_t *num_map;
} kb_t;

extern char kb_prefix;
extern kb_t layout_en_US;
extern kb_t *keyboard_layout;
void set_kb_layout(unsigned char *kb_layout);

#endif
