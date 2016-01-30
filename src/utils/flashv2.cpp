/*
 * flashv2.cpp
 *
 *  Created on: Jan 28, 2016
 *      Author: denis.bilyk
 */

#include "flashv2.h"

void fillCharBuffer(char* ptr, uint32_t startAddres, uint8_t len) {
	for (int8_t i = 0; i < len / 4; i++) {
		*ptr++ = (char) ((*(uint32_t*) (startAddres + CELL * i)) >> 24 & 0xFF);
		*ptr++ = (char) ((*(uint32_t*) (startAddres + CELL * i)) >> 16 & 0xFF);
		*ptr++ = (char) ((*(uint32_t*) (startAddres + CELL * i)) >> 8 & 0xFF);
		*ptr++ = (char) ((*(uint32_t*) (startAddres + CELL * i)) & 0xFF);
	}
}

void erasePagev2() {
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(START_ADDRESS);
	FLASH_Lock();
}
