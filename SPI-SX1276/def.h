/*
 * def.h
 * Definitions for different deploys
 *
 */

//------Select the Chip being used here---------
#define SX1276_CHIP
//#define SX1261_CHIP
//#define SX1262_CHIP
 
#define TTN         // Encodes the payload in order to be compatible with TTN
#define REDUCED     // Non-critical and repetitive configurations are not executed, e.g. setting the Invert_IQ every time a packet is send
//#define SPIARRAY  // Instead of calling the LoRa library, a pre-filled array is completed with the data to send ! it needs REDUCED
#define UART        // Enables and uses the UART-Backchannel to display the actual TX-Power and SF values
//#define TIME_MEASUREMENT    // Pin for the measurement of Active- and Send-Time are enabled
#define LEDS        // Determines if LEDs are used or not

// Pins for Time-Measurement
#define TM1_PIN      GPIO_PORT_P3, GPIO_PIN4    // Wake-Up Time
#define TM2_PIN      GPIO_PORT_P3, GPIO_PIN5    // Sending Time

// Board-specific pins
#define LED_RED      GPIO_PORT_P4, GPIO_PIN6
#define LED_GREEN    GPIO_PORT_P1, GPIO_PIN0
#define PUSH_LEFT    GPIO_PORT_P4, GPIO_PIN5
#define PUSH_RIGHT   GPIO_PORT_P1, GPIO_PIN1

