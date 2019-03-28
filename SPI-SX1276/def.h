/*
 * def.h
 * Definitions for different deploys
 *
 */
 
#define TTN         // Encodes the payload in order to be compatible with TTN
#define REDUCED     // Non-critical and repetitive configurations are not executed, e.g. setting the Invert_IQ every time a packet is send
#define UART        // Enables and uses the UART-Backchannel to display the actual TX-Power and SF values
#define LEDS        // Determines if LEDs are used or not
//#define TIME_MEASUREMENT    // Pin for the measurement of Active- and Send-Time are enabled



// Pins for Time-Measurement
#define TM1_PIN      GPIO_PORT_P3, GPIO_PIN4    // Wake-Up Time
#define TM2_PIN      GPIO_PORT_P3, GPIO_PIN5    // Sending Time
