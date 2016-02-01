/*
 * esp.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: denis.bilyk
 */

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifdef TRACE
#include "diag/Trace.h"
#endif
#include "esp.h"
#include "uart/uart.h"
#include "utils/delay.h"
#include "utils/flashv2.h"

extern UART1 esp_port;

char *trim(char *str) {
	char *end;

	// Trim leading space
	while (isspace(*str))
		str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;

	// Write new null terminator
	*(end + 1) = 0;

	return str;
}

bool cmd(const char* cmd, uint16_t delay) {
	bool res = false;
	esp_port.println(cmd);
	delay_ms(delay);
	const char* resp = esp_port.readString();
#ifdef TRACE
	trace_printf("Response: %s \r\n", resp);
#endif
	if (strstr(resp, "ERROR") == 0) {
		res = true;
	}
	return res;
}

bool esp_setMuxMode() {
	if (!cmd("AT+CIPMUX=0", 300)) {
		return false;
	}
	if (!cmd("AT+CWMODE=1", 300)) {
		return false;
	}
	if (!cmd("AT+CIPMUX=1", 300)) {
		return false;
	}
	return true;
}

bool r_check() {
	bool res = false;
	esp_port.println("AT+CIPSTA?");
	delay_ms(500);
	char* resp = (char*) esp_port.readString();
#ifdef TRACE
	trace_printf("Response: %s \r\n", trim(resp));
#endif
	if (strstr(resp, "+CIPSTA:") != 0) {
		res = true;
	}
	return res;
}

bool esp_check() {
	uint8_t counter = 5;
	bool res = false;
	do {
		res = r_check();
		counter--;
		if (res)
			break;
	} while (counter != 0);
	return res;
}

bool esp_init() {
	if (!cmd("AT+RST", 300))
		return false;
	if (!cmd("AT", 300))
		return false;
	if (!cmd("AT+CWMODE=1", 300))
		return false;

	char ssid[SSID_LEN];
	char ssid_pass[SSID_PASS_LEN];
	char* ptr;
	ptr = ssid;
	fillCharBuffer(ptr, SSID_BASE_ADDR, SSID_LEN);
	ptr = ssid_pass;
	fillCharBuffer(ptr, SSID_PASS_BASE_ADDR, SSID_PASS_LEN);
	char cmd_jap[11 + strlen(ssid) + 5 + strlen(ssid_pass) + 5];
	strcpy(cmd_jap, "AT+CWJAP=\"");
	strcat(cmd_jap, ssid);
	strcat(cmd_jap, "\",\"");
	strcat(cmd_jap, ssid_pass);
	strcat(cmd_jap, "\"");
	if (!cmd(cmd_jap, 7000))
		return false;

	if (!cmd("AT+CIPSTA?", 200))
		return false;
	if (!esp_setMuxMode())
		return false;

	return true;
}

bool esp_connect_to_host() {
	bool is_connected = false;
	char host[HOST_LEN];
	char port[PORT_LEN];
	char* ptr;
	ptr = host;
	fillCharBuffer(ptr, HOST_BASE_ADDR, HOST_LEN);
	ptr = port;
	fillCharBuffer(ptr, PORT_BASE_ADDR, PORT_LEN);

	char tcp_host[50];
	memset(&tcp_host, 0, 50);
	strcpy(tcp_host, "AT+CIPSTART=0,\"TCP\",\"");
	strcat(tcp_host, host);
	strcat(tcp_host, "\",");
	strcat(tcp_host, port);
	esp_port.println(tcp_host);
	delay_ms(300);
	const char *resp = esp_port.readString();
	if (strstr(resp, "CONNECT") != 0) {
#ifdef TRACE
		trace_puts("Connected!\n");
#endif
		is_connected = true;
	} else {
#ifdef TRACE
		trace_printf("Err Res - %s\n", resp);
#endif
	}
	return is_connected;
}

bool sendTcp(const char* tcp_packet) {
	bool res = false;
	if (esp_connect_to_host()) {
#ifdef TRACE
		trace_printf("Tcp paket - %s\n", tcp_packet);
#endif
		char* cmd_send = (char*) malloc(strlen(tcp_packet) + 13);
		strcpy(cmd_send, "AT+CIPSEND=0,");
		char len[3];
		char* ptr = len;
		itoa(strlen(tcp_packet), ptr, 10);
		strcat(cmd_send, len);
		esp_port.println(cmd_send);
		delay_ms(300);
		const char* resp = esp_port.readString();

		if (strstr(resp, ">") != 0) {
			esp_port.print(tcp_packet);
#ifdef TRACE
			trace_puts("Packet sent\n");
#endif
			res = true;
		} else {
			delay_ms(500);
			esp_port.println("AT+CIPCLOSE=0");
#ifdef TRACE
			trace_printf("Close - %s", resp);
#endif
		}
	}
	return res;
}

bool esp_send_data(const char* raw_data) {
	char auth[AUTH_LEN];
	char* ptr;
	ptr = auth;
	fillCharBuffer(ptr, AUTH_BASE_ADDR, AUTH_LEN);
	char* g = (char*) malloc(18 + strlen(auth) + 5 + strlen(raw_data) + 2);
	memset(g, 0, sizeof(g));
	strcpy(g, "GET /api/raw?auth=");
	strcat(g, auth);
	strcat(g, "&raw=");
	strcat(g, raw_data);
	strcat(g, "\r\n");
	return sendTcp(g);
}

