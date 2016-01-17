/*
 * spi.h
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#ifndef SPI_SPI_H_
#define SPI_SPI_H_

#include "stm32f10x.h"
#include "pins.h"

uint16_t spi_transfer(uint16_t data);

void spi_init(GPIO_Pin* miso, GPIO_Pin* mosi, GPIO_Pin* sck);

#endif /* SPI_SPI_H_ */
