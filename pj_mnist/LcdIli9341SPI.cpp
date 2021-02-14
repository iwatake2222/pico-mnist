#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <vector>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "font.h"
#include "LcdIli9341SPI.h"

static inline auto getSpi(int32_t spiNum)
{
	return (spiNum == 0) ? spi0 : spi1;
}

int32_t LcdIli9341SPI::initialize(const CONFIG& config)
{
	m_spiPortNum = config.spiPortNum;
	m_pinSck = config.pinSck;
	m_pinMosi = config.pinMosi;
	m_pinMiso = config.pinMiso;
	m_pinCs = config.pinCs;
	m_pinDc = config.pinDc;
	m_pinReset = config.pinReset;

	m_charPosX = 0;
	m_charPosY = 0;

	initializeIo();
	initializeDevice();
	setArea(0, 0, WIDTH, HEIGHT);

	return RET_OK;
}

void LcdIli9341SPI::initializeIo(void)
{
	spi_init(getSpi(m_spiPortNum), 50 * 1000 * 1000);
	gpio_set_function(m_pinSck , GPIO_FUNC_SPI);
	gpio_set_function(m_pinMosi , GPIO_FUNC_SPI);
	gpio_set_function(m_pinMiso, GPIO_FUNC_SPI);

	// gpio_set_function(m_pinCs, GPIO_FUNC_SPI);
	gpio_init(m_pinCs);
	gpio_set_dir(m_pinCs, GPIO_OUT);
	disableCs();

	gpio_init(m_pinDc);
	gpio_set_dir(m_pinDc, GPIO_OUT);

	gpio_init(m_pinReset);
	gpio_set_dir(m_pinReset, GPIO_OUT);
	gpio_put(m_pinReset, 0);
	sleep_ms(50);
	gpio_put(m_pinReset, 1);
	sleep_ms(50);
}

void LcdIli9341SPI::initializeDevice(void)
{
	writeCmd(0x01);	// Software Reset
	sleep_ms(50);
	writeCmd(0x11);	// Sleep Out
	sleep_ms(50);
	
	uint8_t dataBuffer[4];
	writeCmd(0xB6);	// Display Function Control
	dataBuffer[0] = 0x0a;	// Default
	dataBuffer[1] = 0xc2;	// G320 -> G1
	writeData(dataBuffer, 2);

	writeCmd(0x36);	// Memory Access Control
	writeData(0x68);	// Row Address Order, Row / Column Exchange, BGR
	writeCmd(0x3A);	// Pixel Format Set
	writeData(0x55);	// 16-bit

	writeCmd(0x29);	// Display ON
}

int32_t LcdIli9341SPI::finalize(void)
{
	spi_deinit(getSpi(m_spiPortNum));
	return RET_OK;
}

void LcdIli9341SPI::setArea(int32_t x, int32_t y, int32_t w, int32_t h)
{
	uint8_t dataBuffer[4];
	writeCmd(0x2A);
	dataBuffer[0] = (x >> 8) & 0xFF;
	dataBuffer[1] = x & 0xFF;
	dataBuffer[2] = ((x + w - 1) >> 8) & 0xFF;
	dataBuffer[3] = (x + w - 1) & 0xFF;
	writeData(dataBuffer, 4);
	writeCmd(0x2B);
	dataBuffer[0] = (y >> 8) & 0xFF;
	dataBuffer[1] = y & 0xFF;
	dataBuffer[2] = ((y + h - 1) >> 8) & 0xFF;
	dataBuffer[3] = (y + h - 1) & 0xFF;
	writeData(dataBuffer, 4);
}

void LcdIli9341SPI::putPixel(int32_t x, int32_t y, std::array<uint8_t, 2> color)
{
	setArea(x, y, 1, 1);
	writeCmd(0x2C);
	writeData(color.data(), 2);
}

void LcdIli9341SPI::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, std::array<uint8_t, 2> color)
{
	setArea(x, y, w, h);
	writeCmd(0x2C);
	for (int32_t i = 0; i < w * h; i++) {
		writeData(color.data(), 2);
	}
}

void LcdIli9341SPI::drawBuffer(int32_t x, int32_t y, int32_t w, int32_t h, std::vector<uint8_t> buffer)
{
	if (w * h * 2 != buffer.size()) {
		printf("error at LcdIli9341SPI::drawBuffer\n");
		return;
	}
	setArea(x, y, w, h);
	writeCmd(0x2C);
	writeData(buffer.data(), buffer.size());
}

void LcdIli9341SPI::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t size, std::array<uint8_t, 2> color)
{
	if (x0 != x1) {
		float a = (y1 - y0) / static_cast<float>(x1 - x0);
		int32_t b = (x1 * y0 - x0 * y1) / (x1 - x0);
		for (int32_t x = std::min(x0, x1); x <= std::max(x0, x1); x++) {
			int32_t y = static_cast<int32_t>(a * x + b);
			// putPixel(x, y, color);
			drawRect(x, y, size, size, color);
		}
	} else {
		for (int32_t y = std::min(y0, y1); y <= std::max(y0, y1); y++) {
			// putPixel(x0, y, color);
			drawRect(x0, y, size, size, color);
		}
	}
}

