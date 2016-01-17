/*
 * rf24.h
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#ifndef RF24_RF24_H_
#define RF24_RF24_H_

#include <string.h>
#include <stdio.h>
#include "../pins.h"
#include "../spi.h"
#include "nRF24L01.h"
#include "../utils/delay.h"

#define _BV(x) (1<<(x))
#define min(a,b) ((a)<(b)?(a):(b))

typedef enum {
	RF24_PA_MIN = 0, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX, RF24_PA_ERROR
} rf24_pa_dbm_e;

typedef enum {
	RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS
} rf24_datarate_e;

typedef enum {
	RF24_CRC_DISABLED = 0, RF24_CRC_8, RF24_CRC_16
} rf24_crclength_e;

class RF24 {
private:
	void csn(uint8_t mode);  //SPI Chip Select
	void ce(uint8_t level);  //Chip Enable Activates RX or TX mode
	uint8_t flush_tx(void);
	uint8_t flush_rx(void);
	uint8_t read_payload(void* buf, uint8_t len);
	uint8_t write_payload(const void* buf, uint8_t len);
	uint8_t write_register(uint8_t reg, uint8_t value);
	uint8_t write_register(uint8_t reg, const uint8_t* buf, uint8_t len);
	uint8_t read_register(uint8_t reg);
	uint8_t read_register(uint8_t reg, uint8_t* buf, uint8_t len);
	uint8_t get_status(void);
	void startWrite(const void* buf, uint8_t len);
	void toggle_features(void);

	void print_observe_tx(uint8_t value);
	void print_status(uint8_t status);
	void print_byte_register(const char* name, uint8_t reg, uint8_t qty = 1);
	void print_address_register(const char* name, uint8_t reg, uint8_t qty = 1);

public:
	RF24(GPIO_Pin* ce, GPIO_Pin* csn, GPIO_Pin* miso, GPIO_Pin* mosi, GPIO_Pin* sck);
	void begin(void);
	void setPALevel(rf24_pa_dbm_e level);
	bool setDataRate(rf24_datarate_e speed);
	void setCRCLength(rf24_crclength_e length);
	void setChannel(uint8_t channel);
	void setRetries(uint8_t delay, uint8_t count);
	void setAutoAck(bool enable);
	void openReadingPipe(uint8_t child, uint64_t address);
	void openWritingPipe(uint64_t value);
	void startListening(void);
	void stopListening(void);
	bool available(uint8_t* pipe_num);
	bool available();
	bool read(void* buf, uint8_t len);
	bool write(const void* buf, uint8_t len);
	uint8_t getDynamicPayloadSize(void);
	void whatHappened(bool& tx_ok, bool& tx_fail, bool& rx_ready);
	void powerDown(void);
	void powerUp(void);
	void setPayloadSize(uint8_t size);
	void printDetails(void);

	uint8_t getPayloadSize(void);
	void enableDynamicPayloads(void);
	void enableAckPayload(void);
	void writeAckPayload(uint8_t pipe, const void* buf, uint8_t len);
	bool isAckPayloadAvailable(void);
	bool isPVariant(void);
	void setAutoAck(uint8_t pipe, bool enable);
	bool testCarrier(void);
	bool testRPD(void);
	rf24_pa_dbm_e getPALevel(void);
	rf24_datarate_e getDataRate(void);
	rf24_crclength_e getCRCLength(void);
	void disableCRC(void);

};

#endif /* RF24_RF24_H_ */
