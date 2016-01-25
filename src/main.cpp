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
#include "utils/flash.h"

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
UART2 uart;
RF24 rf24(&NRF_CE, &NRF_CSN, &NRF_SPI_MISO, &NRF_SPI_MOSI, &NRF_SPI_SCK);
uint16_t received_data[6];
ESP esp;

String host = "";
String port = "";
String ssid = "";
String ssid_pass = "";
String auth = "";

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
	delay_ms(1000);
	digitalWrite(&LED_C13, LOW);
	delay_ms(1000);
}

int main() {

	//erasePage();
	//writeAuthKeyToFlash("AJKSJDKALS");
	//writeSsidToFlash("infinity");
	//writeSsidPassToFlash("0672086028");
	//writeHostToFlash("10.10.0.171");
	//writePortToFlash("8080");

	auth = readFromFlash(AUTH_BASE_ADDR, AUTH_LEN); //AJKSJDKALS
	host = readFromFlash(HOST_BASE_ADDR, HOST_LEN); //"10.10.0.171";
	port = readFromFlash(PORT_BASE_ADDR, PORT_LEN); //8080
	ssid = readFromFlash(SSID_BASE_ADDR, SSID_LEN); //"infinity";
	ssid_pass = readFromFlash(SSID_PASS_BASE_ADDR, SSID_PASS_LEN); //0672086028

	trace_printf("Auth: %s\n", auth.c_str());
	trace_printf("Ssid: %s\n", ssid.c_str());
	trace_printf("Ssid Pass: %s\n", ssid_pass.c_str());
	trace_printf("Host: %s\n", host.c_str());
	trace_printf("Port: %s\n", port.c_str());

	pinMode(&LED_C13, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	delay_init();
	esp_port.begin(115200);
	uart.begin(115200);
	uart.println("Test UART");

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

		check_data_request(auth.c_str(), host.c_str(), port.c_str(), ssid.c_str(), ssid_pass.c_str());
		check_auth_write_command();

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
		if (uart.available()) {
			String s = String(uart.readString());
			trace_printf("ReadString: %s", s.c_str());
			uart.println(s.c_str());
			uart.flush();
		}
		if (rf24.available()) {
			bool done = false;
			raw_data = "";
			send_tries = -1;
			while (!done) {
				done = rf24.read(&received_data, sizeof(received_data));
			}
			trace_puts("> ");
			for (uint8_t i = 0; i < 6; i++) {
				trace_printf("received_data[%u] = %d \r\n", i, received_data[i]);
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
			}
		}

	}
	return 0;
}

#pragma GCC diagnostic pop
