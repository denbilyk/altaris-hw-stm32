/*
 * delay.h
 *
 *  Created on: Jan 14, 2016
 *      Author: denis.bilyk
 */

#ifndef UTILS_DELAY_H_
#define UTILS_DELAY_H_

#include <stdint.h>
#include <stm32f10x.h>

void delay_init();

void delay_ms(volatile uint32_t ms);

#endif /* UTILS_DELAY_H_ */
