/*
 * transmitter.h
 *
 *  Created on: 07.08.2018
 *      Author: baeumker
 */

#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

void rf_init_lora();
void rf_waitTillIdle();
void rf_setSpreadingF(int sf);
void rf_setPAPower(int pa_power);
void rf_transmit(char *data, unsigned int length);
int rf_transmitReceiveB(char *dataToSend, unsigned int length, char *buffer);		//Returns -1: no data received, else: length of received data

void rf_loop();
void rf_turnOff(int state);		//state=0 -> lowest possible power mode



void rf_continuousListen(void *(*recCallback)(char *buffer, int length));		//Non blocking function, should be called in a loop




#endif /* TRANSMITTER_H_ */
