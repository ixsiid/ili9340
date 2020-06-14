#pragma once
#include <driver/gpio.h>

namespace LCD {

enum Color : uint16_t {
	RED	 = 0b0000000011111000,
	GREEN = 0b1110000000000111,
	BLUE	 = 0b0001111100000000,
	BLACK = 0x0000,
	WHITE = 0xffff,
};

enum Direction {
	Direction__0,
	Direction_90,
	Direction180,
	Direction270,
};

enum Model {
	Model9340,
	Model9341,
	Model7735,
	Model9225,
	Model9226,
};

typedef struct {
	gpio_num_t cs;
	gpio_num_t dc;
	gpio_num_t reset;
	gpio_num_t backlight;
	gpio_num_t mosi;
	gpio_num_t sclk;
} gpio_t;

typedef struct {
	int x;
	int y;
	int width;
	int height;
} rect_t;

typedef struct {
	gpio_t gpio;
	rect_t rect;
} Parameter;
}  // namespace LCD
