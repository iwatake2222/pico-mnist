#include <cstdint>
#include <cstdio>
// #include <cstdlib>
// #include <cstring>

#include "pico/stdlib.h"

int main() {
	stdio_init_all();
	const uint32_t LED_PIN = 25;
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	while (true) {
		printf("Hello, world!\n");
		gpio_put(LED_PIN, 1);
		sleep_ms(250);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);
	}

	return 0;
}
