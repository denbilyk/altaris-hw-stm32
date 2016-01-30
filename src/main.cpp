#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f10x.h"

#include "main_config.h"
#include "uart/uart.h"
#include "utils/WString.h"
#include "utils/delay.h"
#include "pins.h"
#include "rf24/rf24.h"
#include "esp-network.h"
#include "utils/flashv2.h"

/********************* USB ***********************/
extern "C" {
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_usr.h"
#include "usb_desc.h"
}
/*************************************************/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

UART1 esp_port;
RF24 rf24(&NRF_CE, &NRF_CSN, &NRF_SPI_MISO, &NRF_SPI_MOSI, &NRF_SPI_SCK);
uint16_t received_data[6];
ESP esp;

static char auth[AUTH_LEN];
static char host[HOST_LEN];
static char port[PORT_LEN];
static char ssid[SSID_LEN];
static char ssid_pass[SSID_PASS_LEN];

#define RESEND_TRIES 3

/* Private variables ---------------------------------------------------------*/
__IO uint8_t send_tries = -1;
String raw_data = "";

void init_usb() {
	Set_System();
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();
	trace_printf("USB startup.");
}

void init_rf24() {
	rf24.begin();
	rf24.setRetries(15, 15);
	rf24.setAutoAck(true);
	rf24.setCRCLength(RF24_CRC_8);
	rf24.setDataRate(RF24_250KBPS);

	rf24.openReadingPipe(0, RX_ADDR_0);
	rf24.openReadingPipe(1, RX_ADDR_1);
	rf24.openReadingPipe(2, RX_ADDR_2);
	rf24.openReadingPipe(3, RX_ADDR_3);
	rf24.openReadingPipe(4, RX_ADDR_4);
	rf24.openReadingPipe(5, RX_ADDR_5);
	rf24.startListening();
	rf24.printDetails();
}

void toggleLed() {
	digitalWrite(&LED_C13, HIGH);
	delay_ms(200);
	digitalWrite(&LED_C13, LOW);
	delay_ms(200);
}

int main() {

	//erasePage();
	//writeAuthKeyToFlash("AJKSJDKALS");
	//writeSsidToFlash("infinity");
	//writeSsidPassToFlash("0672086028");
	//writeHostToFlash("10.10.0.171");
	//writePortToFlash("8080");

	char* ptr;
	ptr = auth;
	fillCharBuffer(ptr, AUTH_BASE_ADDR, AUTH_LEN);
	ptr = host;
	fillCharBuffer(ptr, HOST_BASE_ADDR, HOST_LEN);
	ptr = port;
	fillCharBuffer(ptr, PORT_BASE_ADDR, PORT_LEN);
	ptr = ssid;
	fillCharBuffer(ptr, SSID_BASE_ADDR, SSID_LEN);
	ptr = ssid_pass;
	fillCharBuffer(ptr, SSID_PASS_BASE_ADDR, SSID_PASS_LEN);

	trace_printf("Auth: %s\n", auth);
	trace_printf("Host: %s\n", host);
	trace_printf("Port: %s\n", port);
	trace_printf("Ssid: %s\n", ssid);
	trace_printf("Ssid Pass: %s\n", ssid_pass);

	pinMode(&LED_C13, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	delay_init();
	esp_port.begin(115200);

	init_usb();
	init_rf24();

	if (!esp.esp_check())
		esp.esp_init(ssid, ssid_pass);
	else
		esp.setMuxMode();

	uint32_t usbLastState;
	while (1) {
		if (bDeviceState != usbLastState) {
			usbLastState = bDeviceState;
			trace_printf("USB Changed state to %s \r\n", get_usb_state_name(bDeviceState));
		}

		check_data_request();
		check_usb_command();

		/*
		 if (uart.available()) {
		 esp_port.print(uart.readString());
		 uart.flush();
		 }
		 if (esp_port.available()) {
		 uart.print(esp_port.readString());
		 esp_port.flush();
		 }
		 */
		//toggleLed();
		if (rf24.available()) {
			bool done = false;
			raw_data = "";
			send_tries = -1;
			while (!done) {
				done = rf24.read(&received_data, sizeof(received_data));
			}
			trace_puts("> ");
			for (uint8_t i = 0; i < 6; i++) {
				trace_printf("RF21-1: received_data[%u] = %d \r\n", i, received_data[i]);
			}

			for (uint8_t i = 0; i < sizeof(received_data) / sizeof(uint16_t); i++) {
				raw_data.concat(received_data[i]);
				raw_data.concat(";");
			}
			send_tries = 1;
		}

		if (send_tries > 0 && send_tries <= RESEND_TRIES) {
			if (!esp.send_data(host, port, auth, raw_data)) {
				send_tries++;
			} else {
				send_tries = -1;
				toggleLed();
			}
		}

	}
	return 0;
}

#pragma GCC diagnostic pop
