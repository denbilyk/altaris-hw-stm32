/*
 * rf24.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: denis.bilyk
 */

#include "rf24.h"

GPIO_Pin* ce_pin;
GPIO_Pin* csn_pin;
GPIO_Pin* mosi_pin;
GPIO_Pin* miso_pin;
GPIO_Pin* sck_pin;

bool wide_band; /* 2Mbs data rate in use? */
bool p_variant; /* False for RF24L01 and true for RF24L01P */
uint8_t payload_size; /**< Fixed size of payloads */
bool ack_payload_available; /**< Whether there is an ack payload waiting */
bool dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */
uint8_t ack_payload_length; /**< Dynamic size of pending ack payload. */
uint64_t pipe0_reading_address; /**< Last address set on pipe 0 for reading. */

/****************************************************************************/

void RF24::csn(uint8_t mode) {
	digitalWrite(csn_pin, mode);
}

/****************************************************************************/

void RF24::ce(uint8_t level) {
	digitalWrite(ce_pin, level);
}

/****************************************************************************/

uint8_t RF24::read_register(uint8_t reg, uint8_t* buf, uint8_t len) {
	uint8_t status;

	csn(LOW);
	status = spi_transfer( R_REGISTER | ( REGISTER_MASK & reg));
	while (len--)
		*buf++ = spi_transfer(0xff);

	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::read_register(uint8_t reg) {
	csn(LOW);
	spi_transfer( R_REGISTER | ( REGISTER_MASK & reg));
	uint8_t result = spi_transfer(0xff);

	csn(HIGH);
	return result;
}

/****************************************************************************/

uint8_t RF24::write_register(uint8_t reg, const uint8_t* buf, uint8_t len) {
	uint8_t status;

	csn(LOW);
	status = spi_transfer( W_REGISTER | ( REGISTER_MASK & reg));
	while (len--)
		spi_transfer(*buf++);

	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::write_register(uint8_t reg, uint8_t value) {
	uint8_t status;

	csn(LOW);
	status = spi_transfer( W_REGISTER | ( REGISTER_MASK & reg));
	spi_transfer(value);
	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::write_payload(const void* buf, uint8_t len) {
	uint8_t status;

	const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

	uint8_t data_len = min(len, payload_size);
	uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

	csn(LOW);
	status = spi_transfer( W_TX_PAYLOAD);
	while (data_len--)
		spi_transfer(*current++);
	while (blank_len--)
		spi_transfer(0);
	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::read_payload(void* buf, uint8_t len) {
	uint8_t status;
	uint8_t* current = reinterpret_cast<uint8_t*>(buf);

	uint8_t data_len = min(len, payload_size);
	uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;

	csn(LOW);
	status = spi_transfer( R_RX_PAYLOAD);
	while (data_len--)
		*current++ = spi_transfer(0xff);
	while (blank_len--)
		spi_transfer(0xff);
	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::flush_rx(void) {
	uint8_t status;

	csn(LOW);
	status = spi_transfer(FLUSH_RX);
	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::flush_tx(void) {
	uint8_t status;

	csn(LOW);
	status = spi_transfer(FLUSH_TX);
	csn(HIGH);

	return status;
}

/****************************************************************************/

uint8_t RF24::get_status(void) {
	uint8_t status;

	csn(LOW);
	status = spi_transfer( NOP);
	csn(HIGH);

	return status;
}

/****************************************************************************/

void RF24::setPALevel(rf24_pa_dbm_e level) {

	uint8_t setup = read_register(RF_SETUP);
	setup &= ~(_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));

	// switch uses RAM (evil!)
	if (level == RF24_PA_MAX) {
		setup |= (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));
	} else if (level == RF24_PA_HIGH) {
		setup |= _BV(RF_PWR_HIGH);
	} else if (level == RF24_PA_LOW) {
		setup |= _BV(RF_PWR_LOW);
	} else if (level == RF24_PA_MIN) {
		// nothing
	} else if (level == RF24_PA_ERROR) {
		// On error, go to maximum PA
		setup |= (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));
	}

	write_register( RF_SETUP, setup);
}

/****************************************************************************/

bool RF24::setDataRate(rf24_datarate_e speed) {
	bool result = false;
	uint8_t setup = read_register(RF_SETUP);

	// HIGH and LOW '00' is 1Mbs - our default
	wide_band = false;
	setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));
	if (speed == RF24_250KBPS) {
		// Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
		// Making it '10'.
		wide_band = false;
		setup |= _BV(RF_DR_LOW);
	} else {
		// Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
		// Making it '01'
		if (speed == RF24_2MBPS) {
			wide_band = true;
			setup |= _BV(RF_DR_HIGH);
		} else {
			// 1Mbs
			wide_band = false;
		}
	}
	write_register(RF_SETUP, setup);

	// Verify our result
	if (read_register(RF_SETUP) == setup) {
		result = true;
	} else {
		wide_band = false;
	}

	return result;
}

/****************************************************************************/

void RF24::setCRCLength(rf24_crclength_e length) {
	uint8_t config = read_register(CONFIG) & ~( _BV(CRCO) | _BV(EN_CRC));

	// switch uses RAM (evil!)
	if (length == RF24_CRC_DISABLED) {
		// Do nothing, we turned it off above.
	} else if (length == RF24_CRC_8) {
		config |= _BV(EN_CRC);
	} else {
		config |= _BV(EN_CRC);
		config |= _BV(CRCO);
	}
	write_register( CONFIG, config);
}

/****************************************************************************/

void RF24::setChannel(uint8_t channel) {
	const uint8_t max_channel = 127;
	write_register(RF_CH, min(channel, max_channel));
}

/****************************************************************************/
void RF24::setRetries(uint8_t delay, uint8_t count) {
	write_register(SETUP_RETR, (delay & 0xf) << ARD | (count & 0xf) << ARC);
}

/****************************************************************************/

static const uint8_t child_pipe[] = { RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5 };
static const uint8_t child_payload_size[] = { RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5 };
static const uint8_t child_pipe_enable[] = { ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5 };

void RF24::openReadingPipe(uint8_t child, uint64_t address) {
	// If this is pipe 0, cache the address.  This is needed because
	// openWritingPipe() will overwrite the pipe 0 address, so
	// startListening() will have to restore it.
	if (child == 0)
		pipe0_reading_address = address;

	if (child <= 6) {
		// For pipes 2-5, only write the LSB
		if (child < 2)
			write_register(child_pipe[child], reinterpret_cast<const uint8_t*>(&address), 5);
		else
			write_register(child_pipe[child], reinterpret_cast<const uint8_t*>(&address), 1);

		write_register(child_payload_size[child], payload_size);

		// Note it would be more efficient to set all of the bits for all open
		// pipes at once.  However, I thought it would make the calling code
		// more simple to do it this way.
		write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(child_pipe_enable[child]));
	}
}

