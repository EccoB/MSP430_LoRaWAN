/*
 * transmitter.c
 *
 *  Created on: 07.08.2018
 *      Author: baeumker
 */
#include "transmitter.h"
#include "sx1276regs-lora.h"
#include "sx1276.h"
#include "custom.h"

static radio_events_t radio_events;






//---------- Functions for reaction of different events --------------
// Should be later be put in other places
void OnTxDone() {
}
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
}
void OnRxError() {
}
//----------------------------------------------------------------------

// ---- Own fields ---------
char paConfig = 0;

//----- Own functions -----------
void rf_waitTillIdle(){
	while(sx1276.Settings.State != RF_IDLE);
}

void rf_setPAPower(int pa_power){
#ifdef RFM95
            if( pa_power > 17 ) pa_power = 2;
            paConfig = sx1276_read(REG_LR_PACONFIG);
            paConfig = (paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t) ((uint16_t) (pa_power - 2) & 0x0F);
            sx1276_write(REG_LR_PACONFIG, paConfig);
#else
            if( pa_power > 14 ) pa_power = -1;
            paConfig = sx1276_read(REG_LR_PACONFIG);
            paConfig = (paConfig & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t) ((uint16_t) (pa_power + 1) & 0x0F);
            sx1276_write(REG_LR_PACONFIG, paConfig);
#endif
}

void rf_setSpreadingF(int sf){
	if(sf > 12)
		sf = 7;

	char LowDatarateOptimize = 0;
	            if((sf == 11) || (sf == 12)) {
	              LowDatarateOptimize = 0x01;
	            } else {
	              LowDatarateOptimize = 0x00;
	            }
	            sx1276_write(REG_LR_MODEMCONFIG2, (sx1276_read(REG_LR_MODEMCONFIG2) & RFLR_MODEMCONFIG2_SF_MASK) | (sf << 4));
	            sx1276_write(REG_LR_MODEMCONFIG3, (sx1276_read(REG_LR_MODEMCONFIG3) & RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) | (LowDatarateOptimize << 3));
}

//---------------



/*
 * Initialises all necessary stuff for the transmitter
 */
void rf_init_lora() {
  radio_events.TxDone = OnTxDone;
  radio_events.RxDone = OnRxDone;
  //radio_events.TxTimeout = OnTxTimeout;
  //radio_events.RxTimeout = OnRxTimeout;
  radio_events.RxError = OnRxError;

  sx1276_init(radio_events);
  sx1276_set_channel(RF_FREQUENCY);


  sx1276_set_txconfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
#ifndef REDUCED
  sx1276_set_rxconfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
#endif
  // Full buffer used for Tx
  sx1276_write(REG_LR_FIFOTXBASEADDR, 0);
  sx1276_write(REG_LR_FIFOADDRPTR, 0);

  // Channel for LoRaWAN network
  sx1276_write(REG_LR_SYNCWORD, 0x34);
  sx1276_write(REG_LR_FRFMSB, 0xD8);
  sx1276_write(REG_LR_FRFMID, 0xEC);
  sx1276_write(REG_LR_FRFLSB, 0xCC);

#ifdef REDUCED
  // One-time Pre-Configurations for Sending ------------------------------------------------------
  // Write: 5 Bytes --> 10 Bytes total as every time address + Byte
  // Read: 1 Byte -> 2 Bytes for reading
  //....... .> 12 Bytes, everytime CS
  sx1276_write(REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
  sx1276_write(REG_LR_INVERTIQ, 0x67); // InvertIQ Enabled (bit 6) | default_value (0x27) = 0x67
#ifdef TTN
  sx1276.Settings.LoRaPacketHandler.Size = PHY_PAYLOAD_SIZE;
  sx1276_write(REG_LR_PAYLOADLENGTH, PHY_PAYLOAD_SIZE); // Initializes the payload size
#else
  sx1276.Settings.LoRaPacketHandler.Size = PAYLOAD_SIZE;
  sx1276_write(REG_LR_PAYLOADLENGTH, PAYLOAD_SIZE); // Initializes the payload size
#endif

  sx1276_write(REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
                                    RFLR_IRQFLAGS_RXDONE |
                                    RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                    RFLR_IRQFLAGS_VALIDHEADER |
                                    //RFLR_IRQFLAGS_TXDONE |
                                    RFLR_IRQFLAGS_CADDONE |
                                    RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                    RFLR_IRQFLAGS_CADDETECTED );
  // DIO0=TxDone
  sx1276_write(REG_LR_DIOMAPPING1, (sx1276_read(REG_LR_DIOMAPPING1) & RFLR_DIOMAPPING1_DIO0_MASK) | RFLR_DIOMAPPING1_DIO0_01);
  //-------------------------------------------------------------------------------------------
#endif

  sx1276_set_opmode(RFLR_OPMODE_SLEEP);

}
