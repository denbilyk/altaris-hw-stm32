/*
 * pins.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#include "pins.h"

void digitalWrite(GPIO_Pin *pin, uint8_t state) {
	if (state) {
		GPIO_SetBits(pin->gpio, pin->pinmask);
	} else {
		GPIO_ResetBits(pin->gpio, pin->pinmask);
	}
}

uint8_t digitalRead(GPIO_Pin *pin) {
	return GPIO_ReadInputDataBit(pin->gpio, pin->pinmask);
}

void pinMode(GPIO_Pin *pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed) {
	pin->rcc_periphery->clock_cmd(pin->rcc_periphery->rcc, ENABLE);

	GPIO_InitTypeDef gpio_config;
	GPIO_StructInit(&gpio_config);
	gpio_config.GPIO_Pin = pin->pinmask;
	gpio_config.GPIO_Speed = speed;
	gpio_config.GPIO_Mode = mode;
	GPIO_Init(pin->gpio, &gpio_config);
}

