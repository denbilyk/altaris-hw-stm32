/*
 * uart.h
 *
 *  Created on: Jan 12, 2016
 *      Author: denis.bilyk
 */

#ifndef UART_H_
#define UART_H_

#include "print.h"
#include "stm32f10x.h"

#define _UART_PORT 1

#if _UART_PORT == 1
#define UART_PORT         USART1
#define UART_TX_PIN       GPIO_Pin_9    // PA9  (USART1_TX)
#define UART_RX_PIN       GPIO_Pin_10   // PA10 (USART1_RX)
#define UART_GPIO_PORT_TX GPIOA
#define UART_GPIO_PORT_RX UART_GPIO_PORT_TX
#define UART_IRQ		  USART1_IRQn
#elif _UART_PORT == 2
#define UART_PORT         USART2
#define UART_TX_PIN       GPIO_Pin_2    // PA2 (USART2_TX)
#define UART_RX_PIN       GPIO_Pin_3    // PA3 (USART2_RX)
#define UART_GPIO_PORT_TX GPIOA
#define UART_GPIO_PORT_RX UART_GPIO_PORT_TX
#define UART_IRQ		  USART2_IRQn
#elif _UART_PORT == 3
#define UART_PORT         USART3
#define UART_TX_PIN       GPIO_Pin_10    // PB10 (USART3_TX)
#define UART_RX_PIN       GPIO_Pin_11    // PB11 (USART3_RX)
#define UART_GPIO_PORT_TX GPIOB
#define UART_GPIO_PORT_RX UART_GPIO_PORT_TX
#define UART_IRQ		  USART3_IRQn
#elif _UART_PORT == 4
#define UART_PORT         UART4
#define UART_TX_PIN       GPIO_Pin_10    // PC10 (UART4_TX)
#define UART_RX_PIN       GPIO_Pin_11    // PC11 (UART4_RX)
#define UART_GPIO_PORT_TX GPIOC
#define UART_GPIO_PORT_RX UART_GPIO_PORT_TX
#define UART_IRQ		  USART4_IRQn
#elif _UART_PORT == 5
#define UART_PORT         UART5
#define UART_TX_PIN       GPIO_Pin_12    // PC12 (UART5_TX)
#define UART_RX_PIN       GPIO_Pin_2     // PD2  (UART5_RX)
#define UART_GPIO_PORT_TX GPIOC
#define UART_GPIO_PORT_RX GPIOD
#define UART_IRQ		  USART5_IRQn
#endif

#define RX_BUFF_SIZE 255

typedef struct {
	uint8_t port_no;
	USART_TypeDef* uart_port;
	uint16_t tx_pin;
	uint16_t rx_pin;
	GPIO_TypeDef* uart_gpio_port_tx;
	GPIO_TypeDef* uart_gpio_port_rx;
	uint16_t irq;

} uart_cfg;

class UART: public Print {
protected:
	void write_char(char, uart_cfg cfg);
	void begin(uint32_t baudrate, uart_cfg cfg);
public:

};

class UART1: public UART {
protected:
	size_t write_to_uart(uint8_t buff);
	void write_char(char);

public:
	void begin(uint32_t);
	const char * readString();
	bool available();
	void flush();
	UART1();
};

class UART2: public UART {
protected:
	size_t write_to_uart(uint8_t buff);
	void write_char(char);

public:
	void begin(uint32_t);
	const char * readString();
	bool available();
	void flush();
	UART2();
};

#endif /* UART_H_ */
