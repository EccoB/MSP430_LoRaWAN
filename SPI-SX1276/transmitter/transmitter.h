/*
 * transmitter.h
 *
 *  Created on: 29.08.2018
 *      Author: Armando Miguel
 */

#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include <stdint.h>

void rf_init_lora();

void rf_setSpreadingF(uint8_t sf);
void rf_setPAPower(int8_t tx_power);
void rf_transmit(uint8_t *data, uint8_t length, void (*recCallback)());
void rf_transmitReceiveB(uint8_t *dataToSend, uint8_t length, void (*recCallback)(uint8_t *buffer, uint8_t length));

void rf_loop();         // Not implemented yet
void rf_waitTillIdle(); // Wait until transmitter is in MODE_STDBY_RC or MODE_STDBY_XOSC mode
void rf_turnOff(uint8_t state);     // state=0 -> "Deep" Sleep; state=1 -> Sleep with data retention

void rf_continuousListen(void (*recCallback)(uint8_t *buffer, uint8_t length));     // Non blocking function




#endif /* TRANSMITTER_H_ */
