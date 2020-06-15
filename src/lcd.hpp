#pragma once

#include <driver/spi_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "font.hpp"
#include "types.hpp"

namespace LCD {

class LCDBase {
    public:
	virtual ~LCDBase();
	void clear(uint16_t color = BLACK);
	void update(uint16_t y = 0, uint16_t height = 0);

	static uint16_t rgb565_conv(uint8_t r, uint8_t g, uint8_t b);

	void setBacklight(bool on = true);

	void drawString(uint16_t x, uint16_t y, uint16_t color, const char *text, Font::FontBase *font = defaultFont);

	uint16_t *getBuffer();

	/** スリープさせます。
	 * 消費電力 ディープスリープ < スリープ < 非表示 の想定
	 */
	virtual void sleep() = 0;
	/** スリープから復帰します。
	 */
	virtual void wakeup(bool update) = 0;

	/** 非表示にします。
	 * 消費電力 ディープスリープ < スリープ < 非表示 の想定
	 */
	virtual void hidden() = 0;
	/** ディスプレイを表示します。
	 */
	virtual void visible() = 0;

	/** ディープスリープさせます。
	 * 消費電力 ディープスリープ < スリープ < 非表示 の想定
	 */
	virtual void deepSleep() = 0;
	/** ディープスリープから復帰します。
	 */
	virtual void recovery() = 0;

    protected:
	LCDBase(Parameter *params);
	void spi_write_bytes(const size_t *dataLengthArray, const uint8_t *data);
	void spi_write_command(const size_t length, const uint8_t *byte);
	void delayMS(int ms);

    private:
	virtual void drawPixelsInitialize(uint16_t y, uint16_t height) = 0;
	void spi_write_large_data(size_t length, const uint32_t *data);

	static Font::FontBase *defaultFont;

	uint16_t *buffer;
	uint32_t pixelCount;

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
