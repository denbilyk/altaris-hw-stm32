#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f10x.h"
#include "uart/uart.h"
#include "utils/WString.h"
#include "utils/delay.h"
#include "pins.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

UART uart;

void toggleLed() {
	digitalWrite(&LED_C13, HIGH);
	delay_ms(1000);
	digitalWrite(&LED_C13, LOW);
	delay_ms(1000);
}

int main() {
	pinMode(&LED_C13, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	delay_init();
	uart.begin(19200);
	uart.println("Test UART");

	while (1) {
		toggleLed();
		if (uart.available()) {
			String s = String(uart.readString());
			trace_printf("ReadString: %s", s.c_str());
			uart.println(s.c_str());
			uart.flush();
		}
	}
	return 0;
}

#pragma GCC diagnostic pop
