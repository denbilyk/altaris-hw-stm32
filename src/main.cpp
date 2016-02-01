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
#include "esp.h"
#include "utils/flashv2.h"
#include "exti.h"

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

#define RX_PAYLOAD 6

UART1 esp_port;
RF24 rf24(&NRF_CE, &NRF_CSN, &NRF_SPI_MISO, &NRF_SPI_MOSI, &NRF_SPI_SCK);
extern uint16_t received_data[RX_PAYLOAD];
static char raw[RX_PAYLOAD * 5 + RX_PAYLOAD];
__IO int8_t to_send = 0;
__IO bool lastSend = false;
extern uint8_t have_data;
uint32_t usbLastState;

/* Private variables ---------------------------------------------------------*/
__IO uint8_t send_tries = -1;
String raw_data = "";

void init_usb() {
	Set_System();
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();
	trace_printf("USB startup. \n");
}

void init_rf24() {
	have_data = 0;
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

void toggleLastSend(bool status) {
	if (status) {
		trace_puts("ESP last send - ok \n");
		digitalWrite(&LED_ESP_STATUS_ERR, LOW);
		digitalWrite(&LED_ESP_STATUS_OK, HIGH);
	} else {
		trace_puts("ESP last send - fail \n");
		digitalWrite(&LED_ESP_STATUS_OK, LOW);
		digitalWrite(&LED_ESP_STATUS_ERR, HIGH);
	}
}

void toggleEspInit(bool status) {
	if (status) {
		trace_puts("ESP init - ok \n");
		digitalWrite(&LED_ESP_INIT_ERR, LOW);
		digitalWrite(&LED_ESP_INIT_OK, HIGH);
	} else {
		trace_puts("ESP init - fail \n");
		digitalWrite(&LED_ESP_INIT_OK, LOW);
		digitalWrite(&LED_ESP_INIT_ERR, HIGH);
	}
}

void setup() {
	esp_port.begin(115200);
	pinMode(&LED_ESP_STATUS_OK, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	pinMode(&LED_ESP_STATUS_ERR, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	pinMode(&LED_ESP_INIT_OK, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	pinMode(&LED_ESP_INIT_ERR, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);
	pinMode(&LED_C13, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);

	exti10_init();
	init_usb();
	init_rf24();

	bool espInit = false;
	if (!esp_check())
		espInit = esp_init();
	else
		espInit = esp_setMuxMode();
	toggleEspInit(espInit);

}
void loop() {
	if (bDeviceState != usbLastState) {
		usbLastState = bDeviceState;

		trace_printf("USB Changed state to %s \r\n", get_usb_state_name(bDeviceState));

	}
	check_data_request();
	check_usb_command();

	toggleLed();

	if ("CONFIGURED" != get_usb_state_name(bDeviceState)) {
		if (have_data) {
			char* raw_ptr = raw;
			memset(raw_ptr, 0, RX_PAYLOAD * 5 + RX_PAYLOAD);
			have_data = 0;
			size_t len = 0;

			trace_puts("Process data > \r\n");

			for (uint8_t i = 0; i < RX_PAYLOAD; i++) {

				trace_printf("RF21: received_data[%u] = %d \r\n", i, received_data[i]);

				sprintf(raw_ptr, "%d", received_data[i]);
				raw_ptr += strlen(raw) - len;
				*raw_ptr++ = ';';
				len = strlen(raw);
			}
			to_send = 3;
		}

		if (to_send > 0) {
			if (esp_send_data(raw)) {
				to_send = 0;
				toggleLastSend(true);
			} else {
				to_send--;
				toggleLastSend(false);
			}
		}
	}

}

/*
 *
 */

int main() {
	delay_init();
	setup();

	while (1) {
		loop();
	}
	return 0;
}

#pragma GCC diagnostic pop
