#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f10x.h"
#include "uart.h"

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

UART uart;

void init_IOPC13() {
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	GPIOC->CRH |= GPIO_CRH_MODE13_1;
	GPIOC->CRH &= ~GPIO_CRH_CNF13;
	GPIOC->BSRR = GPIO_BSRR_BS13;
}

void toggleLed() {
	volatile int i = 0;
	GPIOC->BRR = GPIO_BRR_BR13;
	for (i = 0; i < 1000000; i++) {
	}
	GPIOC->BSRR = GPIO_BSRR_BS13;
	for (i = 0; i < 1000000; i++) {
	}
}

int main() {
	init_IOPC13();
	uart.begin(19200);
	uart.println("Test UART");

	while (1) {
		toggleLed();
		if (uart.available()) {
			uart.println(uart.readString());
			uart.flush();
		}
	}
	return 0;
}

#pragma GCC diagnostic pop
