// ----------------------------------------------------------------------------
// uart.h   ('FR6989 Launchpad)
// ----------------------------------------------------------------------------
#ifndef UART_H_
#define UART_H_

//***** Header Files **********************************************************
#include <driverlib.h>

//***** Defines ***************************************************************
#define ASCII_ENTER       0x0D
#define ASCII_CR          0x0D
#define ASCII_LINEFEED    0x0A
#define ASCII_LF          0x0A

#define NOCRLF            0
#define CRLF              1

#define NOECHO            0
#define DOECHO            1

#define DEFAULT_MAX_READ  80

#define NUM_CHANNELS      2
#define CHANNEL_0         0
#define CHANNEL_1         1
#define BACKCHANNEL_UART  0                                                     // Backchannel UART - which connects to PC through emulation port on the LaunchPad

#define STATUS_INIT_SUCCESSFUL     0
#define STATUS_ALREADY_OPEN        1
#define STATUS_INIT_FAILED         2
#define STATUS_FAIL_NOBUFFER       3
#define STATUS_FAIL_BUSY           4
#define STATUS_RX_MISSED_REAL_TIME 5

char gStr[30]; // Buffer for UART Backchannel

//***** TypeDefs **************************************************************
struct Channel
{
	const uint16_t BaseAddress;                                                 // Base Address of EUSCI_A port                     
	const uint8_t  TxPort;														// GPIO Port settings for TX pin                    
	const uint8_t  TxPin;														                                                    
	const uint8_t  TxSel;														                                                    
	const uint8_t  RxPort;														// GPIO Port settings for RX pin                    
	const uint8_t  RxPin;														                                                    
	const uint8_t  RxSel;														                                                    
	uint32_t BaudRate;															// Baud Rate
	uint8_t  Open;																// Open - UART port has been opened and configured  
	uint8_t  TxBusy;														    // Busy -
	uint8_t  TxRDY;
	uint8_t  RxBusy;
	uint8_t  RxRDY;
	uint8_t  RxEcho;
} Channel;

struct Uart_t
{
	const uint16_t NumChannels;
	struct Channel Channels[ NUM_CHANNELS ];
} Uart_t;


//***** Global Variables ******************************************************
extern struct Uart_t myUart;
extern EUSCI_A_UART_initParam myUart_Param_9600_8N1_SMCLK8MHz;
extern EUSCI_A_UART_initParam myUart_Param_9600_8N1_ACLK32Kz;


//***** Function Prototypes ***************************************************
int myUart_init    ( uint16_t, uint32_t, EUSCI_A_UART_initParam * );
int myUart_writeBuf( uint16_t, unsigned char *, uint16_t, int );
int myUart_readBuf ( uint16_t, unsigned char *, uint16_t );


#endif /* UART_H_ */
