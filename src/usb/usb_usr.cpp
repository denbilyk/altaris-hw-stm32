/*
 * usb_usr.c
 *
 *  Created on: Jan 23, 2016
 *      Author: denis.bilyk
 */

#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "../utils/flash.h"
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

void send_process() {
	if (bDeviceState == CONFIGURED && PrevXferComplete) {
		SendDataForRequest();
		requestCommand = 0;
	}
}

void prepareBufferWithAuth(const char* auth) {
	uint8_t* ptr;
	ptr = Buffer;
	memset(ptr, 0, 64);
	*ptr++ = 4;
	*ptr++ = CMD_READ_AUTH;
	memcpy(ptr, auth, strlen(auth));
	ptr += 32;
	*ptr++ = 255;
}

void prepareBufferWithHost(const char* host, const char* port) {
	uint8_t* ptr;
	ptr = Buffer;
	memset(ptr, 0, 64);
	*ptr++ = 4;
	*ptr++ = CMD_READ_HOST;
	memcpy(ptr, host, strlen(host));
	ptr += 16;
	*ptr++ = 254;
	memcpy(ptr, port, strlen(port));
	ptr += 8;
	*ptr++ = 255;
}

void prepareBufferWithSsid(const char* ssid) {
	uint8_t* ptr;
	ptr = Buffer;
	memset(ptr, 0, 64);
	*ptr++ = 4;
	*ptr++ = CMD_READ_SSID;
	memcpy(ptr, ssid, strlen(ssid));
	ptr += 32;
	*ptr++ = 255;
}

void prepareBufferWithSsidPass(const char* ssid_pass) {
	uint8_t* ptr;
	ptr = Buffer;
	memset(ptr, 0, 64);
	*ptr++ = 4;
	*ptr++ = CMD_READ_SSID_PASS;
	memcpy(ptr, ssid_pass, strlen(ssid_pass));
	ptr += 32;
	*ptr++ = 255;
}

void check_data_request(const char* auth, const char* host, const char* port, const char* ssid, const char* ssid_pass) {

	switch (requestCommand) {

	case CMD_READ_AUTH:
		prepareBufferWithAuth(auth);
		send_process();
		break;

	case CMD_READ_HOST:
		prepareBufferWithHost(host, port);
		send_process();
		break;

	case CMD_READ_SSID:
		prepareBufferWithSsid(ssid);
		send_process();
		break;

	case CMD_READ_SSID_PASS:
		prepareBufferWithSsidPass(ssid_pass);
		send_process();
		break;

	}
}

void check_auth_write_command() {
	if(requestCommand == CMD_ERASE) {
		erasePage();
		trace_printf("Erase command\r\n");
		requestCommand = 0;
	}

	if (requestCommand == CMD_WRITE_AUTH) {
		uint8_t b[32];
		memset(&b, 0, 32);
		uint8_t* ptr;
		ptr = report_buf;
		ptr += 2;
		memcpy(&b, ptr, 32);
		const char *p = reinterpret_cast<const char*>(b);
		trace_printf("New auth: %s \r\n", p);
		writeAuthKeyToFlash(p);
		requestCommand = 0;
	}

}

