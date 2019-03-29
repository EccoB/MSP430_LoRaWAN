/*
 * custom.h
 *
 *  Created on: 07.08.2018
 *      Author: baeumker
 */

#ifndef TRANSMITTER_CUSTOM_H_
#define TRANSMITTER_CUSTOM_H_

#include "mcuspecific/mcu.h"    //needed for int/uint definition
#include "mcuspecific/uart.h"
#include "../def.h"

// ------- TTN Configuration - Adopt this to your needs in custom.c -----------
extern const uint8_t net_key[16];
extern const uint8_t app_key[16];
extern const uint32_t device_address;

//----------------- Settings ---------------------------------------------------
#ifdef SX1276_CHIP

#endif

#define RF_FREQUENCY   868100000 // Hz
#define RF_FREQUENCY_MHz    868 // MHz


#ifndef SX1276_CHIP
#define LORA_BANDWIDTH                              4         // [4: 125 kHz, 5: 250 kHz, 6: 500 kHz]   //
#else
#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#endif

#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        6         // SX1276: Same for Tx and Rx
#define LORA_PREAMBLE_LENGTH_TX                     6         // SX126x: Default is 12
#define LORA_PREAMBLE_LENGTH_RX                     32        // SX126x: For receiving (when unknown: longer is better)
#define LORA_SYMBOL_TIMEOUT                         3         // Symbols
#define LORA_CRC_ACTIVE                             true      // For SX126x
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false

#ifndef SX1276_CHIP
#define LORA_IQ_INVERSION_ON                        false
#else
#define LORA_IQ_INVERSION_ON                        true
#endif

#define RX_TIMEOUT_VALUE                            1000    // Used by SX1276 (no receiving windows yet)
#define RX1_TIMEOUT_VALUE                           5000 // Timeout of receiving window = Timeout_value * 15.625 us
#define RX2_TIMEOUT_VALUE                           17000


#define RADIO_RAMP                                  RADIO_RAMP_200_US

// Value for TimerA
#define COMPARE_VALUE 32768     // CCR0 value for timer interruption, used by Receiving Windows, @32.768kHz: 32768 = 1 second

#define RFM95       // Must be defined when using the RFM95 module, as this uses PA_BOOST. For the inAir9 module this line should be commented.

#ifdef SX1276_CHIP
#ifdef RFM95
    #define TX_OUTPUT_POWER                   7 // (real power = output_power (PA_BOOST in use) [dBm])
#else
    #define TX_OUTPUT_POWER                   10 // (real power = output_power (RFO in use) [dBm])
#endif
#else   // SX1261 or SX1262
    #define TX_OUTPUT_POWER                   15 // (real power = output_power[dBm])
#endif


//---------- Pins for LoraModule --------------------
// SX1276-related pins
#define BUSY_PIN     GPIO_PORT_P3, GPIO_PIN0
#define IRQ_PIN      GPIO_PORT_P1, GPIO_PIN3
#define SX_RESET_PIN GPIO_PORT_P1, GPIO_PIN4
#define NSS_PIN      GPIO_PORT_P1, GPIO_PIN5
#define ANT_SW_PIN   GPIO_PORT_P4, GPIO_PIN3


#define SPIPORTCLK	GPIO_PORT_P2
#define SPICLK		GPIO_PIN2
#define SPIPORTDATA	GPIO_PORT_P1
#define SPITX		GPIO_PIN6
#define SPIRX		GPIO_PIN7

/*
#define LORA_PORT	GPIO_PORT_P1
#define LORA_CSPIN	GPIO_PIN5
#define LORA_RESET  GPIO_PIN4
#define LORA_IVTXD	GPIO_PIN3
*/

//---------------------------------------------------------------------------------------

// ---- default values for Init of transmitter ---------------
#define PAYLOAD_SIZE        4
#define MESSAGE_SIZE        PAYLOAD_SIZE + 9        // 9 Bytes MHDR+FHDR+FPort
#define PHY_PAYLOAD_SIZE    MESSAGE_SIZE + 4        // 4 Bytes Message Integrity Check

//--------Definitions for SPIARRAY-------------
#define ARRAY_TRANSFER_SIZE   PHY_PAYLOAD_SIZE + 1    // 1 Byte for FIFO-Write Command
#define TTNASIZE 30
#define SPIASIZE 30
#define DATA_BUFFER_SIZE 256

extern uint8_t ttn[TTNASIZE];
extern uint8_t spi_commands[SPIASIZE];
//------------------------------------


//---------- Functions ----------
void SendPing();                                    // Test function, it sends PAYLOAD_SIZE Byte(s) and increments the value by 1 every time
void receivingWindowsISR();                         // Function for the handling of Receiving Windows
void sendLoRaWAN(uint8_t *data, uint8_t length);	//Sends a LoRaWANPacket to TTN, set the Networkkey and Devicekey for that!
void TestTransmission();							//Sends variing size to TTN for Tests
#endif /* TRANSMITTER_CUSTOM_H_ */