/****************************************************************************/

void RF24::openWritingPipe(uint64_t value) {
	write_register(RX_ADDR_P0, reinterpret_cast<uint8_t*>(&value), 5);
	write_register(TX_ADDR, reinterpret_cast<uint8_t*>(&value), 5);

	const uint8_t max_payload_size = 32;
	write_register(RX_PW_P0, min(payload_size, max_payload_size));
}

/****************************************************************************/

void RF24::startListening(void) {
	write_register(CONFIG, read_register(CONFIG) | _BV(PWR_UP) | _BV(PRIM_RX));
	write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

	// Restore the pipe0 adddress, if exists
	if (pipe0_reading_address)
		write_register(RX_ADDR_P0, reinterpret_cast<const uint8_t*>(&pipe0_reading_address), 5);

	// Flush buffers
	flush_rx();
	flush_tx();

	// Go!
	ce(HIGH);

	// wait for the radio to come up (130us actually only needed)
	delay_ms(1);
}

/****************************************************************************/

void RF24::stopListening(void) {
	ce(LOW);
	flush_tx();
	flush_rx();
}

/****************************************************************************/

bool RF24::available(uint8_t* pipe_num) {
	uint8_t status = get_status();

	bool result = (status & _BV(RX_DR));

	if (result) {
		// If the caller wants the pipe number, include that
		if (pipe_num)
			*pipe_num = (status >> RX_P_NO) & 0x07;

		// Clear the status bit

		// ??? Should this REALLY be cleared now?  Or wait until we
		// actually READ the payload?

		write_register(STATUS, _BV(RX_DR));

		// Handle ack payload receipt
		if (status & _BV(TX_DS)) {
			write_register(STATUS, _BV(TX_DS));
		}
	}
	return result;
}

