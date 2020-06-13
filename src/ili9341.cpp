#include "ili9341.hpp"

#define TAG "ILI9341"
#include <esp_log.h>

using namespace LCD;

static const gpio_t gpio = {
    .cs		= (gpio_num_t)14,
    .dc		= (gpio_num_t)27,
    .reset	= (gpio_num_t)33,
    .backlight = (gpio_num_t)32,
    .mosi		= (gpio_num_t)23,
    .sclk		= (gpio_num_t)18,
};
static const rect_t rect = {
    .x	  = 0,
    .y	  = 0,
    .width  = 320,
    .height = 230,
};
static Parameter parameters = {
    .gpio = gpio,
    .rect = rect,
};

ILI9341::ILI9341() : LCDBase(&parameters) {
    width = parameters.rect.width;
    height = parameters.rect.height;

	ESP_LOGI(TAG, "Initialize ILI9341 driver TFT Display");
	ESP_LOGI(TAG, "Screen width: %d, height: %d", width, height);


	size_t dataLengthArray[] = {
	    1, 1, 1, 1, 1, 2, 1, 1,  //
	    1, 1, 1, 1, 2, 2, 1, 4,  //
	    1, 1, 1, 15,		    // Positive Gamma
	    1, 15,			    // Negative Gamma
	    1, 0,				    // 24
	    1, 0};
	uint8_t data[] = {
	    0xc0, 0x23, 0xc1, 0x10, 0xc5, 0x3e, 0x28, 0xc7, 0x86,											   // 9
	    0x36, 0x08, 0x3a, 0x55, 0x20, 0xb1, 0x00, 0x18, 0xb6, 0x08, 0xa2, 0x27, 0x00,						   // 22 (+13)
	    0x26, 0x01, 0xE0, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,  // 40 (+18) Positive Gamma
	    0xE1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,			   // 56 (+16) Negative Gamma
	    0x11,																					   // 57 (+1)  Sleep out
	    0x29};

	spi_write_bytes(dataLengthArray, data);
	delayMS(120);
	spi_write_bytes(&dataLengthArray[24], &data[57]);

	setBacklight();
}

/* case Model9225: 
    uint8_t dataLengthArray[] = {
        1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0,  // 11
        1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0,  // 22 (+11)
        1, 2, 0,					   // 25 (+3)
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2, 0,  // 90 (+64)
        1, 2, 0};
    uint8_t data[] = {
        0x10, 0x00, 0x00, 0x11, 0x00, 0x00, 0x12, 0x00, 0x00, 0x13, 0x00, 0x00, 0x14, 0x00, 0x00,	// 15
        0x11, 0x00, 0x18, 0x12, 0x61, 0x21, 0x13, 0x00, 0x6F, 0x14, 0x49, 0x5F, 0x10, 0x08, 0x00,	// 30 (+15)
        0x11, 0x10, 0x3b,															// 33 (+3)
        0x01, 0x01, 0x1C, 0x02, 0x01, 0x00, 0x03, 0x10,
        0x30, 0x07, 0x00, 0x00, 0x08, 0x08, 0x08, 0x0B,
        0x11, 0x00, 0x0C, 0x00, 0x00, 0x0F, 0x08, 0x01,
        0x15, 0x00, 0x20, 0x20, 0x00, 0x00, 0x21, 0x00,
        0x00, 0x30, 0x00, 0x00, 0x31, 0x00, 0xDB, 0x32,
        0x00, 0x00, 0x33, 0x00, 0x00, 0x34, 0x00, 0xDB,
        0x35, 0x00, 0x00, 0x36, 0x00, 0xAF, 0x37, 0x00,
        0x00, 0x38, 0x00, 0xDB, 0x39, 0x00, 0x00, 0x50,
        0x00, 0x00, 0x51, 0x08, 0x08, 0x52, 0x08, 0x0A,
        0x53, 0x00, 0x0A, 0x54, 0x0A, 0x08, 0x55, 0x08,
        0x08, 0x56, 0x00, 0x00, 0x57, 0x0A, 0x00, 0x58,
        0x07, 0x10, 0x59, 0x07, 0x10, 0x07, 0x00, 0x12,  // 129 (+96)
        0x07, 0x10, 0x17};

    spi_write_bytes(&dataLengthArray[0], &data[0]);
    delayMS(40);
    spi_write_bytes(&dataLengthArray[11], &data[15]);
    delayMS(10);
    spi_write_bytes(&dataLengthArray[22], &data[30]);
    delayMS(50);
    spi_write_bytes(&dataLengthArray[25], &data[33]);
    delayMS(50);
    spi_write_bytes(&dataLengthArray[90], &data[129]);

*/

