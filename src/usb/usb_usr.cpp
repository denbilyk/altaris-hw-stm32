/*
 * usb_usr.c
 *
 *  Created on: Jan 23, 2016
 *      Author: denis.bilyk
 */

#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "../utils/flashv2.h"
#include "diag/Trace.h"

extern "C" {
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_usr.h"
#include "usb_desc.h"
}

extern uint8_t Buffer[RPT4_COUNT + 1];
extern uint8_t requestCommand;
extern uint8_t report_buf[wMaxPacketSize];
__IO uint8_t PrevXferComplete = 1;

static uint32_t flash_buffer_auth[AUTH_LEN / 4];
static uint32_t flash_buffer_host[HOST_LEN / 4];
static uint32_t flash_buffer_port[PORT_LEN / 4];
static uint32_t flash_buffer_ssid[SSID_LEN / 4];
static uint32_t flash_buffer_ssid_pass[SSID_PASS_LEN / 4];

void send_process() {
	if (bDeviceState == CONFIGURED && PrevXferComplete) {
		SendDataForRequest();
		requestCommand = 0;
	}
}

void fillBuffer(uint8_t* ptr_Buffer, uint32_t startBase, uint8_t len) {
	for (int8_t i = 0; i < len / 4; i++) {
		*ptr_Buffer++ = ((*(uint32_t*) (startBase + CELL * i)) >> 24 & 0xFF);
		*ptr_Buffer++ = ((*(uint32_t*) (startBase + CELL * i)) >> 16 & 0xFF);
		*ptr_Buffer++ = ((*(uint32_t*) (startBase + CELL * i)) >> 8 & 0xFF);
		*ptr_Buffer++ = ((*(uint32_t*) (startBase + CELL * i)) & 0xFF);
	}
}

void check_data_request() {
	uint8_t* ptr_Buffer;

	switch (requestCommand) {

	case CMD_READ_AUTH:
		ptr_Buffer = Buffer;
		memset(ptr_Buffer, 0, RPT4_COUNT + 1);
		*ptr_Buffer++ = 4;
		*ptr_Buffer++ = CMD_READ_AUTH;
		fillBuffer(ptr_Buffer, AUTH_BASE_ADDR, AUTH_LEN);
		ptr_Buffer += AUTH_LEN;
		*ptr_Buffer++ = 255;
		send_process();
		break;

	case CMD_READ_HOST:
		ptr_Buffer = Buffer;
		memset(ptr_Buffer, 0, RPT4_COUNT + 1);
		*ptr_Buffer++ = 4;
		*ptr_Buffer++ = CMD_READ_HOST;
		fillBuffer(ptr_Buffer, HOST_BASE_ADDR, HOST_LEN);
		ptr_Buffer += HOST_LEN;
		*ptr_Buffer++ = 254;
		fillBuffer(ptr_Buffer, PORT_BASE_ADDR, PORT_LEN);
		ptr_Buffer += PORT_LEN;
		*ptr_Buffer++ = 255;
		send_process();
		break;

	case CMD_READ_SSID:
		ptr_Buffer = Buffer;
		memset(ptr_Buffer, 0, RPT4_COUNT + 1);
		*ptr_Buffer++ = 4;
		*ptr_Buffer++ = CMD_READ_SSID;
		fillBuffer(ptr_Buffer, SSID_BASE_ADDR, SSID_LEN);
		ptr_Buffer += SSID_LEN;
		*ptr_Buffer++ = 255;
		send_process();
		break;

	case CMD_READ_SSID_PASS:
		ptr_Buffer = Buffer;
		memset(ptr_Buffer, 0, RPT4_COUNT + 1);
		*ptr_Buffer++ = 4;
		*ptr_Buffer++ = CMD_READ_SSID_PASS;
		fillBuffer(ptr_Buffer, SSID_PASS_BASE_ADDR, SSID_PASS_LEN);
		ptr_Buffer += SSID_PASS_LEN;
		*ptr_Buffer++ = 255;
		send_process();
		break;

	}
}

