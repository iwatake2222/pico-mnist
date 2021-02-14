#ifndef LCD_ILI9341_SPI_H_
#define LCD_ILI9341_SPI_H_

#include <cstdint>
#include <array>
#include <vector>
#include <string>

class LcdIli9341SPI {
public:
	static constexpr int32_t WIDTH = 320;
	static constexpr int32_t HEIGHT = 240;
	static constexpr int32_t FONT_DISPLAY_SIZE = 3;
	static constexpr uint32_t COLOR_TEXT_FG[2] = {0x00, 0x00};
	static constexpr uint32_t COLOR_TEXT_BG[2] = {0xFF, 0xFF};
	
	enum {
		RET_OK = 0,
		RET_ERR = -1,
	};

	typedef struct CONFIG_ {
		int32_t spiPortNum;
		int32_t pinSck;
		int32_t pinMosi;
		int32_t pinMiso;
		int32_t pinCs;
		int32_t pinDc;
		int32_t pinReset;
	} CONFIG;

public:
	LcdIli9341SPI() {}
	~LcdIli9341SPI() {}
	int32_t initialize(const CONFIG& config);
	int32_t finalize(void);
	void test();
	void setArea(int32_t x, int32_t y, int32_t w, int32_t h);
	void putPixel(int32_t x, int32_t y, std::array<uint8_t, 2> color);
	void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, std::array<uint8_t, 2> color);
	void drawBuffer(int32_t x, int32_t y, int32_t w, int32_t h, std::vector<uint8_t> buffer);
	void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t size, std::array<uint8_t, 2> color);

	void drawChar(int32_t x, int32_t y, char c);
	void putChar(char c);
	void putText(std::string text);
	void setCharPos(int32_t charPosX, int32_t charPosY);

private:
	void initializeIo(void);
	void initializeDevice(void);
	void enableCs(void);
	void disableCs(void);
	void writeCmd(uint8_t cmd);
	void writeData(uint8_t data);
	void writeData(uint8_t dataBuffer[], int32_t len);
	void readData(uint8_t cmd, uint8_t dataBuffer[], int32_t len);
	

private:
	int32_t m_spiPortNum;
	int32_t m_pinSck;
	int32_t m_pinMosi;
	int32_t m_pinMiso;
	int32_t m_pinCs;
	int32_t m_pinDc;
	int32_t m_pinReset;

private:
	int32_t m_charPosX;
	int32_t m_charPosY;
};

#endif
