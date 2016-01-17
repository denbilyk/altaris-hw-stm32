/*
 * delay.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#include "delay.h"

static volatile uint32_t timing_delay;

void delay_init() {
	if (SysTick_Config(SystemCoreClock / 1000)) {
		/* If not a zero - error */
		while (1)
			;
	}
}

void delay_ms(volatile uint32_t ms) {
	timing_delay = ms;
	while (timing_delay != 0);
}

void timing_delay_decrement(void) {
	if (timing_delay != 0x00) {
		timing_delay--;
	}
}

#ifdef __cplusplus
extern "C" {
#endif

void SysTick_Handler(void) {
	timing_delay_decrement();
}

#ifdef __cplusplus
}
#endif


