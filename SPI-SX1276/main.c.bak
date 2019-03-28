/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
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
 * *  Neither the name of Texas Instruments Incorporated nor the names of
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
//! USCI_B0, SPI 3-Wire Master Incremented Data
//! This example shows how SPI master talks to SPI slave using 3-wire mode.
//! Incrementing data is sent by the master starting at 0x01. Received data is
//! expected to be same as the previous transmission.  eUSCI RX ISR is used to
//! handle communication with the CPU, normally in LPM0. If high, P1.0 indicates
//! valid data reception.  Because all execution after LPM0 is in ISRs,
//! initialization waits for DCO to stabilize against ACLK.
//! ACLK = ~32.768kHz, MCLK = SMCLK = DCO ~ 1048kHz.  BRCLK = SMCLK/2
//!
//! Use with SPI Slave Data Echo code example.  If slave is in debug mode, P1.1
//! slave reset signal conflicts with slave's JTAG; to work around, use IAR's
//! "Release JTAG on Go" on slave device.  If breakpoints are set in
//! slave RX ISR, master must stopped also to avoid overrunning slave
//! RXBUF.
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
//!                |             P1.4|-> Slave Reset Out (RST)
//!
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SPI peripheral
//! - GPIO Port peripheral (for SPI pins)
//! - UCB0SIMO
//! - UCB0SOMI
//! - UCB0CLK
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - USCI_B0_VECTOR
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

uint8_t RXData = 0;
uint8_t TXData = 0;

char gStr[30];

int8_t pa_power = TX_OUTPUT_POWER;
int sf=7;

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

    sprintf(gStr, "TX Output Power: %d", pa_power );
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

#ifndef LOWPOWER
    GPIO_setOutputHighOnPin( GPIO_PORT_P3, GPIO_PIN4 );   //Wake Up Time - High
#endif

    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0)
            + COMPARE_VALUE;

    TestTransmission(); // 8 bytes payload

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
        compVal
        );

#ifndef LOWPOWER
    GPIO_setOutputLowOnPin( GPIO_PORT_P3, GPIO_PIN4 );   //Wake Up Time - Low
#endif

    // Enter LPM3 again, is it automatically made when exiting interrupt
}
