/*
 * custom.h
 *
 *  Created on: 07.08.2018
 *      Author: baeumker
 */

#ifndef TRANSMITTER_CUSTOM_H_
#define TRANSMITTER_CUSTOM_H_

#include "mcu.h"		//needed for int/uint definition
#include "../def.h"

// ------- TTN Configuration - Adopt this to your needs in custom.c -----------
extern const uint8_t net_key[16];
extern const uint8_t app_key[16];
extern const uint32_t device_address;

//----------------- Settings ---------------------------------------------------
#define RF_FREQUENCY   868000000 // Hz
#define RF_FREQUENCY_MHz    868 // MHz

#define FSK_FDEV                          25e3      // Hz
#define FSK_DATARATE                      50e3      // bps
#define FSK_BANDWIDTH                     50e3      // Hz
#define FSK_AFC_BANDWIDTH                 83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH               5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON         false

#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        6         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         3         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        true

#define RX_TIMEOUT_VALUE                  1000


//---------- Pins for LoraModule --------------------
#define SPIPORTCLK	GPIO_PORT_P2
#define SPICLK		GPIO_PIN2
#define SPIPORTDATA	GPIO_PORT_P1
#define SPITX		GPIO_PIN6
#define SPIRX		GPIO_PIN7

#define LORA_PORT	GPIO_PORT_P1
#define LORA_CSPIN	GPIO_PIN5
#define LORA_RESET  GPIO_PIN4
#define LORA_IVTXD	GPIO_PIN3


//#define RFM95		// Must be defined when using the RFM95 module, as this uses PA_BOOST. For the inAir9 module this line should be commented.
//---------------------------------------------------------------------------------------

// ---- default values for Init of transmitter ---------------
#define PAYLOAD_SIZE        4
#define MESSAGE_SIZE        PAYLOAD_SIZE + 9        // 9 Bytes MHDR+FHDR+FPort
#define PHY_PAYLOAD_SIZE    MESSAGE_SIZE + 4        // 4 Bytes Message Integrity Check

#ifdef RFM95
	#define TX_OUTPUT_POWER                   13 // (real power = output_power (PA_BOOST in use) [dBm])
#else
	#define TX_OUTPUT_POWER                   10 // (real power = output_power (RFO in use) [dBm])
#endif


//---------- Functions ----------

void sendLoRaWAN(uint8_t *data, uint8_t length);	//Sends a LoRaWANPacket to TTN, set the Networkkey and Devicekey for that!
void TestTransmission();							//Sends variing size to TTN for Tests
#endif /* TRANSMITTER_CUSTOM_H_ */
