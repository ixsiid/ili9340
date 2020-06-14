#pragma once

#include "lcd.hpp"

/*
use gpio
    .cs		= (gpio_num_t)14,
    .dc		= (gpio_num_t)27,
    .reset	= (gpio_num_t)33,
    .backlight = (gpio_num_t)32,
    .mosi		= (gpio_num_t)23,
    .sclk		= (gpio_num_t)18,
*/

namespace LCD {
class ILI9341 : public LCDBase {
    public:
	ILI9341();

    private:
	void drawPixelsInitialize() override;

	uint16_t width;
	uint16_t height;
};

}  // namespace LCD