void LcdIli9341SPI::drawChar(int32_t x, int32_t y, char c)
{
	constexpr int32_t COLOR_SIZE = 2;
	std::vector<uint8_t> charBuffer(FONT_WIDTH * FONT_DISPLAY_SIZE * FONT_HEIGHT * FONT_DISPLAY_SIZE * COLOR_SIZE);
	for (int32_t x = 0; x < FONT_WIDTH; x++ ) {
		uint8_t line = *(font + ((c & 0x7F) * FONT_WIDTH) + x);
		for (uint8_t y = 0; y < FONT_HEIGHT; y++) {
			for (uint8_t sizeX = 0; sizeX < FONT_DISPLAY_SIZE; sizeX++) {
				for (uint8_t sizeY = 0; sizeY < FONT_DISPLAY_SIZE; sizeY++) {
					int32_t index = y * (FONT_DISPLAY_SIZE * FONT_WIDTH * FONT_DISPLAY_SIZE * COLOR_SIZE) + sizeY * (FONT_WIDTH * FONT_DISPLAY_SIZE * COLOR_SIZE) + x * (FONT_DISPLAY_SIZE * COLOR_SIZE) + sizeX * COLOR_SIZE;
					if (line & 0x1) {
						for (int32_t color = 0; color < COLOR_SIZE; color++) {
							charBuffer[index + color] = COLOR_TEXT_FG[color];
						}
					} else {
						for (int32_t color = 0; color < COLOR_SIZE; color++) {
							charBuffer[index + color] = COLOR_TEXT_BG[color];
						}
					}
				}
			}
			line >>= 1;
		}
	}
	drawBuffer(x, y, FONT_WIDTH * FONT_DISPLAY_SIZE, FONT_HEIGHT * FONT_DISPLAY_SIZE, charBuffer);
}

void LcdIli9341SPI::putChar(char c)
{
	drawChar(m_charPosX, m_charPosY, c);
	m_charPosX += FONT_WIDTH * FONT_DISPLAY_SIZE;
	if (m_charPosX + FONT_WIDTH * FONT_DISPLAY_SIZE >= WIDTH) {
		m_charPosY += FONT_HEIGHT * FONT_DISPLAY_SIZE;
		m_charPosX = 0;
		if (m_charPosY + FONT_HEIGHT * FONT_DISPLAY_SIZE >= HEIGHT) {
			m_charPosY = 0;
		}
	}
}

void LcdIli9341SPI::putText(std::string text)
{
	for (auto c : text) {
		if (c == '\0') break;
		putChar(c);
	}
}

void LcdIli9341SPI::setCharPos(int32_t charPosX, int32_t charPosY)
{
	m_charPosX = charPosX;
	m_charPosY = charPosY;
}




void LcdIli9341SPI::enableCs(void)
{
	// asm volatile("nop \n nop \n nop");
	gpio_put(m_pinCs, 0);	// Active low
	// asm volatile("nop \n nop \n nop");
}

void LcdIli9341SPI::disableCs(void)
{
	// asm volatile("nop \n nop \n nop");
	gpio_put(m_pinCs, 1);	// Active low
	// asm volatile("nop \n nop \n nop");
}

void LcdIli9341SPI::writeCmd(uint8_t cmd)
{
	gpio_put(m_pinDc, 0);
	enableCs();
	spi_write_blocking(getSpi(m_spiPortNum), &cmd, 1);
	disableCs();
}

void LcdIli9341SPI::writeData(uint8_t data)
{
	gpio_put(m_pinDc, 1);
	enableCs();
	spi_write_blocking(getSpi(m_spiPortNum), &data, 1);
	disableCs();
}

void LcdIli9341SPI::writeData(uint8_t dataBuffer[], int32_t len)
{
	gpio_put(m_pinDc, 1);
	enableCs();
	spi_write_blocking(getSpi(m_spiPortNum), dataBuffer, len);
	disableCs();
}

void LcdIli9341SPI::readData(uint8_t cmd, uint8_t dataBuffer[], int32_t len)
{
	gpio_put(m_pinDc, 0);
	enableCs();
	spi_write_blocking(getSpi(m_spiPortNum), &cmd, 1);
	
	gpio_put(m_pinDc, 1);
	spi_read_blocking(getSpi(m_spiPortNum), 0, dataBuffer, len);
	disableCs();
}

void LcdIli9341SPI::test()
{
	std::array<uint8_t, 2> colorBg = { 0x00, 0x1F };
	drawRect(0, 0, WIDTH, HEIGHT, colorBg);

	std::array<uint8_t, 2> color = { 0xF8, 0x00 };
	for(int32_t y = 100; y < 200; y++) {
		for(int32_t x = 100; x < 200; x++) {
			putPixel(x, y, color);
		}
	}

	std::vector<uint8_t> colorBuffer;
	for(int32_t i = 0; i < 20 * 20; i++) {
		colorBuffer.push_back(0x07);
		colorBuffer.push_back(0xE0);
	}
	drawBuffer(10, 10, 20, 20, colorBuffer);

	std::array<uint8_t, 2> colorLine = { 0x00, 0x00 };
	drawLine(50, 50, 100, 100, 2, colorLine);
	drawLine(100, 100, 150, 50, 2, colorLine);
	drawLine(150, 50, 100, 0, 2, colorLine);
	drawLine(100, 0, 50, 50, 2, colorLine);
	drawLine(50, 50, 150, 50, 2, colorLine);
	drawLine(50, 50,  50, 150, 2, colorLine);

	putText("ABC");
}
