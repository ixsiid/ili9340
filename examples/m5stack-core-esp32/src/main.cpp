#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ili9341.hpp>

#define TAG "LCD test"
#include "log.h"

extern "C" {
void app_main();
}

using namespace LCD;

void app_main() {
	_i("Start");
	ILI9341 *lcd = new ILI9341();

	xTaskCreatePinnedToCore([](void *lcd_p) {
		ILI9341 *lcd	   = (ILI9341 *)lcd_p;
		uint16_t colors[] = {
		    BLACK, WHITE, RED, GREEN, BLUE,
		    LCD::LCDBase::rgb565_conv(0, 0, 0),
		    LCD::LCDBase::rgb565_conv(255, 255, 255),
		    LCD::LCDBase::rgb565_conv(255, 0, 0),
		    LCD::LCDBase::rgb565_conv(0, 255, 0),
		    LCD::LCDBase::rgb565_conv(0, 0, 255),
		    BLACK};
		size_t n	 = 10;
		uint16_t i = 0;
		while (true) {
			// Core1はWatch Dog Timerを介入させるためにDelay挿入
			vTaskDelay(3000 / portTICK_PERIOD_MS);

			_i("Change %x", colors[i]);
			lcd->clear(colors[i]);
			lcd->drawString(20, 30, ~colors[i], "Hello, world");
			lcd->swap();

			if (i++ >= n) i = 0;
		}
	},
					    "BlinkLCD", 4096, lcd, 1, NULL, 1);
}
