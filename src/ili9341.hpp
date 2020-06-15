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

	void sleep() override;
	void wakeup(bool update) override;

	void hidden() override;
	void visible() override;

	void deepSleep() override;
	void recovery() override;

    private:
	void drawPixelsInitialize(uint16_t y, uint16_t height) override;
	void writeInitialRegister();

	uint16_t width;
	uint16_t height;

	bool deepsleeping;
};

}  // namespace LCD
