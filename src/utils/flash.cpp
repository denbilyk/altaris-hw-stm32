/*
 * flash.cpp
 *
 *  Created on: Jan 17, 2016
 *      Author: denis.bilyk
 */

#include "flash.h"

static char buff[32];
static uint32_t buff32[8];

/************************************************************************************/

void readRaw(uint32_t* buff, uint8_t size, uint32_t startAddr) {
	//We should read 4*8bytes
	//uint32_t buff[8];
	memset(buff, 0, size);
	uint32_t* ptr;
	ptr = buff;

	for (int8_t i = 0; i < size / 4; i++) {
		*ptr = *(uint32_t*) (startAddr + CELL * i);
		ptr++;
	}
}

/************************************************************************************/

void programBlock(uint32_t* buff, uint8_t size, uint32_t startAddr) {
	for (int8_t i = 0; i < size / 4; i++) {
		FLASH_ProgramWord(startAddr + CELL * i, buff[i]);
	}
}

/************************************************************************************/

const char* convertToString(uint32_t* buff, uint8_t size) {
	String s = "";
	for (int8_t i = 0; i < size / 4; i++) {
		s.concat((char) (buff[i] >> 24 & 0xFF));
		s.concat((char) (buff[i] >> 16 & 0xFF));
		s.concat((char) (buff[i] >> 8 & 0xFF));
		s.concat((char) (buff[i] & 0xFF));
	}
	return s.c_str();
}

/************************************************************************************/
uint32_t* convertToUint32(char* buff) {
	memset(&buff32, 0, 32);
	for (int8_t i = 0; i < 32; i = i + 4) {
		buff32[i / 4] = buff[i] << 24 | buff[i + 1] << 16 | buff[i + 2] << 8 | buff[i + 3];
	}
	return buff32;
}

/************************************************************************************/

char* allocStrBuf(String str) {
	memset(&buff, 0, 32);
	str.toCharArray(buff, 32, 0);
	return buff;
}

/************************************************************************************/

void writeAuthKeyToFlash(String auth_key) {
	char* buff = allocStrBuf(auth_key);

	uint32_t ssid[8];
	readRaw(ssid, 32, SSID_BASE_ADDR);
	uint32_t ssid_pass[8];
	readRaw(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	uint32_t host[4];
	readRaw(host, 16, HOST_BASE_ADDR);
	uint32_t port[4];
	readRaw(port, 4, PORT_BASE_ADDR);

	uint32_t * auth = convertToUint32(buff);

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(AUTH_BASE_ADDR);

	programBlock(auth, 32, AUTH_BASE_ADDR);
	programBlock(ssid, 32, SSID_BASE_ADDR);
	programBlock(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	programBlock(host, 16, HOST_BASE_ADDR);
	programBlock(port, 4, PORT_BASE_ADDR);

	FLASH_Lock();
}

/************************************************************************************/

void writeHostToFlash(String host) {
	//Should write 16 byte. End address - host_base_addr + cell * 4
	char* buff = allocStrBuf(host);

	uint32_t auth[8];
	readRaw(auth, 32, AUTH_BASE_ADDR);
	uint32_t ssid[8];
	readRaw(ssid, 32, SSID_BASE_ADDR);
	uint32_t ssid_pass[8];
	readRaw(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	uint32_t port[4];
	readRaw(port, 4, PORT_BASE_ADDR);

	uint32_t * host32 = convertToUint32(buff);

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(HOST_BASE_ADDR);

	programBlock(auth, 32, AUTH_BASE_ADDR);
	programBlock(ssid, 32, SSID_BASE_ADDR);
	programBlock(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	programBlock(host32, 16, HOST_BASE_ADDR);
	programBlock(port, 4, PORT_BASE_ADDR);

	FLASH_Lock();
}

/************************************************************************************/

void writePortToFlash(String port) {
	char* buff = allocStrBuf(port);

	uint32_t auth[8];
	readRaw(auth, 32, AUTH_BASE_ADDR);
	uint32_t ssid[8];
	readRaw(ssid, 32, SSID_BASE_ADDR);
	uint32_t ssid_pass[8];
	readRaw(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	uint32_t host[4];
	readRaw(host, 16, HOST_BASE_ADDR);

	uint32_t * port32 = convertToUint32(buff);

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(PORT_BASE_ADDR);

	programBlock(port32, 8, PORT_BASE_ADDR);
	programBlock(auth, 32, AUTH_BASE_ADDR);
	programBlock(ssid, 32, SSID_BASE_ADDR);
	programBlock(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	programBlock(host, 16, HOST_BASE_ADDR);

	FLASH_Lock();

}

/************************************************************************************/

void writeSsidToFlash(String ssid) {
	char* buff = allocStrBuf(ssid);

	uint32_t auth[8];
	readRaw(auth, 32, AUTH_BASE_ADDR);
	uint32_t ssid_pass[8];
	readRaw(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	uint32_t host[4];
	readRaw(host, 16, HOST_BASE_ADDR);
	uint32_t port[4];
	readRaw(port, 4, PORT_BASE_ADDR);

	uint32_t *ssid32 = convertToUint32(buff);

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(SSID_BASE_ADDR);

	programBlock(auth, 32, AUTH_BASE_ADDR);
	programBlock(ssid32, 32, SSID_BASE_ADDR);
	programBlock(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	programBlock(host, 16, HOST_BASE_ADDR);
	programBlock(port, 4, PORT_BASE_ADDR);

	FLASH_Lock();

}

/************************************************************************************/

void writeSsidPassToFlash(String pass) {
	char* buff = allocStrBuf(pass);

	uint32_t auth[8];
	readRaw(auth, 32, AUTH_BASE_ADDR);
	uint32_t ssid[8];
	readRaw(ssid, 32, SSID_BASE_ADDR);
	uint32_t host[4];
	readRaw(host, 16, HOST_BASE_ADDR);
	uint32_t port[4];
	readRaw(port, 4, PORT_BASE_ADDR);

	uint32_t * ssid_pass = convertToUint32(buff);

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(SSID_PASS_BASE_ADDR);

	programBlock(auth, 32, AUTH_BASE_ADDR);
	programBlock(ssid, 32, SSID_BASE_ADDR);
	programBlock(ssid_pass, 32, SSID_PASS_BASE_ADDR);
	programBlock(host, 16, HOST_BASE_ADDR);
	programBlock(port, 4, PORT_BASE_ADDR);

	FLASH_Lock();
}

/************************************************************************************/

const char* readFromFlash(uint32_t addr, uint8_t size) {
	uint32_t buff32[size / 4];
	readRaw(buff32, size, addr);
	return convertToString(buff32, size);
}

/************************************************************************************/

void erasePage() {
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(START_ADDRESS);
	FLASH_Lock();
}

