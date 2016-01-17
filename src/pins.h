/*
 * pins.h
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#ifndef PINS_H_
#define PINS_H_

#include "stm32f10x.h"

#define HIGH 1
#define LOW 0

typedef void (*RCC_APBPeriphClockCmd)(uint32_t RCC_APBPeriph, FunctionalState NewState);

typedef struct {
	RCC_APBPeriphClockCmd clock_cmd;
	uint32_t rcc;
} RCC_Periphery;

#define RCC_PERIPHERY(bus, periph) { RCC_##bus##PeriphClockCmd, RCC_##bus##Periph_##periph }

typedef struct {
	RCC_Periphery *rcc_periphery;
	GPIO_TypeDef *gpio;
	uint16_t pinmask;
	uint8_t pinsource;
} GPIO_Pin;

#define GPIO_PIN(gpio, pin_num) { &gpio##_RCC, gpio, GPIO_Pin_##pin_num, GPIO_PinSource##pin_num }


void digitalWrite(GPIO_Pin *pin, uint8_t state);

uint8_t digitalRead(GPIO_Pin *pin);

void pinMode(GPIO_Pin *pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed);

#endif /* PINS_H_ */