/****************************************************************************/

bool RF24::available() {
	return available(__null);
}

/****************************************************************************/

bool RF24::read(void* buf, uint8_t len) {
	// Fetch the payload
	read_payload(buf, len);

	// was this the last of the data available?
	return read_register(FIFO_STATUS) & _BV(RX_EMPTY);
}

/****************************************************************************/

bool RF24::write(const void* buf, uint8_t len) {
	bool result = false;

	startWrite(buf, len);

	uint8_t observe_tx;
	uint8_t status;

	do {
		status = read_register(OBSERVE_TX, &observe_tx, 1);
	} while (!(status & ( _BV(TX_DS) | _BV(MAX_RT)))); //TODO: blocking can appear

	bool tx_ok, tx_fail;
	whatHappened(tx_ok, tx_fail, ack_payload_available);
	result = tx_ok;

	if (ack_payload_available) {
		ack_payload_length = getDynamicPayloadSize();
	}
	powerDown();
	flush_tx();
	return result;
}

/****************************************************************************/

void RF24::whatHappened(bool& tx_ok, bool& tx_fail, bool& rx_ready) {
	// Read the status & reset the status in one easy call
	// Or is that such a good idea?
	uint8_t status = write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

	// Report to the user what happened
	tx_ok = status & _BV(TX_DS);
	tx_fail = status & _BV(MAX_RT);
	rx_ready = status & _BV(RX_DR);
}

/****************************************************************************/

void RF24::startWrite(const void* buf, uint8_t len) {
	// Transmitter power-up
	write_register(CONFIG, (read_register(CONFIG) | _BV(PWR_UP)) & ~_BV(PRIM_RX));
	delay_ms(1);

	write_payload(buf, len);

	ce(HIGH);
	delay_ms(1);
	ce(LOW);
}

/****************************************************************************/

uint8_t RF24::getDynamicPayloadSize(void) {
	uint8_t result = 0;

	csn(LOW);
	spi_transfer( R_RX_PL_WID);
	result = spi_transfer(0xff);
	csn(HIGH);

	return result;
}

/****************************************************************************/

void RF24::powerDown(void) {
	write_register(CONFIG, read_register(CONFIG) & ~_BV(PWR_UP));
}

/****************************************************************************/

void RF24::powerUp(void) {
	write_register(CONFIG, read_register(CONFIG) | _BV(PWR_UP));
}

/******************************************************************/

