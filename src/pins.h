/*
 * pins.h
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#include "stm32f10x.h"

#ifndef PINS_H_
#define PINS_H_

#define HIGH 1
#define LOW 0

typedef void (*RCC_APBPeriphClockCmd)(uint32_t RCC_APBPeriph,
		FunctionalState NewState);

typedef struct {
	RCC_APBPeriphClockCmd clock_cmd;
	uint32_t rcc;
} RCC_Periphery;

#define RCC_PERIPHERY(bus,periph) { RCC_##bus##PeriphClockCmd, RCC_##bus##Periph_##periph }

typedef struct {
	RCC_Periphery *rcc_periphery;
	GPIO_TypeDef *gpio;
	uint16_t pinmask;
	uint8_t pinsource;
} GPIO_Pin;

#define GPIO_PIN(gpio,pin_num) { &gpio##_RCC, gpio, GPIO_Pin_##pin_num, GPIO_PinSource##pin_num }

RCC_Periphery GPIOA_RCC = RCC_PERIPHERY(APB2, GPIOA), GPIOB_RCC =
RCC_PERIPHERY(APB2, GPIOB), GPIOC_RCC = RCC_PERIPHERY(APB2, GPIOC), GPIOD_RCC =
RCC_PERIPHERY(APB2, GPIOD);

GPIO_Pin
LED_C13 = GPIO_PIN(GPIOC, 13),
NRF_CE = GPIO_PIN(GPIOA, 3), NRF_CSN = GPIO_PIN(GPIOA, 4), NRF_SPI_SCK =
GPIO_PIN(GPIOA, 5), NRF_SPI_MISO = GPIO_PIN(GPIOA, 6), NRF_SPI_MOSI =
GPIO_PIN(GPIOA, 7);

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

#endif /* PINS_H_ */
