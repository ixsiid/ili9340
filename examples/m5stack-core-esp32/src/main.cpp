#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ili9341.hpp>

#define TAG "LCD test"
#include "log.h"

extern "C" {
void app_main();
}

void app_main() {
	_i("Start");
	ILI9341 *lcd = new ILI9341();

	// 周期Notifyループ
	xTaskCreatePinnedToCore([](void *lcd_p) {
		ILI9341 *lcd	   = (ILI9341 *)lcd_p;
		uint16_t colors[] = {};
		size_t n		   = 10;
		uint16_t i	   = 0;
		while (true) {
			// Core1はWatch Dog Timerを介入させるためにDelay挿入
			vTaskDelay(3000 / portTICK_PERIOD_MS);

			lcd->clear(colors[i++]);
			if (i >= n) i = 0;
			lcd->swap();
		}, "BlinkLCD", 4096, lcd, 1, NULL, 1);
}
