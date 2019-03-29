/*
 * transmitter.c
 *
 *  Created on: 29.08.2018
 *      Author: Armando Miguel
 */
#include "transmitter.h"
#include "sx126x.h"
#include "custom.h"
#include "../def.h"

#ifndef SX1276_CHIP

static RadioEvents_t radio_events;  // Callback functions for OnTxDone, OnRxTimeout, etc.
ModulationParams_t modulationParams;
PacketParams_t packetParams;
uint8_t receivingWindow = 0;    // 1-first receiving window, 2-second receiving window

void (*RxCallback) (uint8_t *buffer, uint8_t length);   // RxCallback returns the received packet and its length
void (*TxCallback) ();

typedef enum
{
    RF_MODE_TX                              = 0x00,         // Receiving windows are not opened after a transmission
    RF_MODE_TX_RX,                                          // Two receiving windows are to be opened after a transmission
    RF_MODE_RX_CONTINUOUS,                                  // If continuous-listen mode is being used, a callback function is called in every RxDone                                           //! The radio is in channel activity detection mode
}rf_operating_modes_t;

rf_operating_modes_t rf_mode;


//---------- Functions for reaction of different events --------------
// Should later be put in other places

void OnTxDone() {
    if (rf_mode == RF_MODE_TX_RX){     // Timer for the receiving windows must be activated after a successful transmission
        Timer_A_clear(TIMER_A1_BASE);
        Timer_A_setCompareValue(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0, COMPARE_VALUE);
        Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_CONTINUOUS_MODE);
        receivingWindow = 1; // First receiving window (same configuration as uplink)
    } else if (rf_mode == RF_MODE_TX){
        if (TxCallback != NULL){
            TxCallback();
        }
    }
    SX126xSetOperatingMode( MODE_STDBY_RC );

#ifdef UART
    myUart_writeBuf( BACKCHANNEL_UART, "TxDone", NULL, CRLF );
#endif
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  Timer_A_stop( TIMER_A1_BASE); // No more receiving windows until the next transmission

  if (rf_mode == RF_MODE_RX_CONTINUOUS){       // Callback function needs to be called, operating mode remains in MODE_RX
      if (RxCallback != NULL){
          RxCallback(&payload[0], size);
      }
  }else if (rf_mode == RF_MODE_TX_RX){         // Callback function needs to be called and operating mode changed to Standby
      SX126xSetOperatingMode( MODE_STDBY_RC );
      if (RxCallback != NULL){
          RxCallback(&payload[0], size);
      }
  }

#ifdef UART     // Write the received packet and its properties through the UART Backchannel
  char counter = 0;

  myUart_writeBuf( BACKCHANNEL_UART, "RxDone:", NULL, CRLF );

  sprintf(gStr, "RSSI: %d", rssi );
  myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

  sprintf(gStr, "SNR: %d", snr );
  myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

  sprintf(gStr, "Size: %d", size );
  myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

  for(counter = 0; counter < size - 1; counter++){
      if(payload[counter] != 0){
          sprintf(gStr, "%c",payload[counter]);
      }else{
          sprintf(gStr, "%d",payload[counter]);
      }
      myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, NOCRLF );
  }
  sprintf(gStr, "%c",payload[size - 1]);  //Print last character
  myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );
  myUart_writeBuf( BACKCHANNEL_UART, "------------", NULL, CRLF );    //Separate received packets
#endif
}

void OnRxError() {
#ifdef UART
    myUart_writeBuf( BACKCHANNEL_UART, "RxError", NULL, CRLF );
#endif
}

void OnRxTimeout() {
    if (rf_mode == RF_MODE_TX_RX){       // Callback function needs to be called on Timeout too
        SX126xSetOperatingMode( MODE_STDBY_RC );
        if (RxCallback != NULL){
            RxCallback(0, 0); // Calls the function with length and buffer-pointer equal 0
        }
    }

#ifdef UART
    myUart_writeBuf( BACKCHANNEL_UART, "RxTimeout", NULL, CRLF );
    myUart_writeBuf( BACKCHANNEL_UART, "------------", NULL, CRLF );    //Separate received packets
#endif
}

