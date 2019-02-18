// ----------------------------------------------------------------------------
// uart.c   ('FR6989 Launchpad)
// ----------------------------------------------------------------------------

//***** Header Files **********************************************************
#include <uart.h>
#include "string.h"

//***** Defines ***************************************************************
//#define MYUART_IDLE    0
//#define MYUART_WRITE   1
//#define MYUART_ECHO    2

//***** Function Prototypes ***************************************************
//static unsigned char Eol( unsigned char );

//***** Global Variables ******************************************************

static unsigned short txBufLen     = 0;                                         // Number of bytes left to output; decremented as each character is sent

//---------------------------------------------------------------------------------------------
// struct Uart_t myUart;
//
// This structure defines the resources used by the UART functions of the EUSCI_A port.
// The typedef is found in the associated header file (myUart.h). The goal was to try and
// encapsulate the various port settings into a single structure, so as to make it easier to
// port this code to a new device.
// - Some of the fields are are defined by the device (Base Address, Number of UART channels).
// - Others are defined by the hardware board layout - in most cases, the UARTs can actually be
//   assigned to a few different Port/Pin locations.
// - Finally, there are a number of "channel variables" which are used to indicate the status
//   of the port at runtime.
//---------------------------------------------------------------------------------------------
struct Uart_t myUart = {
                         NUM_CHANNELS,                                          // Number of Uarts on the device
                         .Channels[0] = { EUSCI_A0_BASE,                        // Base Address of EUSCI_A port
                                          GPIO_PORT_P2,                         // GPIO Port settings for TX pin
                                          GPIO_PIN0,
                                          GPIO_SECONDARY_MODULE_FUNCTION,
                                          GPIO_PORT_P2,                         // GPIO Port settings for RX pin
                                          GPIO_PIN1,
										  GPIO_SECONDARY_MODULE_FUNCTION,
                                          0,                                    // Baud Rate
                                          0,                                    // Open - UART port has been opened and configured
                                          1,                                    // TxBusy -
                                          0,                                    // TxRDY
                                          1,                                    // RxBusy
                                          0,                                    // RxRDY
                                          NOECHO                                // RxEcho
                                        },
                         .Channels[1] = { EUSCI_A1_BASE,                        // Base Address of EUSCI_A port
                                          GPIO_PORT_P2,                         // GPIO Port settings for TX pin
                                          GPIO_PIN5,
										  GPIO_SECONDARY_MODULE_FUNCTION,
                                          GPIO_PORT_P2,                         // GPIO Port settings for RX pin
                                          GPIO_PIN6,
										  GPIO_SECONDARY_MODULE_FUNCTION,
                                          0,                                    // Baud Rate
                                          0,                                    // Open - UART port has been opened and configured
                                          1,                                    // TxBusy -
                                          0,                                    // TxRDY
                                          1,                                    // RxBusy
                                          0,                                    // RxRDY
                                          DOECHO                                // RxEcho
                                        }
                       };

// The following structure will configure the EUSCI_A port to run at 9600 baud from an 8MHz SMCLK
// The baud rate values were calculated at: http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
EUSCI_A_UART_initParam myUart_Param_9600_8N1_SMCLK8MHz = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,
    52,                                                                         // clockPrescalar
    1,                                                                          // firstModReg
    73,                                                                         // secondModReg
    EUSCI_A_UART_NO_PARITY,
    EUSCI_A_UART_LSB_FIRST,
    EUSCI_A_UART_ONE_STOP_BIT,
    EUSCI_A_UART_MODE,
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

// The following structure will configure the EUSCI_A port to run at 9600 baud from an 32KHz ACLK
// The baud rate values were calculated at: http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
EUSCI_A_UART_initParam myUart_Param_9600_8N1_ACLK32Kz = {
    EUSCI_A_UART_CLOCKSOURCE_ACLK,
    3,                                                                          // clockPrescalar
    0,                                                                          // firstModReg
    146,                                                                        // secondModReg
    EUSCI_A_UART_NO_PARITY,
    EUSCI_A_UART_LSB_FIRST,
    EUSCI_A_UART_ONE_STOP_BIT,
    EUSCI_A_UART_MODE,
    EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION
};

