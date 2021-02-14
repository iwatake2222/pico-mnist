#ifndef TP_TSC2046_SPI_H_
#define TP_TSC2046_SPI_H_

#include <cstdint>

class TpTsc2046SPI {
public:
	static constexpr int32_t MEASURE_NUM = 10;		// Measure values several times to reduce noise
	
	enum {
		RET_OK = 0,
		RET_ERR = -1,
	};

	typedef void(*FP_CALLBACK)(uint, uint32_t);

	typedef struct CONFIG_ {
		int32_t spiPortNum;
		int32_t pinSck;
		int32_t pinMosi;
		int32_t pinMiso;
		int32_t pinCs;
		int32_t pinIrq;
		FP_CALLBACK callback;
	} CONFIG;
	
public:
	TpTsc2046SPI() {}
	~TpTsc2046SPI() {}
	int32_t initialize(const CONFIG& config);
	int32_t finalize(void);
	void getFromDevice(float& x, float& y, float& pressure);

private:
	void initializeIo(void);
	void enableCs(void);
	void disableCs(void);
	void enableTouchIrq(void);
	void disableTouchIrq(void);
	uint8_t createCmd(uint8_t A, uint8_t mode, uint8_t ser);
	
private:
	int32_t m_spiPortNum;
	int32_t m_pinSck;
	int32_t m_pinMosi;
	int32_t m_pinMiso;
	int32_t m_pinCs;
	int32_t m_pinIrq;
	FP_CALLBACK m_callback;
};

#endif
