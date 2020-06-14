# M5Stack LCD driver

Double buffered LCD driver for M5Stack on ESP-IDF

Use default font JFドット東雲ゴシック16 (http://web.archive.org/web/20181210064642/http://openlab.ring.gr.jp/efont/shinonome/)


```cpp:main.cpp
#include <ili9341.hpp>

extern "C" {
void app_main();
}

void app_main() {
	_i("Start");
	LCD::ILI9341 *lcd = new ILI9341();
    
	lcd->clear(LCD::Color::WHITE);
	lcd->drawString(20, 30, LCD::Color::BLACK, "Hello, world");
	lcd->swap();
}

```