/* case Model9226:
    uint8_t dataLengthArray[] = {
        1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0,  // 21
        1, 2, 1, 2, 0,										   // 26 (+5)
        1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
        1, 2, 1, 2, 1, 2, 0,	 // 73 (+47)
        1, 2, 0};
    uint8_t data[] = {
        0x01, 0x02, 0x1c, 0x02, 0x01, 0x00, 0x03, 0x10,
        0x30, 0x08, 0x08, 0x08, 0x0B, 0x11, 0x00, 0x0C,
        0x00, 0x00, 0x0F, 0x14, 0x01, 0x15, 0x00, 0x00,
        0x20, 0x00, 0x00, 0x21, 0x00, 0x00,	 // 30
        0x10, 0x08, 0x00, 0x11, 0x1F, 0x3F,	 // 36 (+6)
        0x12, 0x01, 0x21, 0x13, 0x00, 0x6F, 0x14, 0x43,
        0x49, 0x30, 0x00, 0x00, 0x31, 0x00, 0xDB, 0x32,
        0x00, 0x00, 0x33, 0x00, 0x00, 0x34, 0x00, 0xDB,
        0x35, 0x00, 0x00, 0x36, 0x00, 0xAF, 0x37, 0x00,
        0x00, 0x38, 0x00, 0xDB, 0x39, 0x00, 0x00, 0x50,
        0x00, 0x01, 0x51, 0x20, 0x0B, 0x52, 0x00, 0x00,
        0x53, 0x04, 0x04, 0x54, 0x0C, 0x0C, 0x55, 0x00,
        0x0C, 0x56, 0x01, 0x01, 0x57, 0x04, 0x00, 0x58,
        0x11, 0x08, 0x59, 0x05, 0x0C,	// 105 (+69)
        0x07, 0x10, 0x17};

    spi_write_bytes(&dataLengthArray[0], &data[0]);
    delayMS(50);
    spi_write_bytes(&dataLengthArray[21], &data[30]);
    delayMS(50);
    spi_write_bytes(&dataLengthArray[26], &data[36]);
    delayMS(50);
    spi_write_bytes(&dataLengthArray[73], &data[105]);
*/

// Add 202001
// Draw multi pixel
// x:X coordinate
// y:Y coordinate
// size:Number of colors
// colors:colors
void ILI9341::drawPixels(uint16_t *pixels) {
	size_t dataLengthArray[] = {
	    1, 4, 1, 4, 1, 0};
	uint8_t data[] = {
	    0x2a, 0x00, 0x00, width >> 8, width & 0xff,
	    0x2b, 0x00, 0x00, height >> 8, height & 0xff,
	    0x2c};
	this->spi_write_bytes(dataLengthArray, data);
	size_t dataLengthArray2[] = {
	    0, width * height * sizeof(uint16_t), 0};
	this->spi_write_bytes(dataLengthArray2, (const uint8_t *)pixels);
}

/*

// Display OFF
void LCD::lcdDisplayOff(TFT_t *dev) {
	spi_master_write_comm_byte(0x28);
}

// Display ON
void LCD::lcdDisplayOn(TFT_t *dev) {
	spi_master_write_comm_byte(0x29);
}

// Display Inversion OFF
void LCD::lcdInversionOff(TFT_t *dev) {
	spi_master_write_comm_byte(0x20);
}

// Display Inversion ON
void LCD::lcdInversionOn(TFT_t *dev) {
	spi_master_write_comm_byte(0x21);
}

// Change Memory Access Control
void LCD::lcdBGRFilter(TFT_t *dev) {
	spi_master_write_comm_byte(0x36);	//Memory Access Control
	spi_master_write_data_byte(0x00);	//Right top start, RGB color filter panel
}

// Vertical Scrolling Definition
// tfa:Top Fixed Area
// vsa:Vertical Scrolling Area
// bfa:Bottom Fixed Area
void LCD::lcdSetScrollArea(uint16_t tfa, uint16_t vsa, uint16_t bfa) {
	spi_master_write_comm_byte(0x33);	// Vertical Scrolling Definition
	spi_master_write_data_word(dev, tfa);
	spi_master_write_data_word(dev, vsa);
	spi_master_write_data_word(dev, bfa);
	//spi_master_write_comm_byte(0x12);	// Partial Mode ON
}

void LCD::lcdResetScrollArea(TFT_t *dev) {
	spi_master_write_comm_byte(0x33);	// Vertical Scrolling Definition
	spi_master_write_data_word(dev, 0);
	spi_master_write_data_word(dev, 0x140);
	spi_master_write_data_word(dev, 0);
}

// Vertical Scrolling Start Address
// vsp:Vertical Scrolling Start Address
void LCD::lcdScroll(uint16_t vsp) {
	spi_master_write_comm_byte(0x37);	// Vertical Scrolling Start Address
	spi_master_write_data_word(dev, vsp);
}
*/
