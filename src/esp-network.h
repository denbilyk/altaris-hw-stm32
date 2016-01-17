//
// Created by Denis Bilyk on 10/1/15.
//

#ifndef ESP_NETWORK_H
#define ESP_NETWORK_H

#include "stdint.h"
#include "diag/Trace.h"
#include "utils/delay.h"
#include "uart/uart.h"

const String ESP_RESP_ERROR = "ERROR";
const String ESP_AT = "AT";
const String ESP_RST = ESP_AT + "+RST";
const String ESP_CWMODE = ESP_AT + "+CWMODE=1";
const String ESP_CWJAP = ESP_AT + "+CWJAP=\"{0}\",\"{1}\"";
const String ESP_CIPSTA = ESP_AT + "+CIPSTA?";
const String ESP_CIPMUX_0 = ESP_AT + "+CIPMUX=0";
const String ESP_CIPMUX_1 = ESP_AT + "+CIPMUX=1";

const String ESP_CIPSEND = ESP_AT + "+CIPSEND=0,{0}";

const String CMD_TCP_HOST = "AT+CIPSTART=0,\"TCP\",\"{0}\",{1}";
const String GET_REQUEST = "GET /api/submit?auth={0}&id={1}&temp={2}&humid={3}\r\n\r\n";
const String GET_RAW_REQUEST = "GET /api/submit?auth={0}&raw={1}\r\n\r\n";

class ESP {

private:
    bool cmd(String cmd, uint16_t delay);

    bool r_check(uint8_t counter);

public:

    bool esp_init(String, String);

    bool esp_check() ;

    bool send_AT();

    bool send_RST();

    bool send_CWMODE();

    bool send_CWJAP(String ssid, String pass);

    String send_CIPSTA();

    bool send_CIPMUX(uint8_t value);

    bool send_data(String host, String port, String auth, String id, String temp, String humid);

    bool send_data(String host, String port, String auth, String raw);

};

#endif //ESP_NETWORK_H
