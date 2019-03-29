#include <msp430.h>
#include <stdint.h>
#include <driverlib.h>
#include "def.h"
#include "spi.h"
#include "stdio.h"
#include "../transmitter/custom.h"

volatile uint8_t spi_buf = 0;





void initSPI(){

    // Configure SPI pins
    // Configure Pins for UCB0CLK

    /*

    * Select Port 2
    * Set Pin 2 to input Secondary Module Function, (UCB0CLK).
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
    	SPIPORTCLK,
		SPICLK,
        GPIO_SECONDARY_MODULE_FUNCTION
    );
    // Configure Pins for UCB0TXD/UCB0SIMO, UCB0RXD/UCB0SOMI
    //Set P1.6, P1.7 as Secondary Module Function Input.
    /*

    * Select Port 1
    * Set Pin 6, 7 to input Secondary Module Function, (UCB0TXD/UCB0SIMO, UCB0RXD/UCB0SOMI).
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
    	SPIPORTDATA,
		SPITX + SPIRX,
        GPIO_SECONDARY_MODULE_FUNCTION
    );

    //Initialize Master
    EUSCI_B_SPI_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = CS_getSMCLK();
    param.desiredSpiClock = 8000000; // 8MHz
    param.msbFirst = EUSCI_B_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
#ifndef SX1276_CHIP
    param.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
#else
    param.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
#endif
    param.spiMode = EUSCI_B_SPI_3PIN;
    EUSCI_B_SPI_initMaster(EUSCI_B0_BASE, &param);

    //Enable SPI module
    EUSCI_B_SPI_enable(EUSCI_B0_BASE);
    //myUart_writeBuf( BACKCHANNEL_UART, "SPI Enabled", NULL, CRLF);

    EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE,
            EUSCI_B_SPI_RECEIVE_INTERRUPT);

    // Enable USCI_B0 RX interrupt
    // EUSCI_B_SPI_enableInterrupt(EUSCI_B0_BASE,
    //                     EUSCI_B_SPI_RECEIVE_INTERRUPT);

    // Enable USCI_B0 TX interrupt
    //EUSCI_B_SPI_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT); ###

    //Wait for slave to initialize
    __delay_cycles(100);

    //TXData = 0x1;                             // Holds TX data

    //USCI_B0 TX buffer ready?
    //while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
    //          EUSCI_B_SPI_TRANSMIT_INTERRUPT)) ;
}

void spi_txready() {
    //USCI_B0 TX buffer ready?
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
              EUSCI_B_SPI_TRANSMIT_INTERRUPT)) ;
}

void spi_rxready() {
    //USCI_B0 RX Received?
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
              EUSCI_B_SPI_RECEIVE_INTERRUPT)) ;
}

void spi_send(uint8_t data) {
  spi_txready();
  EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, data);
}

void spi_recv() {
  spi_rxready();
  spi_buf = EUSCI_B_SPI_receiveData(EUSCI_B0_BASE);         // Store received data
}

void spi_transfer(uint8_t data) {
  spi_send(data);
  spi_recv();
}

void spi_transfer_short(uint8_t data) {
  EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, data);
  //spi_buf = EUSCI_B_SPI_receiveData(EUSCI_B0_BASE);
}

void spi_chipEnable() {
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5);
}

void spi_chipDisable() {
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN5);
}
