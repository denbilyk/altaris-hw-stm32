/*
 * flashv2.h
 *
 *  Created on: Jan 28, 2016
 *      Author: denis.bilyk
 */

#ifndef UTILS_FLASHV2_H_
#define UTILS_FLASHV2_H_

#include "stm32f10x.h"


#define CELL 0x04
#define PAGE (CELL*4)				// 16 byte
#define START_ADDRESS 0x0801F000 //start address from 60KB (4kB)

#define AUTH_BASE_ADDR START_ADDRESS
#define SSID_BASE_ADDR (START_ADDRESS + PAGE*2)
#define SSID_PASS_BASE_ADDR (SSID_BASE_ADDR + PAGE * 2)
#define HOST_BASE_ADDR (SSID_PASS_BASE_ADDR + PAGE * 2)
#define PORT_BASE_ADDR (HOST_BASE_ADDR + PAGE)

#define AUTH_LEN	32
#define HOST_LEN	16
#define PORT_LEN	16
#define SSID_LEN	32
#define SSID_PASS_LEN	32


void fillCharBuffer(char* ptr, uint32_t startAddres, uint8_t len);
void erasePagev2();


#endif /* UTILS_FLASHV2_H_ */
