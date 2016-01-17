//
// Created by Denis Bilyk on 10/1/15.
//

#include "esp-network.h"

extern UART1 esp_port;

bool ESP::send_AT() {
	return cmd(ESP_AT, 300);
}

bool ESP::send_RST() {
	return cmd(ESP_RST, 300);
}

bool ESP::send_CWMODE() {
	return cmd(ESP_CWMODE, 300);
}

bool ESP::send_CWJAP(String ssid, String pass) {
	String command = String(ESP_CWJAP);
	command.replace("{0}", ssid);
	command.replace("{1}", pass);
	return cmd(command, 7000);
}

String ESP::send_CIPSTA() {
	esp_port.println(ESP_CIPSTA);
	delay_ms(200);
	String res = String(esp_port.readString());
	trace_printf("CIPSTA: %s\r\n", res.c_str());
	esp_port.flush();
	return res;
}

bool ESP::send_CIPMUX(uint8_t value) {
	return value == 0 ? cmd(ESP_CIPMUX_0, 300) : cmd(ESP_CIPMUX_1, 300);
}

bool ESP::r_check(uint8_t counter) {
	String s = send_CIPSTA();
	trace_printf("Response: %s", s.c_str());
	return s.indexOf("+CIPSTA:") == -1 ? false : true;
}

bool ESP::esp_check() {
	uint8_t counter = 5;
	bool res = false;
	do {
		res = ESP::r_check(counter);
		counter--;
		if (res)
			break;
	} while (counter != 0);
	return res;
}

bool ESP::esp_init(String ssid, String ssid_pass) {
	trace_puts("Start init ESP...");
	if (!send_RST()) {
		trace_puts("RST - Failed");
		return false;
	}
	if (!send_AT()) {
		trace_puts("AT - Failed");
		return false;
	}
	if (!send_CWMODE()) {
		trace_puts("CWMODE - Failed");
		return false;
	}
	if (!send_CWJAP(ssid, ssid_pass)) {
		trace_puts("CWJAP - Failed");
		return false;
	}
	const char* s = send_CIPSTA().c_str();
	trace_puts(s);
	if (!send_CIPMUX(0)) {
		trace_puts("CIPMUX=0 = Failed");
		return false;
	}
	if (!send_CWMODE()) {
		trace_puts("CWMODE - Last Failed");
		return false;
	}
	if (!send_CIPMUX(1)) {
		trace_puts("CIPMUX=1 = Failed");
		return false;
	}
	trace_puts("ESP init complete");
	return true;
}

bool connect_to_host(String host, String port) {
	uint8_t counter = 10;
	bool is_connected = false;
	do {
		String tcp_host = String(CMD_TCP_HOST);
		tcp_host.replace("{0}", host);
		tcp_host.replace("{1}", port);
		esp_port.println(tcp_host);
		delay_ms(300);
		const char *resp = esp_port.readString();
		if (strstr(resp, "CONNECT") != 0) {
			trace_puts("Connected!\n");
			is_connected = true;
			esp_port.flush();
			break;
		} else {
			trace_printf("Err Res - %s\n", resp);
		}
		delay_ms(1000);
		counter--;
	} while (counter > 0);

	return is_connected;
}

bool sendTcp(String host, String port, String tcp_packet) {
	bool res = false;
	if (connect_to_host(host, port)) {
		trace_printf("Tcp paket - %s\n", tcp_packet.c_str());

		String cmd_send = String(ESP_CIPSEND);
		cmd_send.replace("{0}", String(tcp_packet.length()));
		esp_port.println(cmd_send);
		delay_ms(300);
		String resp = String(esp_port.readString());
		esp_port.flush();
		if (resp.indexOf(">") != -1) {
			esp_port.print(tcp_packet);
			trace_puts("Packet sent\n");
			res = true;
		} else {
			delay_ms(500);
			esp_port.println("AT+CIPCLOSE=0");
			trace_printf("Close - %s", resp.c_str());
		}
	}
	return res;
}

bool ESP::send_data(String host, String port, String auth, String id, String temp, String humid) {
	String packet = String(GET_REQUEST);
	packet.replace("{0}", auth);
	packet.replace("{1}", id);
	packet.replace("{2}", temp);
	packet.replace("{3}", humid);
	return sendTcp(host, port, packet);
}

bool ESP::send_data(String host, String port, String auth, String raw) {
	String packet = String(GET_RAW_REQUEST);
	packet.replace("{0}", auth);
	packet.replace("{1}", raw);
	return sendTcp(host, port, packet);
}

bool ESP::cmd(String cmd, uint16_t delay) {
	trace_printf("Command: %s \r\n", cmd.c_str());
	esp_port.println(cmd);
	delay_ms(delay);
	String resp = esp_port.readString();
	esp_port.flush();
	trace_printf("Response: %s", resp.c_str());
	return resp.indexOf(ESP_RESP_ERROR) == -1;
}
