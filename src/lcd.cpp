#include "lcd.hpp"

#include <math.h>
#include <string.h>

#include "shnm8x16r.hpp"

#define TAG "LCD"
#include <esp_log.h>

using namespace LCD;

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode    = 1;
//static const int SPI_Frequency = SPI_MASTER_FREQ_20M;
//static const int SPI_Frequency = SPI_MASTER_FREQ_26M;
static const int SPI_Frequency = SPI_MASTER_FREQ_40M;
//static const int SPI_Frequency = SPI_MASTER_FREQ_80M;

Font::FontBase *LCDBase::defaultFont = nullptr;

LCDBase::LCDBase(Parameter *params) {
	if (!defaultFont) LCDBase::defaultFont = new Font::shnm8x16r();

	esp_err_t ret;

	this->gpio = params->gpio;

	uint32_t pixels = params->rect.width * params->rect.height;

	buffer = (uint16_t *)heap_caps_malloc(pixels * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
	if (!buffer) {
		bool horizon = true;

		while (!(buffer = (uint16_t *)malloc(pixels * sizeof(uint16_t)))) {
			if (horizon)
				params->rect.width /= 2;
			else
				params->rect.height /= 2;

			horizon = !horizon;
			if (!(pixels = params->rect.width * params->rect.height)) {
				ESP_LOGI(TAG, "Cannot allocate frame buffer: %d", pixels);
				abort();
			}
		}
	}

	this->rect	  = params->rect;
	this->pixelCount = pixels;

	gpio_pad_select_gpio(gpio.cs);
	gpio_set_direction(gpio.cs, GPIO_MODE_OUTPUT);
	gpio_set_level(gpio.cs, 0);

	gpio_pad_select_gpio(gpio.dc);
	gpio_set_direction(gpio.dc, GPIO_MODE_OUTPUT);
	gpio_set_level(gpio.dc, 0);

	if (gpio.reset >= 0) {
		gpio_pad_select_gpio(gpio.reset);
		gpio_set_direction(gpio.reset, GPIO_MODE_OUTPUT);
		gpio_set_level(gpio.reset, 0);
		delayMS(100);
		gpio_set_level(gpio.reset, 1);
	}

	if (gpio.backlight >= 0) {
		gpio_pad_select_gpio(gpio.backlight);
		gpio_set_direction(gpio.backlight, GPIO_MODE_OUTPUT);
		gpio_set_level(gpio.backlight, 0);
	}

	// initialize SPI
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
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
	free(buffer);
}

void LCDBase::clear(uint16_t color) {
	for (int i = 0; i < pixelCount; i++) buffer[i] = color;
}

uint16_t *LCDBase::getBuffer() { return buffer; }

void LCDBase::update(uint16_t y, uint16_t height) {
	height = height ? height : rect.height - y;
	if (y + height > rect.height) height = rect.height - y;
	drawPixelsInitialize(y, height);
	spi_write_large_data((height * rect.width) / 2, (const uint32_t *)(buffer + y * rect.width));
	return;

	/* レジスタ直操作 Do not work
	gpio_set_level(gpio.dc, SPI_Data_Mode);

	const int SPI_NUM	 = 2;
	uint32_t repeat	 = pixelCount / 2;
	const uint32_t *data = buffer;

	if (repeat > 15) {
		SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(SPI_NUM), SPI_USR_MOSI_DBITLEN, 255, SPI_USR_MOSI_DBITLEN_S);

		while (repeat > 15) {
			while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR) continue;
			for (uint32_t i = 0; i < 16; i++) {
				WRITE_PERI_REG((SPI_W0_REG(SPI_NUM) + (i << 2)), *data++);
			}
			SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);
			repeat -= 16;
		}
		while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR) continue;
	}

	if (repeat) {
		repeat = (repeat << 4) - 1;
		SET_PERI_REG_BITS(SPI_MOSI_DLEN_REG(SPI_NUM), SPI_USR_MOSI_DBITLEN, repeat, SPI_USR_MOSI_DBITLEN_S);
		for (uint32_t i = 0; i < 16; i++) {
			WRITE_PERI_REG((SPI_W0_REG(SPI_NUM) + (i << 2)), *data++);
		}
		SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);
		while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR) continue;
	}
*/
}

uint16_t LCDBase::rgb565_conv(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t rgb = g;
	rgb <<= 3;
	rgb &= 0b0000000011100000;
	rgb |= (b >> 3);
	rgb <<= 5;
	rgb |= (r >> 3);
	rgb <<= 3;
	rgb |= (g >> 5);
	return rgb;
}

void LCDBase::spi_write_bytes(const size_t *dataLengthArray, const uint8_t *data) {
	spi_transaction_t transaction;
	int mode = SPI_Command_Mode;	//

	memset(&transaction, 0, sizeof(spi_transaction_t));

	do {
		gpio_set_level(gpio.dc, mode);

		transaction.length	  = *dataLengthArray * 8;
		transaction.tx_buffer = data;

		spi_device_transmit(this->spi, &transaction);

		data += *dataLengthArray;
		dataLengthArray++;
		mode = 1 - mode;  // Flip [SPI_Command_Mode : 0] <-> [SPI_Data_Mode : 1]
	} while (*dataLengthArray > 0);
}

void LCDBase::spi_write_command(const size_t length, const uint8_t *data) {
	spi_transaction_t transaction;
	memset(&transaction, 0, sizeof(spi_transaction_t));
	transaction.length	  = length * 8;
	transaction.tx_buffer = data;

	gpio_set_level(gpio.dc, SPI_Command_Mode);
	spi_device_transmit(this->spi, &transaction);
}

void LCDBase::spi_write_large_data(size_t length, const uint32_t *data) {
	spi_transaction_t transaction;

	memset(&transaction, 0, sizeof(spi_transaction_t));

	gpio_set_level(gpio.dc, SPI_Data_Mode);

	const size_t max = 512;
	while (length > 0) {
		size_t p			  = (length > max ? max : length);
		transaction.tx_buffer = data;
		transaction.length	  = p * 32;

		spi_device_transmit(spi, &transaction);

		data += p;
		length -= p;
	}
}

void LCDBase::drawString(uint16_t x, uint16_t y, uint16_t color, const char *text, Font::FontBase *font) {
	font->drawString(&rect, x, y, color, buffer, text);
}