void RF24::begin(void) {
	// Initialize pins
	pinMode(ce_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	pinMode(csn_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);

	// Initialize SPI bus
	spi_init(miso_pin, mosi_pin, sck_pin);

	ce(LOW);
	csn(HIGH);

	delay_ms(5);

	// Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
	// WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
	// sizes must never be used. See documentation for a more complete explanation.
	write_register(SETUP_RETR, (0x04 << ARD) | (0x0F << ARC));

	// Restore our default PA level
	setPALevel(RF24_PA_MAX);

	// Determine if this is a p or non-p RF24 module and then
	// reset our data rate back to default value. This works
	// because a non-P variant won't allow the data rate to
	// be set to 250Kbps.
	if (setDataRate(RF24_250KBPS)) {
		p_variant = true;
	}

	// Then set the data rate to the slowest (and most reliable) speed supported by all
	// hardware.
	setDataRate(RF24_1MBPS);

	// Initialize CRC and request 2-byte (16bit) CRC
	setCRCLength(RF24_CRC_16);

	// Disable dynamic payloads, to match dynamic_payloads_enabled setting
	write_register(DYNPD, 0);

	// Reset current status
	// Notice reset and flush is the last thing we do
	write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

	// Set up default configuration.  Callers can always change it later.
	// This channel should be universally safe and not bleed over into adjacent
	// spectrum.
	setChannel(76);

	// Flush buffers
	flush_rx();
	flush_tx();
}

/****************************************************************************/

void RF24::print_status(uint8_t status) {
	printf("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x TX_FULL=%x\r\n", status,
			(status & _BV(RX_DR)) ? 1 : 0, (status & _BV(TX_DS)) ? 1 : 0, (status & _BV(MAX_RT)) ? 1 : 0,
			((status >> RX_P_NO) & 0x07), (status & _BV(TX_FULL)) ? 1 : 0);
}

/****************************************************************************/

void RF24::print_observe_tx(uint8_t value) {
	printf("OBSERVE_TX=%02x: POLS_CNT=%x ARC_CNT=%x\r\n", value, (value >> PLOS_CNT) & 0x0F, (value >> ARC_CNT) & 0x0F);
}

/****************************************************************************/

void RF24::print_byte_register(const char* name, uint8_t reg, uint8_t qty) {
	char extra_tab = strlen(name) < 8 ? '\t' : 0;
	printf("%s\t%c =", name, extra_tab);
	while (qty--)
		printf(" 0x%02x", read_register(reg++));
	printf("\r\n");
}

/****************************************************************************/

void RF24::print_address_register(const char* name, uint8_t reg, uint8_t qty) {
	char extra_tab = strlen(name) < 8 ? '\t' : 0;
	printf("%s\t%c =", name, extra_tab);

	while (qty--) {
		uint8_t buffer[5];
		read_register(reg++, buffer, sizeof buffer);

		printf(" 0x");
		uint8_t* bufptr = buffer + sizeof buffer;
		while (--bufptr >= buffer)
			printf("%02x", *bufptr);
	}

	printf("\r\n");
}

/****************************************************************************/

static const char rf24_datarate_e_str_0[] = "1MBPS";
static const char rf24_datarate_e_str_1[] = "2MBPS";
static const char rf24_datarate_e_str_2[] = "250KBPS";
static const char * const rf24_datarate_e_str_P[] = { rf24_datarate_e_str_0, rf24_datarate_e_str_1,
		rf24_datarate_e_str_2, };
static const char rf24_model_e_str_0[] = "nRF24L01";
static const char rf24_model_e_str_1[] = "nRF24L01+";
static const char * const rf24_model_e_str_P[] = { rf24_model_e_str_0, rf24_model_e_str_1, };
static const char rf24_crclength_e_str_0[] = "Disabled";
static const char rf24_crclength_e_str_1[] = "8 bits";
static const char rf24_crclength_e_str_2[] = "16 bits";
static const char * const rf24_crclength_e_str_P[] = { rf24_crclength_e_str_0, rf24_crclength_e_str_1,
		rf24_crclength_e_str_2, };
static const char rf24_pa_dbm_e_str_0[] = "PA_MIN";
static const char rf24_pa_dbm_e_str_1[] = "PA_LOW";
static const char rf24_pa_dbm_e_str_2[] = "LA_MED";
static const char rf24_pa_dbm_e_str_3[] = "PA_HIGH";
static const char * const rf24_pa_dbm_e_str_P[] = { rf24_pa_dbm_e_str_0, rf24_pa_dbm_e_str_1, rf24_pa_dbm_e_str_2,
		rf24_pa_dbm_e_str_3, };

/****************************************************************************/

void RF24::printDetails(void) {
	print_status(get_status());

	print_address_register("RX_ADDR_P0-1", RX_ADDR_P0, 2);
	print_byte_register("RX_ADDR_P2-5", RX_ADDR_P2, 4);
	print_address_register("TX_ADDR", TX_ADDR);

	print_byte_register("RX_PW_P0-6", RX_PW_P0, 6);
	print_byte_register("EN_AA", EN_AA);
	print_byte_register("EN_RXADDR", EN_RXADDR);
	print_byte_register("RF_CH", RF_CH);
	print_byte_register("RF_SETUP", RF_SETUP);
	print_byte_register("CONFIG", CONFIG);
	print_byte_register("DYNPD/FEATURE", DYNPD, 2);

	printf(("Data Rate\t = %s\r\n"), rf24_datarate_e_str_P[getDataRate()]);
	printf(("Model\t\t = %s\r\n"), rf24_model_e_str_P[isPVariant()]);
	printf(("CRC Length\t = %s\r\n"), rf24_crclength_e_str_P[getCRCLength()]);
	printf(("PA Power\t = %s\r\n"), rf24_pa_dbm_e_str_P[getPALevel()]);
}

/****************************************************************************/

RF24::RF24(GPIO_Pin* ce, GPIO_Pin* csn, GPIO_Pin* miso, GPIO_Pin* mosi, GPIO_Pin* sck) {
	ce_pin = ce;
	csn_pin = csn;
	mosi_pin = mosi;
	miso_pin = miso;
	sck_pin = sck;
	wide_band = true;
	p_variant = false;
	payload_size = 32;
	ack_payload_available = false;
	dynamic_payloads_enabled = false;
	pipe0_reading_address = 0;

}

/****************************************************************************/

void RF24::setPayloadSize(uint8_t size) {
	const uint8_t max_payload_size = 32;
	payload_size = min(size, max_payload_size);
}

/****************************************************************************/

uint8_t RF24::getPayloadSize(void) {
	return payload_size;
}

/****************************************************************************/

void RF24::toggle_features(void) {
	csn(LOW);
	spi_transfer( ACTIVATE);
	spi_transfer(0x73);
	csn(HIGH);
}

/****************************************************************************/

void RF24::enableDynamicPayloads(void) {
	write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));

	if (!read_register(FEATURE)) {
		// So enable them and try again
		toggle_features();
		write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));
	}
	write_register(DYNPD,
			read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

	dynamic_payloads_enabled = true;
}

