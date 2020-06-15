#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ili9341.hpp>

#include <button.hpp>

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
		ILI9341 *lcd = (ILI9341 *)lcd_p;
		bool sleeped = false;

		uint8_t buttonPins[] = {37, 38, 39};
		Button *button = new Button(buttonPins, 3);

		uint16_t colors[] = {
		    BLACK, WHITE, RED, GREEN, BLUE, (uint16_t)~RED, (uint16_t)~GREEN, (uint16_t)~BLUE,
		    BLACK};
		size_t n	 = 8;
		uint16_t i = 0;

		int64_t current = esp_timer_get_time();
		while (true) {
			// Core1はWatch Dog Timerを介入させるためにDelay挿入
			vTaskDelay(1);

			button->check(nullptr, [&](uint8_t pin) {
				switch (pin) {
					case 37:
						if (sleeped) {
							sleeped = !sleeped;
							lcd->wakeup(true);
						}
						break;
					case 39:
						if (!sleeped) {
							sleeped = !sleeped;
							lcd->sleep();
						}
						break;
				}
			});

			int64_t now = esp_timer_get_time();

			if (now - current > 3000000) {
				current	  = now;
				uint16_t bg = ~colors[i];

				lcd->clear(colors[i]);
				lcd->drawString(20, 30, bg, "Hello, world");
				if (!sleeped) lcd->update();

				if (i++ >= n) i = 0;
			}
		}
	}, "BlinkLCD", 8192, lcd, 1, NULL, 1);
}
