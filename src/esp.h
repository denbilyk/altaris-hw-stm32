/*
 * esp.h
 *
 *  Created on: Jan 31, 2016
 *      Author: denis.bilyk
 */

#ifndef ESP_H_
#define ESP_H_


bool esp_init();
bool esp_check();
bool esp_setMuxMode();

bool esp_connect_to_host() ;
bool esp_send_data(const char* raw_data);

#endif /* ESP_H_ */
