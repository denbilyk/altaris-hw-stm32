#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include "string.h"
#include "uart.h"

//uint_fast8_t buff[RX_BUFF_SIZE];
static char rx_buffer[RX_BUFF_SIZE];
uint8_t pointer = 0;
bool rx_ready = false;

void UART::begin(uint32_t baudrate) {
	GPIO_InitTypeDef PORT;

	// U(S)ART init
#if _UART_PORT == 1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,
			ENABLE);
#elif _UART_PORT == 2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
#elif _UART_PORT == 3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
#elif _UART_PORT == 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
#elif _UART_PORT == 5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);
#endif

	PORT.GPIO_Pin = UART_TX_PIN;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Mode = GPIO_Mode_AF_PP; // TX as AF with Push-Pull
	GPIO_Init(UART_GPIO_PORT_TX, &PORT);
	PORT.GPIO_Pin = UART_RX_PIN;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Mode = GPIO_Mode_IN_FLOATING; // RX as in without pull-up
	GPIO_Init(UART_GPIO_PORT_RX, &PORT);

	USART_InitTypeDef UART;
	UART.USART_BaudRate = baudrate;
	UART.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // No flow control
	UART.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // RX+TX mode
	UART.USART_Parity = USART_Parity_No; // No parity check
	UART.USART_StopBits = USART_StopBits_1; // 1 stop bit
	UART.USART_WordLength = USART_WordLength_8b; // 8-bit frame
	USART_Init(UART_PORT, &UART);
	USART_Cmd(UART_PORT, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = UART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(UART_PORT, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART_PORT, USART_IT_IDLE, ENABLE);

}

void UART::write_char(char ch) {
	while (!USART_GetFlagStatus(UART_PORT, USART_FLAG_TC))
		; // wait for "Transmission Complete" flag cleared
	USART_SendData(UART_PORT, ch);
}

size_t UART::write_to_uart(uint8_t buff) {
	UART::write_char(buff);
	return 0;
}

void UART::flush() {
	memset(rx_buffer, 0, RX_BUFF_SIZE);
	pointer = 0;
	rx_ready = false;
}

bool UART::available() {
	return rx_ready;
}

const char *UART::readString() {
	rx_ready = false;
	return rx_buffer;
}

#ifdef __cplusplus
extern "C" {
#endif

void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		if (pointer > RX_BUFF_SIZE) {
			memset(rx_buffer, 0, RX_BUFF_SIZE);
			pointer = 0;
			rx_ready = true;
		}
		rx_buffer[pointer++] = USART_ReceiveData(UART_PORT);
		USART_ClearITPendingBit(UART_PORT, USART_IT_RXNE);
	}
	if (USART_GetITStatus(UART_PORT, USART_IT_IDLE) != RESET) {
		rx_ready = true;
		USART_ReceiveData(UART_PORT);
	}
}

#ifdef __cplusplus
}
#endif

