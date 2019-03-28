/* --COPYRIGHT--,BSD
 * Copyright (c) 2019
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of the university nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//! LoRaWAN - SX1276
//! This code implements the LoRaWAN protocol on the SX1276 module.
//!
//!             Tested on MSP430FR5969
//!                 -----------------
//!            /|\ |                 |
//!             |  |                 |
//!    Master---+->|RST              |
//!                |                 |
//!                |             P1.6|-> Data Out (UCB0MOSI)
//!                |                 |
//!                |             P1.7|<- Data In (UCB0MISO)
//!                |                 |
//!                |             P2.2|-> Serial Clock Out (UCB0CLK)
//!                |                 |
//!                |             P1.5|-> Slave Select Out (NSS)
//!                |                 |
//!                |             P1.4|-> Slave Reset Out (RST) (optional)
//!                |                 |
//!                |             P1.3|<- Interrupt Request (DIO0) (optional, TxDone, RxDone)
//!
//!
//! This code uses the above mentioned peripherals and I/O signals. You must
//! review these and change as needed for your own board.
//!
//! The network key, application key and device address should be specified in
//! the custom.c file.
//!
//!
//*****************************************************************************

#include "def.h"

#include <clocks.h>
#include <driverlib.h>
#include <gpio.h>
#include <uart.h>
#include "stdio.h"

#include <stdint.h>
#include "mcu.h"
#include "spi.h"

#include "transmitter/custom.h"
#include "transmitter/transmitter.h"



#define BUFFER_SIZE                       64 // Define the payload size here
uint8_t buffer[BUFFER_SIZE];


#define COMPARE_VALUE 624     // CCR0 value for timer interruption, @8MHz: 312 = 1 second

char gStr[30];

void initGPIO(void);
void initTimer1A();

void main(void)
{
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Initialize GPIOs
    initGPIO();

    // Initialize clocks
    initClocks();

    // Initialize SPI
    initSPI();

    // Initialize Timer 1A
    initTimer1A();

#ifdef UART
    // Initialize UART (for backchannel communications)
    if ( myUart_init( BACKCHANNEL_UART, 9600, &myUart_Param_9600_8N1_SMCLK8MHz ) >> 0 ){
        return;
    }else{
        myUart_writeBuf( BACKCHANNEL_UART, "UART Initialized", NULL, CRLF);
    }

    sprintf(gStr, "TX Output Power: %d", TX_OUTPUT_POWER );
    myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

    sprintf(gStr, "LORA Spreading Factor: %d", LORA_SPREADING_FACTOR );
    myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)gStr, NULL, CRLF );

    myUart_writeBuf( BACKCHANNEL_UART, "--", NULL, CRLF );
#endif

   rf_init_lora();
   //--------- END INIT --------------------------------


   //------- MAIN LOOP ------------------------
   while(1){
       __bis_SR_register(LPM3_bits + GIE);   // Enter LPM3
       __no_operation();   // For debugger
   }
}



//---------------------------------- TIMERS -----------------------------------
void initTimer1A(){
    //Start timer in continuous mode sourced by SMCLK
    Timer_A_initContinuousModeParam initContParam = {0};
    initContParam.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initContParam.timerClear = TIMER_A_DO_CLEAR;
    initContParam.startTimer = false;
    Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

    //Initiaze compare mode
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0
        );

    Timer_A_initCompareModeParam initCompParam = {0};
    initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompParam.compareValue = COMPARE_VALUE; //2500 * 10kHz * 1:32 ~= 8 seconds
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);

    Timer_A_startCounter( TIMER_A1_BASE,
            TIMER_A_CONTINUOUS_MODE
                );
}

//******************************************************************************
//
//Interrupt vector service routine for Timer1_A0.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR (void)
{

#ifdef TIME_MEASUREMENT
    GPIO_setOutputHighOnPin(TM1_PIN);   //Wake Up Time - High
#endif

    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0)
            + COMPARE_VALUE;

    TestTransmission(); // Send the specified amount of bytes as payload

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
        compVal
        );

#ifdef TIME_MEASUREMENT
    GPIO_setOutputLowOnPin(TM1_PIN);   //Wake Up Time - Low
#endif

    // Enter LPM3 again, is it automatically made when exiting interrupt, if in LPM3 before
}