void OnTxTimeout() {
    SX126xSetOperatingMode( MODE_STDBY_RC );

#ifdef UART
    myUart_writeBuf( BACKCHANNEL_UART, "TxTimeout", NULL, CRLF );
    myUart_writeBuf( BACKCHANNEL_UART, "------------", NULL, CRLF );    //Separate received packets
#endif
}

//----------------------------------------------------------------------

// ---- Own fields ---------


//----- Own functions -----------
void rf_waitTillIdle(){
    while((SX126xGetOperatingMode() != MODE_STDBY_RC) && (SX126xGetOperatingMode() != MODE_STDBY_XOSC));
}

void rf_turnOff(uint8_t state){
    SleepParams_t params = {0};

    switch (state){    // Lowest possible Power-Mode
    case 0:
        SX126xSetSleep(params);  // Cold start + Timeout disabled
        break;
    case 1:
        params.Fields.WarmStart = 1;
        SX126xSetSleep(params);  // Warm start (data retention) + Timeout disabled
        break;
    default:
        break;
    }
}

void rf_setPAPower(int8_t tx_power){
    SX126xSetTxParams(tx_power, RADIO_RAMP);
}

void rf_setSpreadingF(uint8_t sf){
    if (sf > 12) sf = 12;
    if (sf < 5) sf = 5;
    modulationParams.Params.LoRa.SpreadingFactor = (RadioLoRaSpreadingFactors_t)sf;
    SX126xSetModulationParams(&modulationParams);
}

//---------------

/*
 * Initialises all necessary stuff for the transmitter
 */
void rf_init_lora() {
  radio_events.TxDone = OnTxDone;
  radio_events.RxDone = OnRxDone;
  radio_events.TxTimeout = OnTxTimeout;
  radio_events.RxTimeout = OnRxTimeout;
  radio_events.RxError = OnRxError;

  modulationParams.PacketType = PACKET_TYPE_LORA;
  modulationParams.Params.LoRa.Bandwidth = (RadioLoRaBandwidths_t)LORA_BANDWIDTH;
  modulationParams.Params.LoRa.CodingRate = (RadioLoRaCodingRates_t)LORA_CODINGRATE; // The error correction code does not have to be known in advance by the receiver since it is encoded in the header part of the packet
  modulationParams.Params.LoRa.SpreadingFactor = (RadioLoRaSpreadingFactors_t)LORA_SPREADING_FACTOR;

  if( ( ( LORA_BANDWIDTH == LORA_BW_125 ) && ( ( LORA_SPREADING_FACTOR == LORA_SF11 ) || ( LORA_SPREADING_FACTOR == LORA_SF12 ) ) ) ||
  ( ( LORA_BANDWIDTH == LORA_BW_250 ) && ( LORA_SPREADING_FACTOR == LORA_SF12 ) ) )
  {
      modulationParams.Params.LoRa.LowDatarateOptimize = 0x01;  // LowDatarateOptimize is On
  }
  else
  {
      modulationParams.Params.LoRa.LowDatarateOptimize = 0x00;  // LowDatarateOptimize is Off
  }

  packetParams.PacketType = PACKET_TYPE_LORA;
  packetParams.Params.LoRa.CrcMode = (RadioLoRaCrcModes_t)LORA_CRC_ACTIVE;
  packetParams.Params.LoRa.HeaderType = (RadioLoRaPacketLengthsMode_t)LORA_FIX_LENGTH_PAYLOAD_ON; // Explicit says: number of bytes, coding rate and whether a CRC is used in the packet
  packetParams.Params.LoRa.InvertIQ = (RadioLoRaIQModes_t)LORA_IQ_INVERSION_ON;
  packetParams.Params.LoRa.PreambleLength = LORA_PREAMBLE_LENGTH_TX; // default is 12
#ifdef TTN
  packetParams.Params.LoRa.PayloadLength = PHY_PAYLOAD_SIZE;    // Adds the overhead of LoRaWAN to PAYLOAD_SIZE (13 Bytes)
#else
  packetParams.Params.LoRa.PayloadLength = PAYLOAD_SIZE;
#endif

  SX126xInit(&radio_events); // Reset + WakeUp + Standby + Callbacks
  SX126xSetStandby( STDBY_RC );
  SX126xSetPacketType(PACKET_TYPE_LORA);
  SX126xSetRfFrequency(RF_FREQUENCY);
  SX126xSetRegulatorMode( USE_DCDC ); // LDO is default
  SX126xSetTxParams(TX_OUTPUT_POWER, RADIO_RAMP);
  SX126xSetModulationParams(&modulationParams);
  SX126xSetPacketParams(&packetParams);

  SX126xSetStopRxTimerOnPreambleDetect(true);

  uint8_t Sync[2];
  Sync[0] = 0x34;   // LoRaWAN for public networks (0x3444), unused bytes should be set to 0x55
  Sync[1] = 0x55;
  SX126xWriteRegisters(REG_LR_SYNCWORD, &Sync[0], 2);   // LoRaWAN for public networks
}

