#ifndef DISPLAY
#define DISPLAY

#include <stdint.h>

typedef struct {
	void (*init)(uint32_t mode_id);
	void (*clear)(void);
	void (*put_char)(uint16_t x, uint16_t y, char c, uint8_t attr);
	void (*print_string)(uint16_t x, uint16_t y, const char *str, uint8_t attr);
	void (*set_cursor)(uint16_t x, uint16_t y);
	void (*put_pixel)(uint16_t x, uint16_t y, uint32_t color_rgb);
	void (*set_mode)(uint32_t new_mode_id);
} DisplayDriver;

typedef enum {
	DISPLAY_MODE_TEXT_VGA_80x25,
	DISPLAY_MODE_GRAPHICS_VGA_320x200_256COLOR,
	DISPLAY_MODE_FRAMEBUFFER_RGBA,
} display_driver_t;

#endif
