#include "lcd.hpp"

#include <math.h>
#include <string.h>

#define TAG "LCD"
#include <esp_log.h>

using namespace LCD;

static const int GPIO_MOSI = 23;
static const int GPIO_SCLK = 18;

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode    = 1;
//static const int SPI_Frequency = SPI_MASTER_FREQ_20M;
////static const int SPI_Frequency = SPI_MASTER_FREQ_26M;
static const int SPI_Frequency = SPI_MASTER_FREQ_40M;
////static const int SPI_Frequency = SPI_MASTER_FREQ_80M;

LCDBase::LCDBase(Parameter *params) {
	esp_err_t ret;

	this->gpio  = params->gpio;
	this->rect  = params->rect;

	this->pixelCount = rect.width * rect.height;
	this->size = pixelCount * sizeof(uint16_t);

	fore = (uint16_t *)malloc(size);
	back = (uint16_t *)malloc(size);

	ESP_LOGI(TAG, "GPIO_CS=%d", gpio.cs);
	gpio_pad_select_gpio(gpio.cs);
	gpio_set_direction(gpio.cs, GPIO_MODE_OUTPUT);
	gpio_set_level(gpio.cs, 0);

	ESP_LOGI(TAG, "GPIO_DC=%d", gpio.dc);
	gpio_pad_select_gpio(gpio.dc);
	gpio_set_direction(gpio.dc, GPIO_MODE_OUTPUT);
	gpio_set_level(gpio.dc, 0);

	ESP_LOGI(TAG, "gpio.reset=%d", gpio.reset);
	if (gpio.reset >= 0) {
		gpio_pad_select_gpio(gpio.reset);
		gpio_set_direction(gpio.reset, GPIO_MODE_OUTPUT);
		gpio_set_level(gpio.reset, 0);
		delayMS(100);
		gpio_set_level(gpio.reset, 1);
	}

	ESP_LOGI(TAG, "gpio.backlight=%d", gpio.backlight);
	if (gpio.backlight >= 0) {
		gpio_pad_select_gpio(gpio.backlight);
		gpio_set_direction(gpio.backlight, GPIO_MODE_OUTPUT);
		gpio_set_level(gpio.backlight, 0);
	}

	// initialize SPI

	spi_bus_config_t buscfg = {
	    .mosi_io_num   = gpio.mosi,
	    .miso_io_num   = -1,
	    .sclk_io_num   = gpio.sclk,
	    .quadwp_io_num = -1,
	    .quadhd_io_num = -1};

	ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
	ESP_LOGD(TAG, "spi_bus_initialize=%d", ret);
	assert(ret == ESP_OK);

	spi_device_interface_config_t devcfg = {
	    .clock_speed_hz = SPI_Frequency,
	    .spics_io_num	= gpio.cs,
	    .flags		= SPI_DEVICE_NO_DUMMY,
	    .queue_size	= 7,
	};

	ret = spi_bus_add_device(HSPI_HOST, &devcfg, &this->spi);
	ESP_LOGD(TAG, "spi_bus_add_device=%d", ret);
	assert(ret == ESP_OK);
}

LCDBase::~LCDBase() {
	free(fore);
	free(back);
}

void LCDBase::clear(uint16_t color) {
	for(int i=0; i<pixelCount; i++) back[i] = color;
}

void LCDBase::swap(bool copy){
	uint16_t * current = fore;

	fore = back;
	back = current;
	drawPixels(fore);
	if (copy) memcpy(back, fore, size);
}

void LCDBase::spi_write_bytes(const size_t *dataLengthArray, const uint8_t *data) {
	spi_transaction_t transaction;
	esp_err_t ret;
	int mode = SPI_Command_Mode;	//

	memset(&transaction, 0, sizeof(spi_transaction_t));

	do {
		transaction.length	  = *dataLengthArray;
		transaction.tx_buffer = data;

		ret = spi_device_transmit(this->spi, &transaction);
		// assert(ret == ESP_OK);

		data += *dataLengthArray;
		dataLengthArray++;
		mode = 1 - mode; // Flip [SPI_Command_Mode] <-> [SPI_Data_Mode]
	} while (*dataLengthArray > 0);
}