void rf_re_init_lora() {
    SX126xSetRfFrequency(RF_FREQUENCY);
    modulationParams.Params.LoRa.SpreadingFactor = (RadioLoRaSpreadingFactors_t)LORA_SPREADING_FACTOR;
    packetParams.Params.LoRa.CrcMode = (RadioLoRaCrcModes_t)LORA_CRC_ACTIVE;
    packetParams.Params.LoRa.InvertIQ = (RadioLoRaIQModes_t)LORA_IQ_INVERSION_ON;
    packetParams.Params.LoRa.PreambleLength = LORA_PREAMBLE_LENGTH_TX; // default is 12

    SX126xSetModulationParams(&modulationParams);
    SX126xSetPacketParams(&packetParams);
}

//---------------
/*
 *  Transmission/Reception functions
 */

void rf_transmit(uint8_t *data, uint8_t length, void (*recCallback)()){
    rf_mode = RF_MODE_TX;
    if (receivingWindow != 0){
        rf_re_init_lora();      // Restores the configuration for the transmitter after receiving windows
        receivingWindow = 0;
    }
    TxCallback = recCallback;
    if(packetParams.Params.LoRa.PayloadLength != length){   // If the size of the last packet is different, updates the payloadLength
        packetParams.Params.LoRa.PayloadLength = length;
        SX126xSetPacketParams(&packetParams);
    }
    SX126xSendPayload(&data[0], length, 0);     // Sends without timeout

#ifdef TIME_MEASUREMENT
    GPIO_setOutputHighOnPin(TM2_PIN);  // Sending Time
#endif
}

void rf_transmitReceiveB(uint8_t *dataToSend, uint8_t length, void (*recCallback)(uint8_t *buffer, uint8_t length)){
    rf_mode = RF_MODE_TX_RX;
    if (receivingWindow != 0){
        rf_re_init_lora();      // Restores the configuration for the transmitter after receiving windows
        receivingWindow = 0;
    }
    RxCallback = recCallback;
    if(packetParams.Params.LoRa.PayloadLength != length){   // If the size of the last packet is different, updates the payloadLength
        packetParams.Params.LoRa.PayloadLength = length;
        SX126xSetPacketParams(&packetParams);
    }
    SX126xSendPayload(&dataToSend[0], length, 0);     // Sends without timeout
}

void rf_continuousListen(void (*recCallback)(uint8_t *buffer, uint8_t length)){
    rf_mode = RF_MODE_RX_CONTINUOUS;
    if (receivingWindow != 0){
        rf_re_init_lora();      // Restores the configuration for the transmitter after receiving windows
        receivingWindow = 0;
    }

    SX126xSetDioIrqParams( IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE );
    SX126xSetRx(0xFFFFFF);      // Rx in Continuous mode
    RxCallback = recCallback;
}

#endif
