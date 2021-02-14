# About
Number recognition with MNIST on Raspberry Pi Pico + TensorFlow Lite for Microcontrollers

## Build
```sh
git clone https://github.com/iwatake2222/pico-mnist.git
cd pico-mnist
git submodule update --init
cd pico-sdk && git submodule update --init && cd ..
mkdir build && cd build

# For Windows Visual Studio 2019 (Developer Command Prompt for VS 2019)
# cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=on
cmake .. -G "NMake Makefiles"
nmake

# For Windows MSYS2 (Run the following commands on MSYS2)
# cmake .. -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug -DPICO_DEOPTIMIZED_DEBUG=on
cmake .. -G "MSYS Makefiles" 
make
```


## Acknowledgements
- pico-sdk
	- https://github.com/raspberrypi/pico-sdk
	- Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.
- pico-examples
	- Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.
	- https://github.com/raspberrypi/pico-examples
- pico-tflmicro
	- Copyright 2019 The TensorFlow Authors. All Rights Reserved.
	- https://github.com/raspberrypi/pico-tflmicro