//*****************************************************************************
// myUart_init()
//
// Initialize the UART functionality of the EUSCI peripheral
// - The "myUart_Instance" parameter allows the user to setup any of the UARTs
//   (Two UARTS are available on the 'FR6989)
// - This function verifies that the DriverLib UART init function returns
//   successfully
// - Waiting for the UART to send data can be done with either "polling" or
//   "interrupts"; this init routine enables the EUSCI UART interrupts
//*****************************************************************************
int myUart_init( uint16_t myUart_Instance, uint32_t BaudRate, EUSCI_A_UART_initParam *param )
{
    // Get this channel's EUSCI's base address from myUart structure
    uint16_t BaseAddr = myUart.Channels[myUart_Instance].BaseAddress;

    // Abort and return an error if the channel has already been initialized
    if( myUart.Channels[myUart_Instance].Open )
        return( STATUS_ALREADY_OPEN );

    // Initialize the UART using one of the two sets of parameters provided (or modify the parameters above to meet your needs)
    if( STATUS_FAIL == EUSCI_A_UART_init( BaseAddr, param ))
        return( STATUS_INIT_FAILED );

    // Baud rate retained for future clock adjustment function
    myUart.Channels[myUart_Instance].BaudRate = BaudRate;

    // Enable (i.e. turn on) the UART
    EUSCI_A_UART_enable( BaseAddr );

    // Set the status flags for this instance of our UART channel
    myUart.Channels[myUart_Instance].Open = 1;                                  // Indicate the port has been opened and initialized
    myUart.Channels[myUart_Instance].TxBusy = 0;                                // Set TX port to "not busy" (since we are not actively sending data, yet)
    myUart.Channels[myUart_Instance].RxBusy = 0;                                // Set RX port to "not busy" (since we are not actively receiving data, yet)
    myUart.Channels[myUart_Instance].TxRDY = 1;                                 // TX port is ready by default, since the transmit buffer is empty)
    myUart.Channels[myUart_Instance].RxRDY = 0;                                 // RX port is not ready by default, as there's nothing yet to read

    // Return successful, if we made it this far
    return( STATUS_INIT_SUCCESSFUL );
}


//*****************************************************************************
// myUart_writeBuf()
//
// Configures UART to send a buffer of data
// - The EUSCI_A_UART_transmitData() DriverLib function handles sending one
//   byte of data, whereas this function will send an entire buffer
// - This is a 'blocking' function - that is, it does not exit until the whole
//   buffer has been sent; rather than a wait-loop, though, this function waits
//   using LPM0 (and the transmit interrupt)
// - If the UART is not busy sending data already, this function sends the
//   first byte of data, then lets the UART ISR send the rest of the data
//
// Parameters
// - myUart_Instance:  Let's you select which UART to send the buffer; no error
//                     checking is done, as it is assumed you successfully
//                     initialized the UART
// - *txBuf:           Buffer of data to be written to UART port; function
//                     checks that the buffer does not have a NULL address
// - BufLen:           How many bytes do you want to send (you don't have to
//                     send the whole buffer which was passed). If a value of
//                     zero is passed, the function will calculate the buffer
//                     length for you
// - doCrLf:           Do you want to send a Carriage Return and Linefeed
//                     after the data buffer has been sent? (1 Yes; 0 N0)
// - Return:           This function returns the number of characters sent
//*****************************************************************************
int myUart_writeBuf( uint16_t myUart_Instance, unsigned char *txBuf, uint16_t BufLen, int DoCrLf )
{
    uint16_t BaseAddr = myUart.Channels[myUart_Instance].BaseAddress;           // Get this channel's EUSCI's base address from myUart structure
    unsigned char out = 0;
    int ret = -1;                                                               // Variable which will hold return value for this function

    // Exit if there is no transmit buffer
    if( txBuf == NULL )
        return( STATUS_FAIL_NOBUFFER );

    // Exit the function if we're already busy writing, else set the 'busy' flag
    if ( myUart.Channels[myUart_Instance].TxBusy )
        return( STATUS_FAIL_BUSY );
    else
        myUart.Channels[myUart_Instance].TxBusy = 1;

    // Check if the transmit length was provided as a parameter; if it's zero,
    // calculate the size of the buffer (i.e. number of chars to transmit)
    if ( BufLen == 0 )
        txBufLen = strlen( (const char *)txBuf );
    else
        txBufLen = BufLen;

    // Add to transmit length for carriage return (enter) and linefeed, if requested by user
    if ( DoCrLf )
    {
        txBufLen += 2;
    }

    // Since 'txBufLen' is decremented during transfers, retain the original value
    ret = txBufLen;

    // Enable USCI_Ax TX interrupt
    EUSCI_A_UART_enableInterrupt( BaseAddr, EUSCI_A_UART_TRANSMIT_INTERRUPT);

    // Keep sending characters until complete
    while( txBufLen >> 0 )
    {
        if ( myUart.Channels[myUart_Instance].TxRDY == 1 )                      // Check if TX port is ready (if we got here, it should be ready)
        {
            myUart.Channels[myUart_Instance].TxRDY = 0;                         // Clear the ready bit now that we're planning to send a byte

            out = *txBuf;                                                       // Read from buffer (note that this reads past end of buffer if we're doing CRLF)
            txBuf++;                                                            // Move buffer pointer to next item to be sent

            if ( DoCrLf ) {                                                     // If doing CRLF, replace 'out' with the CR or LF
                if ( txBufLen == 2 )
        		    out = ASCII_LINEFEED;
        	    else if ( txBufLen == 1 )
        		    out = ASCII_ENTER;
            }

            txBufLen--;                                                         // Decrement the transmit count

            EUSCI_A_UART_transmitData( BaseAddr, out );                         // Send the data to the transmit port
            if ( myUart.Channels[myUart_Instance].TxRDY == 0 )                  // Test TxRDY to help prevent race condition where interrupt occurs before we reach this step
                __low_power_mode_0();                                           // Sleep CPU until woken up by transmit ready interrupt event
        }                                                                       // (other LPMx modes could be used, depending upon clock requirements)
    }

    // Disable the UART transmit interrupt
    EUSCI_A_UART_disableInterrupt( BaseAddr, EUSCI_A_UART_TRANSMIT_INTERRUPT);

    // Return the length of the string that was sent
    return (int) ret;
}


