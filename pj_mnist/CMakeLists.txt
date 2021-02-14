set(BinName "pj_mnist")
add_executable(${BinName}
	Main.cpp
	LcdIli9341SPI.h
	LcdIli9341SPI.cpp
	TpTsc2046SPI.h
	TpTsc2046SPI.cpp
	font.cpp
	font.h
	conv_mnist_quant.h
	conv_mnist_quant.cpp
)

pico_enable_stdio_usb(${BinName} 1)
pico_enable_stdio_uart(${BinName} 1)

target_link_libraries(${BinName}
	pico_stdlib
	hardware_spi
	pico-tflmicro
)

pico_add_extra_outputs(${BinName})