/****************************************************************************/

void RF24::enableAckPayload(void) {

	write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));

	if (!read_register(FEATURE)) {
		// So enable them and try again
		toggle_features();
		write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));
	}
	write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
}

/****************************************************************************/

void RF24::writeAckPayload(uint8_t pipe, const void* buf, uint8_t len) {
	const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);

	csn(LOW);
	spi_transfer( W_ACK_PAYLOAD | (pipe & 0x07));
	const uint8_t max_payload_size = 32;
	uint8_t data_len = min(len, max_payload_size);
	while (data_len--)
		spi_transfer(*current++);

	csn(HIGH);
}

/****************************************************************************/

bool RF24::isAckPayloadAvailable(void) {
	bool result = ack_payload_available;
	ack_payload_available = false;
	return result;
}

/****************************************************************************/

bool RF24::isPVariant(void) {
	return p_variant;
}

/****************************************************************************/

void RF24::setAutoAck(bool enable) {
	if (enable)
		write_register(EN_AA, 0x3F);
	else
		write_register(EN_AA, 0);
}

/****************************************************************************/

void RF24::setAutoAck(uint8_t pipe, bool enable) {
	if (pipe <= 6) {
		uint8_t en_aa = read_register( EN_AA);
		if (enable) {
			en_aa |= _BV(pipe);
		} else {
			en_aa &= ~_BV(pipe);
		}
		write_register( EN_AA, en_aa);
	}
}

/****************************************************************************/

bool RF24::testCarrier(void) {
	return (read_register(CD) & 1);
}

/****************************************************************************/

bool RF24::testRPD(void) {
	return (read_register(RPD) & 1);
}

/****************************************************************************/

rf24_pa_dbm_e RF24::getPALevel(void) {
	rf24_pa_dbm_e result = RF24_PA_ERROR;
	uint8_t power = read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));

	// switch uses RAM (evil!)
	if (power == (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) {
		result = RF24_PA_MAX;
	} else if (power == _BV(RF_PWR_HIGH)) {
		result = RF24_PA_HIGH;
	} else if (power == _BV(RF_PWR_LOW)) {
		result = RF24_PA_LOW;
	} else {
		result = RF24_PA_MIN;
	}

	return result;
}

/****************************************************************************/

rf24_datarate_e RF24::getDataRate(void) {
	rf24_datarate_e result;
	uint8_t dr = read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

	if (dr == _BV(RF_DR_LOW)) {
		// '10' = 250KBPS
		result = RF24_250KBPS;
	} else if (dr == _BV(RF_DR_HIGH)) {
		// '01' = 2MBPS
		result = RF24_2MBPS;
	} else {
		// '00' = 1MBPS
		result = RF24_1MBPS;
	}
	return result;
}

/****************************************************************************/

rf24_crclength_e RF24::getCRCLength(void) {
	rf24_crclength_e result = RF24_CRC_DISABLED;
	uint8_t config = read_register(CONFIG) & ( _BV(CRCO) | _BV(EN_CRC));

	if (config & _BV(EN_CRC)) {
		if (config & _BV(CRCO))
			result = RF24_CRC_16;
		else
			result = RF24_CRC_8;
	}

	return result;
}

/****************************************************************************/

void RF24::disableCRC(void) {
	uint8_t disable = read_register(CONFIG) & ~_BV(EN_CRC);
	write_register( CONFIG, disable);
}