//*****************************************************************************
// myUart_readBuf()
//
// This function receives a buffer of data via the UART. This is a blocking
// function, thus it will not return until the full length has been received.
//
// Parameters:
// - myUart_Instance:  Let's you select which UART should recieve the buffer;
//                     no error checking is done, as it is assumed you
//                     successfully initialized the UART
// - *rxBuf:           Buffer that the received data should be written into;
//                     this function checks that the buffer doesn'tt have a
//                     NULL address (if so, it exits
// - rxSize:           How many bytes do you want to receive; a default size
//                     of 1 line (80 bytes) is used if a "0" is passed;
//                     Note that receiving a carriage return or linefeed will
//                     force the function to stop receiving data, even if the
//                     'Length' of bytes has not yet been received
// - Return:           The status of the function
//*****************************************************************************
int myUart_readBuf( uint16_t myUart_Instance, unsigned char *rxBuf, uint16_t rxSize )
{
  uint16_t BaseAddr = myUart.Channels[myUart_Instance].BaseAddress;             // Check if TX port is ready (if we got here, it should be ready)
  unsigned int  ret      = STATUS_INIT_SUCCESSFUL;                              // Variable to hold function status (initialize as 'successful')
  unsigned int  i        = 0;                                                   // Local variable used in 'for' loop
  unsigned char in[3]    = { 0, 0, 0 };                                         // Temporary variable to hold received data (only 1 byte will be used for rec'd data; extra locations to add CRLF)
  uint8_t       inLen    = 0;                                                   // Current length of "in" array
  uint16_t      rxBufLen = 0;                                                   // Current length of input buffer

    // Exit if there is no read buffer
    if( rxBuf == NULL )
        return( STATUS_FAIL_NOBUFFER );

    // Exit the function if we're already busy reading, else set the 'busy' flag
    if ( myUart.Channels[myUart_Instance].RxBusy )
        return( STATUS_FAIL_BUSY );
    else
        myUart.Channels[myUart_Instance].RxBusy = 1;

    // If '0' is passed as the "number of bytes to read", set to default size
    if ( rxSize == 0 )
        rxSize = DEFAULT_MAX_READ;

    // If recieve "echo" feature is enabled, wait until the transmit channel isn't busy
    if ( myUart.Channels[myUart_Instance].RxEcho ) {
        if ( myUart.Channels[myUart_Instance].TxBusy == 1 )
        {
            __low_power_mode_0();
        }
    }

    // Clear and enable USCI_Ax RX interrupt
    EUSCI_A_UART_clearInterrupt( BaseAddr, EUSCI_A_UART_RECEIVE_INTERRUPT);
    EUSCI_A_UART_enableInterrupt( BaseAddr, EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Keep transmitting until the receive data buffer has reached the specified rxSize
    while ( rxBufLen < rxSize )
    {
        // We 'missed' reading the data in time if RxRDY ever goes above '1'
        if ( myUart.Channels[myUart_Instance].RxRDY >> 1 )
            ret = STATUS_RX_MISSED_REAL_TIME;

        // If not ready, sleep until we get a receive interrupt event that sets the RxRDY flag
        if ( myUart.Channels[myUart_Instance].RxRDY == 0 ) {
            __low_power_mode_0();
        }
        else {
            myUart.Channels[myUart_Instance].RxRDY = 0;                         // If Rx is ready, clear the ready bit
            in[0] = EUSCI_A_UART_receiveData( BaseAddr );                       // Read byte from the RX receive buffer
            inLen = 1;                                                          // Set 'in' buffer length to '1' byte

            // If input byte is CR (or linefeed) add the other character to 'in'
            switch ( in[0] )
            {
                case ASCII_LINEFEED:
                    in[1] = ASCII_ENTER;
                    inLen++;                                                    // Increment the size of 'in' buffer length
                    rxSize = rxBufLen;                                          // Set receive size to current buffer length to force the function to complete
                    break;

                case ASCII_ENTER:
                    in[1] = ASCII_LINEFEED;
                    inLen++;                                                    // Increment the size of 'in' buffer length
                    rxSize = rxBufLen;                                          // Set receive size to current buffer length to force the function to complete
                    break;
            }

            // Copy 'in' character(s) to the data receive buffer
            for ( i = 1; i <= inLen; i++ ) {
                *( rxBuf + rxBufLen ) = in[i-1];
                rxBufLen++;
            }

           // If 'echo' is enabled, trasmit the newly received character(s) back to sender (for terminals without 'local echo')
           if ( myUart.Channels[myUart_Instance].RxEcho ) {
                if ( !myUart.Channels[myUart_Instance].TxBusy )                 // Wait until transmit isn't busy, before sending character(s)
                {
                    myUart_writeBuf( BACKCHANNEL_UART, (unsigned char *)in, inLen, NOCRLF );
                }
            }
        }
    }

    // Disable the RX interrupt and set the port to 'not busy'
    EUSCI_A_UART_disableInterrupt( BaseAddr, EUSCI_A_UART_RECEIVE_INTERRUPT);
    myUart.Channels[myUart_Instance].RxBusy = 0;

    // Return length of bytes received
    //Not yet implemented

    // Return with status
    return ( ret );
}


//*****************************************************************************
// USCI_A0 Interrupt Service Routine
//*****************************************************************************
#pragma vector = USCI_A0_VECTOR
__interrupt void myUart0_isr(void)
{
	int chan = 0;

    switch ( __even_in_range( UCA0IV, USCI_UART_UCTXCPTIFG ))
    {
        case USCI_NONE:
        	break;

        // UART receive interrupt
        case USCI_UART_UCRXIFG:

            myUart.Channels[chan].RxRDY++;                                      // Interrupt tells us that that the UART RX buffer is ready to read
            break;

        // UART transmit interrupt
        case USCI_UART_UCTXIFG:

            myUart.Channels[chan].TxRDY = 1;                                    // Interrupt tells us that that the UART TX buffer is available for writing
            if ( txBufLen == 0 )
                myUart.Channels[chan].TxBusy = 0;                               // Set the Tx as 'not busy' if full buffer has been transfered
            break;

        case USCI_UART_UCSTTIFG:
            __no_operation();
            break;

        case USCI_UART_UCTXCPTIFG:
            __no_operation();
            break;
    }

    // Exit low-power mode:
    //   Now that we either have received a byte - or are ready to transmit
    //   a another byte - we need to wake up the CPU (since our read/write
    //   routines enter LPM while waiting for the UART to do its thing)
    _low_power_mode_off_on_exit();

}

//*****************************************************************************
// USCI_A1 Interrupt Service Routine
//*****************************************************************************
#pragma vector = USCI_A1_VECTOR
__interrupt void myUart1_isr(void)
{
	int chan = 1;

    switch ( __even_in_range( UCA1IV, USCI_UART_UCTXCPTIFG ))
    {
        case USCI_NONE:
        	break;

        // UART receive interrupt
        case USCI_UART_UCRXIFG:

            myUart.Channels[chan].RxRDY++;                                      // Interrupt tells us that that the UART RX buffer is ready to read
            break;

        // UART transmit interrupt
        case USCI_UART_UCTXIFG:

            myUart.Channels[chan].TxRDY = 1;                                    // Interrupt tells us that that the UART TX buffer is available for writing
            if ( txBufLen == 0 )
                myUart.Channels[chan].TxBusy = 0;                               // Set the Tx as 'not busy' if full buffer has been transfered
            break;

        case USCI_UART_UCSTTIFG:
            __no_operation();
            break;

        case USCI_UART_UCTXCPTIFG:
            __no_operation();
            break;
    }

    // Exit low-power mode:
    //   Now that we either have received a byte - or are ready to transmit
    //   a another byte - we need to wake up the CPU (since our read/write
    //   routines enter LPM while waiting for the UART to do its thing)
    _low_power_mode_off_on_exit();

}
