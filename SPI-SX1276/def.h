/*
 * def.h
 * Definitions for different deploys
 *
 */
 
#define TTN      // Encodes the payload in order to be compatible with TTN
#define REDUCED   // Non-critical and repetitive configurations are not executed, e.g. setting the Invert_IQ every time a packet is send
#define UART      // Enables and uses the UART-Backchannel to display the actual TX-Power and SF values
#define LEDS      // Determines if LEDs are used or not
#define RFM95   // Uses the configuration for RFM95 module (PA_Boost) instead of InAir9 (RFO)

