/*
 * exti.c
 *
 *  Created on: Jan 30, 2016
 *      Author: denis.bilyk
 */

#include <stdbool.h>
#include "exti.h"
#include "stm32f10x.h"
#include "rf24/rf24.h"

#define RX_PAYLOAD 6
uint16_t received_data[RX_PAYLOAD];
volatile uint8_t have_data;
extern RF24 rf24;

void exti10_init() {

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	// EXTI pin
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);

	// Configure EXTI line1
	EXTI_InitTypeDef EXTIInit;
	EXTIInit.EXTI_Line = EXTI_Line10;             // EXTI will be on line 10
	EXTIInit.EXTI_LineCmd = ENABLE;               // EXTI1 enabled
	EXTIInit.EXTI_Mode = EXTI_Mode_Interrupt;     // Generate IRQ
	EXTIInit.EXTI_Trigger = EXTI_Trigger_Falling; // IRQ on signal falling
	EXTI_Init(&EXTIInit);

	// Configure EXTI1 interrupt
	NVIC_InitTypeDef NVICInit;
	NVICInit.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVICInit.NVIC_IRQChannelCmd = ENABLE;
	NVICInit.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVICInit.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_Init(&NVICInit);
}

extern "C" {
void EXTI15_10_IRQHandler(void) {
	uint8_t i;
	bool status;

	if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
		status = rf24.available();
		if (status) {
			memset(&received_data, 0, RX_PAYLOAD);
			have_data = 1;
			bool done = false;
			while (!done) {
				done = rf24.read(&received_data, RX_PAYLOAD * 2); //method expected read uint8_t data. It will be uint16_t size * 2
			}
		}

		EXTI_ClearITPendingBit(EXTI_Line10);
	}
}
}
