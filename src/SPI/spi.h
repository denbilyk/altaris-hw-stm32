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

void SPI_init() {
	//GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	// Тактирование модуля SPI1 и порта А
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	pinMode(&NRF_SPI_MISO, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
	pinMode(&NRF_SPI_MOSI, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
	pinMode(&NRF_SPI_SCK, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);

	/*
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
*/
	//Заполняем структуру с параметрами SPI модуля
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //полный дуплекс
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // передаем по 8 бит
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; // Полярность и
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; // фаза тактового сигнала
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // Управлять состоянием сигнала NSS программно
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; // Предделитель SCK
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // Первым отправляется старший бит
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; // Режим - мастер
	SPI_Init(SPI1, &SPI_InitStructure); //Настраиваем SPI1
	SPI_Cmd(SPI1, ENABLE); // Включаем модуль SPI1....

	// Поскольку сигнал NSS контролируется программно, установим его в единицу
	// Если сбросить его в ноль, то наш SPI модуль подумает, что
	// у нас мультимастерная топология и его лишили полномочий мастера.
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
}

uint16_t spi_transfer(uint16_t data) {
	SPI_I2S_SendData(SPI1, data);
	do {} while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}

#endif /* SPI_SPI_H_ */
