#pragma once

#include <sys/types.h>

#include "types.hpp"

namespace Font {
class FontBase {
    public:
	void drawString(const LCD::rect_t* screen, uint16_t x, uint16_t y, uint16_t color, uint16_t *buffer, const char * text);
	void drawString(const LCD::rect_t* screen, uint16_t x, uint16_t y, uint16_t color, uint16_t bg_color, uint16_t *buffer, const char * text);

    protected:
	size_t width;
	size_t height;
	uint8_t* font[128];
};
}  // namespace Font
