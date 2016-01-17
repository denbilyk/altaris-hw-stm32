#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include "string.h"
#include "uart.h"

static char uart1_rx_buffer[RX_BUFF_SIZE];
uint16_t uart1_pointer = 0;
bool uart1_rx_ready = false;

static char uart2_rx_buffer[RX_BUFF_SIZE];
uint16_t uart2_pointer = 0;
bool uart2_rx_ready = false;

uart_cfg uart1_cfg;
uart_cfg uart2_cfg;

/******************************************    UART1   **************************************************/

UART1::UART1() {
	uart1_cfg.port_no = 1;
	uart1_cfg.uart_port = USART1;
	uart1_cfg.uart_gpio_port_tx = GPIOA;
	uart1_cfg.uart_gpio_port_rx = GPIOA;
	uart1_cfg.rx_pin = GPIO_Pin_10;   // PA10 (USART1_RX)
	uart1_cfg.tx_pin = GPIO_Pin_9; // PA9  (USART1_TX)
	uart1_cfg.irq = USART1_IRQn;
}

void UART1::begin(uint32_t baudrate) {
	UART::begin(baudrate, uart1_cfg);
}

void UART1::write_char(char ch) {
	UART::write_char(ch, uart1_cfg);
}

size_t UART1::write_to_uart(uint8_t buff) {
	UART::write_char(buff, uart1_cfg);
	return 0;
}

void UART1::flush() {
	memset(uart1_rx_buffer, 0, RX_BUFF_SIZE);
	uart1_pointer = 0;
	uart1_rx_ready = false;
}

bool UART1::available() {
	return uart1_rx_ready;
}

const char *UART1::readString() {
	uart1_rx_ready = false;
	return uart1_rx_buffer;
}
/******************************************************************************************************/

/******************************************    UART2   **************************************************/

UART2::UART2() {
	uart2_cfg.port_no = 2;
	uart2_cfg.uart_port = USART2;
	uart2_cfg.uart_gpio_port_tx = GPIOA;
	uart2_cfg.uart_gpio_port_rx = GPIOA;
	uart2_cfg.rx_pin = GPIO_Pin_3;    // PA3 (USART2_RX)
	uart2_cfg.tx_pin = GPIO_Pin_2;    // PA2 (USART2_TX)
	uart2_cfg.irq = USART2_IRQn;
}

void UART2::begin(uint32_t baudrate) {
	UART::begin(baudrate, uart2_cfg);
}

void UART2::write_char(char ch) {
	UART::write_char(ch, uart2_cfg);
}

size_t UART2::write_to_uart(uint8_t buff) {
	UART::write_char(buff, uart2_cfg);
	return 0;
}

void UART2::flush() {
	memset(uart2_rx_buffer, 0, RX_BUFF_SIZE);
	uart2_pointer = 0;
	uart2_rx_ready = false;
}

bool UART2::available() {
	return uart2_rx_ready;
}

const char *UART2::readString() {
	uart2_rx_ready = false;
	return uart2_rx_buffer;
}
/******************************************************************************************************/

void UART::begin(uint32_t baudrate, uart_cfg cfg) {
	GPIO_InitTypeDef PORT;
	switch (cfg.port_no) {
	case 1:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		break;
	case 2:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		break;
	case 3:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		break;
	case 4:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
		break;
	case 5:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
		break;
	}

	PORT.GPIO_Pin = cfg.tx_pin;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Mode = GPIO_Mode_AF_PP; // TX as AF with Push-Pull
	GPIO_Init(cfg.uart_gpio_port_tx, &PORT);
	PORT.GPIO_Pin = cfg.rx_pin;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	PORT.GPIO_Mode = GPIO_Mode_IN_FLOATING; // RX as in without pull-up
	GPIO_Init(cfg.uart_gpio_port_rx, &PORT);

	USART_InitTypeDef UART;
	UART.USART_BaudRate = baudrate;
	UART.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // No flow control
	UART.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // RX+TX mode
	UART.USART_Parity = USART_Parity_No; // No parity check
	UART.USART_StopBits = USART_StopBits_1; // 1 stop bit
	UART.USART_WordLength = USART_WordLength_8b; // 8-bit frame
	USART_Init(cfg.uart_port, &UART);
	USART_Cmd(cfg.uart_port, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = cfg.irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(cfg.uart_port, USART_IT_RXNE, ENABLE);
	USART_ITConfig(cfg.uart_port, USART_IT_IDLE, ENABLE);

}

/*
 void UART::begin(uint32_t baudrate) {
 GPIO_InitTypeDef PORT;

 // U(S)ART init
 #if _UART_PORT == 1
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
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
 */

void UART::write_char(char ch, uart_cfg cfg) {
	while (!USART_GetFlagStatus(cfg.uart_port, USART_FLAG_TC))
		; // wait for "Transmission Complete" flag cleared
	USART_SendData(cfg.uart_port, ch);
}

#ifdef __cplusplus
extern "C" {
#endif

void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		if (uart1_pointer > RX_BUFF_SIZE) {
			memset(uart1_rx_buffer, 0, RX_BUFF_SIZE);
			uart1_pointer = 0;
			uart1_rx_ready = true;
		}
		uart1_rx_buffer[uart1_pointer++] = USART_ReceiveData(USART1);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) {
		uart1_rx_ready = true;
		USART_ReceiveData(USART1);
	}
}

void USART2_IRQHandler(void) {
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		if (uart2_pointer > RX_BUFF_SIZE) {
			memset(uart2_rx_buffer, 0, RX_BUFF_SIZE);
			uart2_pointer = 0;
			uart2_rx_ready = true;
		}
		uart2_rx_buffer[uart2_pointer++] = USART_ReceiveData(USART2);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) {
		uart2_rx_ready = true;
		USART_ReceiveData(USART2);
	}
}

#ifdef __cplusplus
}
#endif

