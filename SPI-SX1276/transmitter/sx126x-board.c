/*!
 * \file      sx1262dvk1das-board.c
 *
 * \brief     Target board SX1262DVK1DAS shield driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <stdlib.h>
#include <stdint.h>
#include "ttn/utilities.h"
#include "../mcuspecific/mcu.h"
//#include "radio.h"
#include "sx126x-board.h"
#include "sx126x.h"
#include <driverlib.h>
#include "mcuspecific/spi.h"
#include "def.h"
#include "custom.h"

/*!
 * Antenna switch GPIO pins objects
 */
//Gpio_t AntPow;
//Gpio_t DeviceSel;

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_RADIO_DEBUG )
Gpio_t DbgPinTx;
Gpio_t DbgPinRx;
#endif

void SX126xIoInit( void )
{
    //GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    //GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    //GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    //GpioInit( &DeviceSel, RADIO_DEVICE_SEL, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

#if defined( USE_RADIO_DEBUG )
    GpioInit( &DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

/*void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
    GpioSetInterrupt( &SX126x.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, dioIrq );
}*/

/*void SX126xIoDeInit( void )
{
    GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}*/

uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return 0; //BOARD_TCXO_WAKEUP_TIME;
}

void SX126xReset( void )
{
    // Reset on P1.4
    mcu_delayms(10);

    GPIO_setAsOutputPin(SX_RESET_PIN);                             // Reset
    GPIO_setOutputLowOnPin(SX_RESET_PIN);

    mcu_delayms(20);

    GPIO_setOutputHighOnPin(SX_RESET_PIN);
    GPIO_setAsInputPinWithPullUpResistor(SX_RESET_PIN);            // Reset

    mcu_delayms(10);
}

void SX126xWaitOnBusy( void )
{
    while(GPIO_getInputPinValue(BUSY_PIN) != 0);
}

void SX126xWakeup( void )
{
    //CRITICAL_SECTION_BEGIN( );

    spi_chipEnable();

    spi_transfer(RADIO_GET_STATUS);
    spi_transfer(0x00);

    spi_chipDisable();

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    //CRITICAL_SECTION_END( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    uint16_t i = 0;

    SX126xCheckDeviceReady( );

    spi_chipEnable();

    spi_transfer((uint8_t)command );

    for( i = 0; i < size; i++ )
    {
        spi_transfer(buffer[i]);
    }

    spi_chipDisable();

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    uint16_t i = 0;

    SX126xCheckDeviceReady( );

    spi_chipEnable();

    spi_transfer((uint8_t)command );

    spi_transfer(0); // NOP
    for( i = 0; i < size; i++ )
    {
        spi_transfer(0x00);
        buffer[i] = spi_buf;
    }

    spi_chipDisable();

    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    uint16_t i = 0;

    SX126xCheckDeviceReady( );

    spi_chipEnable();
    
    spi_transfer(RADIO_WRITE_REGISTER);
    spi_transfer( ( address & 0xFF00 ) >> 8 );
    spi_transfer( address & 0x00FF );
    
    for( i = 0; i < size; i++ )
    {
        spi_transfer( buffer[i] );
    }

    spi_chipDisable();

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    uint16_t i = 0;

    SX126xCheckDeviceReady( );

    spi_chipEnable();

    spi_transfer( RADIO_READ_REGISTER );
    spi_transfer( ( address & 0xFF00 ) >> 8 );
    spi_transfer( address & 0x00FF );
    spi_transfer(0);
    for( i = 0; i < size; i++ )
    {
        spi_transfer(0);
        buffer[i] = spi_buf;
    }
    spi_chipDisable();

    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    uint16_t i = 0;

    SX126xCheckDeviceReady( );

    spi_chipEnable();

    spi_transfer( RADIO_WRITE_BUFFER );
    spi_transfer( offset );
    for( i = 0; i < size; i++ )
    {
        spi_transfer( buffer[i] );
    }
    spi_chipDisable();

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    uint16_t i = 0;

    SX126xCheckDeviceReady( );

    spi_chipEnable();

    spi_transfer( RADIO_READ_BUFFER );
    spi_transfer( offset );
    spi_transfer( 0 );
    for( i = 0; i < size; i++ )
    {
        spi_transfer(0);
        buffer[i] = spi_buf;
    }
    spi_chipDisable();

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetDeviceId( void )
{
    //if( GpioRead( &DeviceSel ) == 1 )
    //{
    //    return SX1261;
    //}
    //else
    //{
    //    return SX1262;
    //}

#ifdef SX1261_CHIP
    return SX1261;
#else
    return SX1262;
#endif
}

void SX126xAntSwOn( void )
{
    GPIO_setOutputHighOnPin(ANT_SW_PIN);
}

void SX126xAntSwOff( void )
{
    GPIO_setOutputLowOnPin(ANT_SW_PIN);
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

#if defined( USE_RADIO_DEBUG )
void SX126xDbgPinTxWrite( uint8_t state )
{
    GpioWrite( &DbgPinTx, state );
}

void SX126xDbgPinRxWrite( uint8_t state )
{
    GpioWrite( &DbgPinRx, state );
}
#endif
