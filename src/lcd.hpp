#pragma once

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace LCD {
enum Color : uint16_t {
	RED	  = 0xf800,
	GREEN  = 0x07e0,
	BLUE	  = 0x001f,
	BLACK  = 0x0000,
	WHITE  = 0xffff,
	GRAY	  = 0x8c51,
	YELLOW = 0xFFE0,
	CYAN	  = 0x07FF,
	PURPLE = 0xF81F,
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

class LCDBase {
    public:
	void clear(uint16_t color = BLACK);
	void swap(bool copy = false);

	uint16_t rgb565_conv(uint16_t r, uint16_t g, uint16_t b);

	void setBacklight(bool on = true);

    protected:
	LCDBase(Parameter *params);
	void spi_write_bytes(const size_t *dataLengthArray, const uint8_t *data);
	void delayMS(int ms);

    private:
	virtual void drawPixels(uint16_t *pixels) = 0;

	uint16_t *fore;
	uint16_t *back;
	uint32_t pixelCount;
	uint32_t size;

	spi_device_handle_t spi;

	gpio_t gpio;
	rect_t rect;
};

inline void LCD::delayMS(int ms) {
	vTaskDelay((ms + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS);
}

inline void LCDBase::setBacklight(bool on) {
	gpio_set_level(gpio.backlight, on);
}
}  // namespace LCD
