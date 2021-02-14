#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "TpTsc2046SPI.h"

static inline auto getSpi(int32_t spiNum)
{
	return (spiNum == 0) ? spi0 : spi1;
}

int32_t TpTsc2046SPI::initialize(const CONFIG& config)
{
	m_spiPortNum = config.spiPortNum;
	m_pinSck = config.pinSck;
	m_pinMosi = config.pinMosi;
	m_pinMiso = config.pinMiso;
	m_pinCs = config.pinCs;
	m_pinIrq = config.pinIrq;
	m_callback = config.callback;

	initializeIo();

	return RET_OK;
}

void TpTsc2046SPI::initializeIo(void)
{
	spi_init(getSpi(m_spiPortNum), 1 * 1000 * 1000);
	gpio_set_function(m_pinSck , GPIO_FUNC_SPI);
	gpio_set_function(m_pinMosi , GPIO_FUNC_SPI);
	gpio_set_function(m_pinMiso, GPIO_FUNC_SPI);

	// gpio_set_function(m_pinCs, GPIO_FUNC_SPI);
	gpio_init(m_pinCs);
	gpio_set_dir(m_pinCs, GPIO_OUT);
	disableCs();

	gpio_init(m_pinIrq);
	gpio_set_dir(m_pinIrq, GPIO_IN);
	enableTouchIrq();
}

int32_t TpTsc2046SPI::finalize(void)
{
	spi_deinit(getSpi(m_spiPortNum));
	return RET_OK;
}

void TpTsc2046SPI::getFromDevice(float& x, float& y, float& pressure)
{
	constexpr float NORM_VALUE = 2048.0;
	
	disableTouchIrq();
	enableCs();
	uint8_t dataBuffer[2] = { 0 };
	uint8_t cmd = 0;

	/* X */
	cmd = createCmd(1, 0, 0);
	int32_t rawX = 0;
	for (int32_t i = 0; i < MEASURE_NUM; i++) {
		spi_write_blocking(getSpi(m_spiPortNum), &cmd, 1);
		spi_read_blocking(getSpi(m_spiPortNum), 0, dataBuffer, 2);
		rawX += ((dataBuffer[0] << 4) & 0xFF0) | ((dataBuffer[1] >> 4) & 0x0F);
		// printf("%d\n", rawX);
	}
	x = rawX / (NORM_VALUE * MEASURE_NUM);

	/* Y */
	cmd = createCmd(5, 0, 0);
	int32_t rawY = 0;
	for (int32_t i = 0; i < MEASURE_NUM; i++) {
		spi_write_blocking(getSpi(m_spiPortNum), &cmd, 1);
		spi_read_blocking(getSpi(m_spiPortNum), 0, dataBuffer, 2);
		rawY += ((dataBuffer[0] << 4) & 0xFF0) | ((dataBuffer[1] >> 4) & 0x0F);
		// printf("%d\n", rawY);
	}
	y = rawY / (NORM_VALUE * MEASURE_NUM);

	/* Pressure */
	cmd = createCmd(3, 0, 0);
	int32_t rawPressure = 0;
	for (int32_t i = 0; i < MEASURE_NUM; i++) {
		spi_write_blocking(getSpi(m_spiPortNum), &cmd, 1);
		spi_read_blocking(getSpi(m_spiPortNum), 0, dataBuffer, 2);
		rawPressure += ((dataBuffer[0] << 4) & 0xFF0) | ((dataBuffer[1] >> 4) & 0x0F);
		// printf("%d\n", rawPressure);
	}
	pressure = rawPressure / MEASURE_NUM;

	disableCs();
	enableTouchIrq();
}

uint8_t TpTsc2046SPI::createCmd(uint8_t A, uint8_t mode, uint8_t ser)
{
	return 0x80 | (A << 4) | (mode << 3) | (ser << 2) | (0 << 0);
}

void TpTsc2046SPI::enableCs(void)
{
	asm volatile("nop \n nop \n nop");
	gpio_put(m_pinCs, 0);	// Active low
	asm volatile("nop \n nop \n nop");
}

void TpTsc2046SPI::disableCs(void)
{
	asm volatile("nop \n nop \n nop");
	gpio_put(m_pinCs, 1);	// Active low
	asm volatile("nop \n nop \n nop");
}

void TpTsc2046SPI::enableTouchIrq(void)
{
	if (m_callback) {
		gpio_set_irq_enabled_with_callback(m_pinIrq, GPIO_IRQ_LEVEL_LOW | GPIO_IRQ_EDGE_FALL, true, m_callback);
	}
}

void TpTsc2046SPI::disableTouchIrq(void)
{
	if (m_callback) {
		gpio_set_irq_enabled_with_callback(m_pinIrq, GPIO_IRQ_LEVEL_LOW | GPIO_IRQ_EDGE_FALL, false, m_callback);
	}
}
