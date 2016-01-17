/*
 * main.config.h
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

#include "pins.h"

RCC_Periphery GPIOA_RCC = RCC_PERIPHERY(APB2, GPIOA);
RCC_Periphery GPIOB_RCC = RCC_PERIPHERY(APB2, GPIOB);
RCC_Periphery GPIOC_RCC = RCC_PERIPHERY(APB2, GPIOC);
RCC_Periphery GPIOD_RCC = RCC_PERIPHERY(APB2, GPIOD);


GPIO_Pin LED_C13 = GPIO_PIN(GPIOC, 13);
GPIO_Pin NRF_CSN = GPIO_PIN(GPIOA, 4);
GPIO_Pin NRF_SPI_SCK = GPIO_PIN(GPIOA, 5);
GPIO_Pin NRF_SPI_MISO = GPIO_PIN(GPIOA, 6);
GPIO_Pin NRF_SPI_MOSI =	GPIO_PIN(GPIOA, 7);
GPIO_Pin NRF_CE = GPIO_PIN(GPIOB, 0);



#endif /* MAIN_CONFIG_H_ */