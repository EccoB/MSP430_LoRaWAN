/*
 * custom.c
 *
 *  Created on: 07.08.2018
 *      Author: baeumker
 */
#include <custom.h>
#include "../ttn/LoRaMacCrypto.h"
#include <driverlib.h>
#include <sx1276.h>
#include <sx1276regs-lora.h>
#include "../def.h"

#ifdef SX1276_CHIP

//-------- Configuration - change here ----------------------------------------
const uint8_t net_key[16] = {0x00};
const uint8_t app_key[16] = {0x00};
const uint32_t device_address = 0x11111111;
//----------------------------------------------------------------------------

#define DIR_UPLINK          0
#define DIR_DOWNLINK        1



#define LORAWANOVERHEAD	9
#define MAXPACKETLENGTH	20

/*
 * Needs:
 * 	-	app_key
 * 	-	device_address
 * Input:
 *  -	data:	data that should be encoded
 *  -	length:	length of data packet
 *
 */
void sendLoRaWAN(uint8_t *data, uint8_t length){

	uint8_t tosend[MAXPACKETLENGTH+LORAWANOVERHEAD+4];
	static uint8_t framecounter=0;
	uint8_t port=0x01;
	tosend[0]=0x40;										//For TTN
	// Insert Device Address into ttn array
	tosend[1] = (uint8_t)((device_address) & 0xFF);         // LSB
	tosend[2] = (uint8_t)((device_address >> 8) & 0xFF);
	tosend[3] = (uint8_t)((device_address >> 16) & 0xFF);
	tosend[4] = (uint8_t)((device_address >> 24) & 0xFF);   // MSB

	//Insert non static fields:
	tosend[5] = 0x80;		//Frame Control: No Adaptive Data Rate, NO Ack, no frame pending
	tosend[6] = framecounter;
	tosend[7] = 0x00;		//FOptions
	tosend[8] = port;		//Port


	LoRaMacPayloadEncrypt(&data[0], length, app_key, device_address, DIR_UPLINK, 0, &tosend[9]);
	//Bug: seems like the conversion to uint_32 only allows
	uint32_t mic=0xFF;
	LoRaMacComputeMic(&tosend[0], length + LORAWANOVERHEAD , net_key, device_address, DIR_UPLINK, 0, &mic);
	int t=0;
	for(t=0; t<4;t++){
		tosend[length+LORAWANOVERHEAD+t] =(uint8_t)((mic >> (t*8)& 0xFF) );
	}


	// ---- Now all the needed stuff in in the array from position 0 to length+LORAWANOVERHEAD(9)+4 -1(index)
	sx1276_send(&tosend[0], length+LORAWANOVERHEAD+4);

}


void TestTransmission() {

#ifdef TTN
  static uint8_t counter=0;
  counter++;
  uint8_t testarray[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D};
  if(counter ==  sizeof(testarray)){
	  counter=1;
  }
  sendLoRaWAN(&testarray[0], counter);
#endif
#ifdef LEDS
  GPIO_toggleOutputOnPin(LED_GREEN); // Green LED
#endif
}

void receivingWindowsISR(){
    // Not implemented yet
}

#endif
