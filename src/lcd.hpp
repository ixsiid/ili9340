#pragma once

#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "types.hpp"
#include "font.hpp"

namespace LCD {

class LCDBase {
    public:
	virtual ~LCDBase();
	void clear(uint16_t color = BLACK);
	void swap();

	static uint16_t rgb565_conv(uint8_t r, uint8_t g, uint8_t b);

	void setBacklight(bool on = true);

	void drawString(uint16_t x, uint16_t y, uint16_t color, const char * text, Font::FontBase * font = defaultFont);

    protected:
	LCDBase(Parameter *params);
	void spi_write_bytes(const size_t *dataLengthArray, const uint8_t *data);
	void delayMS(int ms);

    private:
	virtual void drawPixelsInitialize() = 0;
	void spi_write_large_data(size_t length, const uint8_t *data);

	static Font::FontBase * defaultFont;

	uint16_t *fore;
	uint16_t *back;
	uint32_t pixelCount;
	uint32_t size;

	spi_device_handle_t spi;

	gpio_t gpio;
	rect_t rect;
};

inline void LCDBase::delayMS(int ms) {
	vTaskDelay((ms + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS);
}

inline void LCDBase::setBacklight(bool on) {
	gpio_set_level(gpio.backlight, on);
}
}  // namespace LCD