void check_usb_command() {
	uint8_t* ptr;
	switch (requestCommand) {

	case CMD_ERASE:
		erasePagev2();
		requestCommand = 0;
		break;

	case CMD_INIT_BUFFERS:
		memset(&flash_buffer_auth, 0, AUTH_LEN / 4);
		memset(&flash_buffer_host, 0, HOST_LEN / 4);
		memset(&flash_buffer_port, 0, PORT_LEN / 4);
		memset(&flash_buffer_ssid, 0, SSID_LEN / 4);
		memset(&flash_buffer_ssid_pass, 0, SSID_PASS_LEN / 4);
		requestCommand = 0;
		break;

	case CMD_WRITE_AUTH:
		ptr = report_buf;
		ptr += 2;
		for (int8_t i = 0; i < AUTH_LEN / 4; i++) {
			flash_buffer_auth[i] = *ptr++ << 24 | *ptr++ << 16 | *ptr++ << 8 | *ptr++;
		}
		requestCommand = 0;
		break;

	case CMD_WRITE_HOST:
		ptr = report_buf;
		ptr += 2;
		for (int8_t i = 0; i < HOST_LEN / 4; i++) {
			flash_buffer_host[i] = *ptr++ << 24 | *ptr++ << 16 | *ptr++ << 8 | *ptr++;
		}
		requestCommand = 0;
		break;

	case CMD_WRITE_PORT:
		ptr = report_buf;
		ptr += 2;
		for (int8_t i = 0; i < PORT_LEN / 4; i++) {
			flash_buffer_port[i] = *ptr++ << 24 | *ptr++ << 16 | *ptr++ << 8 | *ptr++;
		}
		requestCommand = 0;
		break;

	case CMD_WRITE_SSID:
		ptr = report_buf;
		ptr += 2;
		for (int8_t i = 0; i < SSID_LEN / 4; i++) {
			flash_buffer_ssid[i] = *ptr++ << 24 | *ptr++ << 16 | *ptr++ << 8 | *ptr++;
		}
		requestCommand = 0;
		break;

	case CMD_WRITE_SSID_PASS:
		ptr = report_buf;
		ptr += 2;
		for (int8_t i = 0; i < SSID_PASS_LEN / 4; i++) {
			flash_buffer_ssid_pass[i] = *ptr++ << 24 | *ptr++ << 16 | *ptr++ << 8 | *ptr++;
		}
		requestCommand = 0;
		break;

	case CMD_STORE_CONFIG:
		uint32_t* flash_buff_ptr;
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		FLASH_ErasePage(START_ADDRESS);

		flash_buff_ptr = flash_buffer_auth;
		for (int8_t i = 0; i < AUTH_LEN / 4; i++) {
			FLASH_ProgramWord(AUTH_BASE_ADDR + CELL * i, flash_buffer_auth[i]);
		}

		flash_buff_ptr = flash_buffer_host;
		for (int8_t i = 0; i < HOST_LEN / 4; i++) {
			FLASH_ProgramWord(HOST_BASE_ADDR + CELL * i, flash_buffer_host[i]);
		}

		flash_buff_ptr = flash_buffer_port;
		for (int8_t i = 0; i < PORT_LEN / 4; i++) {
			FLASH_ProgramWord(PORT_BASE_ADDR + CELL * i, flash_buffer_port[i]);
		}

		flash_buff_ptr = flash_buffer_ssid;
		for (int8_t i = 0; i < SSID_LEN / 4; i++) {
			FLASH_ProgramWord(SSID_BASE_ADDR + CELL * i, flash_buffer_ssid[i]);
		}

		flash_buff_ptr = flash_buffer_ssid_pass;
		for (int8_t i = 0; i < SSID_PASS_LEN / 4; i++) {
			FLASH_ProgramWord(SSID_PASS_BASE_ADDR + CELL * i, flash_buffer_ssid_pass[i]);
		}

		FLASH_Lock();

		requestCommand = 0;
		break;
	}
}

