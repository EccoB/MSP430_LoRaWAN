/*
 * custom.c
 *
 *  Created on: 29.08.2018
 *      Author: Armando Miguel
 */
#include "custom.h"
#include "../ttn/LoRaMacCrypto.h"
#include "sx126x.h"
#include "transmitter.h"
#include <driverlib.h>
#include "../def.h"

#ifndef SX1276_CHIP

//-------- Configuration - change here ----------------------------------------
const uint8_t net_key[] = {0xED, 0xFA, 0xDF, 0xCB, 0xE7, 0xE2, 0x85, 0x0E, 0x3E, 0x67, 0x8F, 0x23, 0x61, 0x51, 0xE6, 0x73};
const uint8_t app_key[] = {0x9E, 0xC9, 0xED, 0x9F, 0xEB, 0xDF, 0x15, 0x55, 0xF9, 0x0F, 0x20, 0x47, 0x76, 0x60, 0x70, 0x4E};
const uint32_t device_address = 0x26011605;
//----------------------------------------------------------------------------

extern uint8_t receivingWindow; // 1-first receiving window, 2-second receiving window
extern ModulationParams_t modulationParams;
extern PacketParams_t packetParams;

uint8_t my_data[DATA_BUFFER_SIZE] = {0};
uint32_t my_mic = 0x00000000;
uint8_t my_enc[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define DIR_UPLINK          0
#define DIR_DOWNLINK        1


#define LORAWANOVERHEAD 9
#define MAXPACKETLENGTH 20


uint8_t transmit[]     ={0x8E,0x00,0x8D,0x00,0x01,0x00,0x81,0x81,0x80,0x4C,0x6F,0x52,0x61,0x50,0x49,0x4E,0x47,0x31,0x01,0x00,0x81,0x83};
//declared in header:
uint8_t ttn[TTNASIZE] ={0x80,0x40,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t spi_commands[SPIASIZE] ={0x8D,0x00,0x81,0x81,0x81,0x83};

uint8_t dummy_buffer[256];

void setReceivingWindow( uint8_t window){
    if (window == 1){
        packetParams.Params.LoRa.CrcMode = LORA_CRC_OFF; // Downlink messages have NO CRC !
        packetParams.Params.LoRa.InvertIQ = LORA_IQ_INVERTED;
        packetParams.Params.LoRa.PreambleLength = LORA_PREAMBLE_LENGTH_RX;

        SX126xSetPacketParams(&packetParams);
    }else if(window == 2){
        SX126xSetRfFrequency(869525000);  // Default frequency for RX2

        modulationParams.Params.LoRa.SpreadingFactor = LORA_SF9; // TTN uses SF9 as default for RX2, instead of SF12 as in LoRaWAN specification

        packetParams.Params.LoRa.CrcMode = LORA_CRC_OFF; // Downlink messages have NO CRC !
        packetParams.Params.LoRa.InvertIQ = LORA_IQ_INVERTED;
        packetParams.Params.LoRa.PreambleLength = LORA_PREAMBLE_LENGTH_RX;

        SX126xSetModulationParams(&modulationParams);
        SX126xSetPacketParams(&packetParams);
    }
}

void receivingWindowsISR(){
    if (receivingWindow == 1){
        setReceivingWindow(receivingWindow);
        SX126xSetDioIrqParams( IRQ_RX_DONE,
                               IRQ_RX_DONE,
                               IRQ_RADIO_NONE,
                               IRQ_RADIO_NONE );
        SX126xSetRx(RX1_TIMEOUT_VALUE); // Rx mode with timeout
        receivingWindow = 2; // Prepares for the next receiving window
    #ifdef UART
        myUart_writeBuf( BACKCHANNEL_UART, "RX1 open", NULL, CRLF );
    #endif
    }else if (receivingWindow == 2){
        setReceivingWindow(receivingWindow);
        SX126xSetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,     // RX2 needs the Timeout IRQ in order to non-block the program
                               IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                              IRQ_RADIO_NONE,
                              IRQ_RADIO_NONE );
        SX126xSetRx(RX2_TIMEOUT_VALUE); // Rx mode with timeout
        Timer_A_stop( TIMER_A1_BASE); // No more receiving windows until the next transmission
    #ifdef UART
        myUart_writeBuf( BACKCHANNEL_UART, "RX2 open", NULL, CRLF );
    #endif
    }
}

void LoRaMacPackage(){
    uint8_t cnt = 0;

    // Insert Device Address into ttn array
    ttn[2] = (uint8_t)((device_address) & 0xFF);         // LSB
    ttn[3] = (uint8_t)((device_address >> 8) & 0xFF);
    ttn[4] = (uint8_t)((device_address >> 16) & 0xFF);
    ttn[5] = (uint8_t)((device_address >> 24) & 0xFF);   // MSB

    LoRaMacPayloadEncrypt(&my_data[0], PAYLOAD_SIZE, app_key, device_address, DIR_UPLINK, 0, &my_enc[0]);

    for (cnt = 0; cnt < PAYLOAD_SIZE; cnt++){
      ttn[cnt + 10] = my_enc[cnt];  // FRMPayload always begins at position 11
    }
    LoRaMacComputeMic(&ttn[1], MESSAGE_SIZE, net_key, device_address, DIR_UPLINK, 0, &my_mic);
    ttn[MESSAGE_SIZE+1] = (uint8_t)((my_mic) & 0xFF);         // LSB, MESSAGE_SIZE + 1* is used due to the first byte (FIFO-Write Command) in array ttn
    ttn[MESSAGE_SIZE+2] = (uint8_t)((my_mic >> 8) & 0xFF);
    ttn[MESSAGE_SIZE+3] = (uint8_t)((my_mic >> 16) & 0xFF);
    ttn[MESSAGE_SIZE+4] = (uint8_t)((my_mic >> 24) & 0xFF);   // MSB
}


void SendPing(void (*recCallback)()){ //(uint8_t *buffer, uint8_t length)) {
#ifdef TTN
  uint8_t cnt_frame = 0;

  LoRaMacPackage(); // The message to be send is encrypted and stored in ttn[]

  my_data[PAYLOAD_SIZE - 1]++;    // Modify the last byte of the message to be send
  for (cnt_frame = (PAYLOAD_SIZE - 1); cnt_frame > 0; cnt_frame--){
    if (my_data[cnt_frame] == 0){
      my_data[cnt_frame - 1]++;     // Increment the next byte
    }else{
      break;
    }
  }
#endif

#ifndef SPIARRAY

#ifdef TTN
  //rf_transmitReceiveB(&ttn[1], PHY_PAYLOAD_SIZE, recCallback);
  rf_transmit(&ttn[1], PHY_PAYLOAD_SIZE, recCallback);

  //SX126xSendPayload(&ttn[1], PHY_PAYLOAD_SIZE, 0);  // Timeout disabled; Send using the library, first byte of ttn is ommited (it contains the write-FIFO command)
#else
  rf_transmit(&my_data[0], PAYLOAD_SIZE, recCallback);

  //SX126xSendPayload(&my_data[0], PAYLOAD_SIZE);    // Send using the library, first byte of ttn is ommited (it contains the write-FIFO command)
#endif

#else
  //Hard-coded SPI:

  //REG_LR_FIFOADDRPTR = 0
  spi_chipEnable();
  spi_transfer_short(spi_commands[0]);
  spi_transfer_short(spi_commands[1]);
  spi_chipDisable();

  //REG_OPMODE = 0x81 = LoRa+Standby
  spi_chipEnable();
  spi_transfer_short(spi_commands[2]);
  spi_transfer_short(spi_commands[3]);
  spi_chipDisable();

  //Write FIFO
  //TTN Command + Payload
  spi_chipEnable();
#ifdef TIME_MEASUREMENT
      GPIO_toggleOutputOnPin(TM1_PIN);   // FIFO-Write Time (Beginning)
#endif

#ifdef TTN
  uint8_t cnt = 0;
  for (cnt = 0; cnt < ARRAY_TRANSFER_SIZE; cnt++){
    spi_transfer_short(ttn[cnt]);
  }
#else
  uint8_t cnt = 0;
  spi_transfer_short(0x80); // Write-FIFO Command
  for (cnt = 0; cnt < PAYLOAD_SIZE; cnt++){
    spi_transfer_short(my_data[cnt]);
  }
#endif
  spi_chipDisable();

#ifndef TIME_MEASUREMENT
      GPIO_toggleOutputOnPin(TM1_PIN);   // FIFO-Write Time (End)
#endif

  //set (RF_OPMODE_TRANSMITTER)
  spi_chipEnable();
  spi_transfer_short(spi_commands[4]);
  spi_transfer_short(spi_commands[5]);
  spi_chipDisable();

  SX126xSetOperatingMode( MODE_TX ); // ### Specific for SX126x

#ifdef TIME_MEASUREMENT    // Sending time for Hard-coded SPI, for Full-Library it is made in send()
  GPIO_setOutputHighOnPin(TM2_PIN);   // Sending Time - High
#endif

#endif

#ifdef LEDS
  GPIO_toggleOutputOnPin(LED_GREEN); // Green LED
#endif
}


/* sendLoRaWAN()
 * Needs:
 *  -   app_key
 *  -   device_address
 * Input:
 *  -   data:   data that should be encoded
 *  -   length: length of data packet
 */
void sendLoRaWAN(uint8_t *data, uint8_t length){

    uint8_t tosend[MAXPACKETLENGTH+LORAWANOVERHEAD+4];
    static uint8_t framecounter=0;
    uint8_t port=0x01;
    tosend[0]=0x40;                                     //For TTN
    // Insert Device Address into ttn array
    tosend[1] = (uint8_t)((device_address) & 0xFF);         // LSB
    tosend[2] = (uint8_t)((device_address >> 8) & 0xFF);
    tosend[3] = (uint8_t)((device_address >> 16) & 0xFF);
    tosend[4] = (uint8_t)((device_address >> 24) & 0xFF);   // MSB

    //Insert non static fields:
    tosend[5] = 0x80;       //Frame Control: No Adaptive Data Rate, NO Ack, no frame pending
    tosend[6] = framecounter;
    tosend[7] = 0x00;       //FOptions
    tosend[8] = port;       //Port


    LoRaMacPayloadEncrypt(&data[0], length, app_key, device_address, DIR_UPLINK, 0, &tosend[9]);
    //Bug: seems like the conversion to uint_32 only allows
    uint32_t mic=0xFF;
    LoRaMacComputeMic(&tosend[0], length + LORAWANOVERHEAD , net_key, device_address, DIR_UPLINK, 0, &mic);
    int t=0;
    for(t=0; t<4;t++){
        tosend[length+LORAWANOVERHEAD+t] =(uint8_t)((mic >> (t*8)& 0xFF) );
    }


    // ---- Now all the needed stuff in in the array from position 0 to length+LORAWANOVERHEAD(9)+4 -1(index)
    rf_transmit(&tosend[0], length+LORAWANOVERHEAD+4, 0);

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

#endif
