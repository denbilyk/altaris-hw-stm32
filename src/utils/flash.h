/*
 * flash.h
 *
 *  Created on: Jan 17, 2016
 *      Author: denis.bilyk
 */

#ifndef __FLASH_H_
#define __FLASH_H_

#include "stm32f10x.h"
#include "WString.h"

/*
 uint8_t cell = 0x04;
 uint8_t page = cell * 4;
 uint32_t startAddress = 0x801F000; //start address from 60KB (4kB)
 uint32_t host_base_addr = startAddress;
 uint32_t ssid_base_addr = startAddress + page;
 */

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
#define PORT_LEN	8
#define SSID_LEN	32
#define SSID_PASS_LEN	32

void writeAuthKeyToFlash(String auth_key);  //32 byte

void writeSsidToFlash(String ssid);			//32 byte
void writeSsidPassToFlash(String pass);		//32 byte

void writeHostToFlash(String host);			//32 byte
void writePortToFlash(String port);		// 2 byte

const char* readFromFlash(uint32_t addr, uint8_t size);

void erasePage();

#endif /* UTILS_FLASH_H_ */
