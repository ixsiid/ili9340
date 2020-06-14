#include "font.hpp"

#include <string.h>

using namespace Font;

void FontBase::drawString(const LCD::rect_t* screen, uint16_t x, uint16_t y, uint16_t color, uint16_t* buffer, const char* text) {
	size_t h = height;
	if (y + h > screen->height) h = screen->height - y;

	size_t w = strlen(text) * width;
	if (x + w > screen->width) w = screen->width - x;

	for (int _y = 0; _y < h; _y++) {
		uint16_t* b = buffer + x + (screen->width * (y + _y));

		uint8_t n = 0x80;
		for (int _x = 0; _x < w; _x++) {
			uint8_t* f = font[(text[_x / 8] - 32) & 0x7f] + _y;
			if (*f & n) *b = color;

			n >>= 1;
			if (!n) n = 0x80;
			b++;
		}
	}
}

void FontBase::drawString(const LCD::rect_t* screen, uint16_t x, uint16_t y, uint16_t color, uint16_t bg_color, uint16_t* buffer, const char* text) {
	int x1 = x - 1;
	int x2 = x + strlen(text) * width + 1;
	int y1 = y - 1;
	int y2 = y + height + 1;

	if (x1 < 0) x1 = 0;
	if (x2 > screen->width) x2 = screen->width;

	if (y1 < 0) y1 = 0;
	if (y2 > screen->height) y2 = screen->height;

	for (int y = y1; y < y2; y++) {
		uint16_t* b = buffer + (screen->width * y) + x1;
		int w	  = x2 - x1;
		while (w-- > 0) *b++ = bg_color;
	}

	drawString(screen, x, y, color, buffer, text);
}
