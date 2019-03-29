// ----------------------------------------------------------------------------
// myGpio.c  (for lab_06c_timerDirectDriveLed project)  ('FR5969 Launchpad)
// ----------------------------------------------------------------------------

//***** Header Files **********************************************************
#include <custom.h>
#include "def.h"
#include <driverlib.h>
#include <gpio.h>
#include <uart.h>

//***** Defines ***************************************************************



//*****************************************************************************
// Initialize GPIO
//*****************************************************************************
void initGPIO(void) {

    // Configure all GPIO pins as Output:
    P1OUT = 0;
    P1DIR = 0xFF;

    P2OUT = 0;
    P2DIR = 0xFF;

    P3OUT = 0;
    P3DIR = 0xFF;

    P4OUT = 0;
    P4DIR = 0xFF;

    PJOUT = 0;
    PJDIR = 0xFFFF;

    // Set pin P4.6 to output direction, then turn LED off
    GPIO_setAsOutputPin(LED_RED);                             // Red LED (LED1)
    GPIO_setOutputLowOnPin(LED_RED);

    // Set pin P1.0 to output direction and turn LED off
    GPIO_setAsOutputPin(LED_GREEN);                             // Green LED (LED2)
    GPIO_setOutputLowOnPin(LED_GREEN);

    // Set pin P1.5 to output direction, disable SPI slave
    GPIO_setAsOutputPin(NSS_PIN);                             // Chip Select
    GPIO_setOutputHighOnPin(NSS_PIN);

    // Set pin P1.4 to input direction, reset pin floating
    GPIO_setAsInputPinWithPullUpResistor(SX_RESET_PIN);     // Reset

    // Set pin P1.3 to input direction, IRQ pin for TXDone
    GPIO_setAsInputPinWithPullDownResistor(IRQ_PIN);        // IRQ pin

#ifndef SX1276_CHIP
    // Set pin P3.0 to input direction, Busy pin
    GPIO_setAsInputPinWithPullUpResistor(BUSY_PIN);       // Busy pin

    // Set pin P4.3 to input direction, antSwPower pin
    GPIO_setAsOutputPin(ANT_SW_PIN);                        // antSwPower pin
#endif

#ifndef LOWPOWER    // Pins that are set as inputs drain more current than outputs
    // Set pin P4.5 to input direction, Push Button S1
    GPIO_setAsInputPinWithPullUpResistor(PUSH_LEFT);            // Left Push

    // Set pin P1.1 to input direction, Push Button S2
    GPIO_setAsInputPinWithPullUpResistor(PUSH_RIGHT);            // Right Push
#endif

    // Unlock pins (required for most FRAM devices)
    // Unless waking from LPMx.5, this should be done before clearing and enabling GPIO port interrupts
    PMM_unlockLPM5();

//    // Set P4.5 as input with pull-up resistor (for push button S1)
//    //  configure interrupt on low-to-high transition
//    //  and then clear flag and enable the interrupt
//    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P4, GPIO_PIN5 );
//    GPIO_selectInterruptEdge ( GPIO_PORT_P4, GPIO_PIN5, GPIO_LOW_TO_HIGH_TRANSITION );
//    GPIO_clearInterrupt ( GPIO_PORT_P4, GPIO_PIN5 );
//    GPIO_enableInterrupt ( GPIO_PORT_P4, GPIO_PIN5 );

//    // Set P1.1 as input with pull-up resistor (for push button S2)
//    //  configure interrupt on low-to-high transition
//    //  and then clear flag and enable the interrupt
//    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P1, GPIO_PIN1 );
//    GPIO_selectInterruptEdge ( GPIO_PORT_P1, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION );
//    GPIO_clearInterrupt ( GPIO_PORT_P1, GPIO_PIN1 );
//    GPIO_enableInterrupt ( GPIO_PORT_P1, GPIO_PIN1 );

    // When running this lab exercise, you will need to pull the J6 jumper and
    // use a jumper wire to connect signal from P1.3 (on boosterpack pinouts) to
    // J6.2 (bottom pin) of LED1 jumper ... this lets the TA1.2 signal drive
    // LED1 directly (without having to use interrupts)

#ifdef UART
    GPIO_setAsPeripheralModuleFunctionOutputPin( GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION );

    // EUSCI_UART_A0 pin configuration
    GPIO_setAsPeripheralModuleFunctionOutputPin(
    		myUart.Channels[0].TxPort,
    		myUart.Channels[0].TxPin,
    		myUart.Channels[0].TxSel
	);
    GPIO_setAsPeripheralModuleFunctionOutputPin(
       		myUart.Channels[0].RxPort,
        	myUart.Channels[0].RxPin,
        	myUart.Channels[0].RxSel
	);

    // EUSCI_UART_A1 pin configuration
    GPIO_setAsPeripheralModuleFunctionOutputPin(
    		myUart.Channels[1].TxPort,
    		myUart.Channels[1].TxPin,
    		myUart.Channels[1].TxSel
	);
    GPIO_setAsPeripheralModuleFunctionOutputPin(
       		myUart.Channels[1].RxPort,
        	myUart.Channels[1].RxPin,
        	myUart.Channels[1].RxSel
	);
#endif


    // Set LFXT (low freq crystal pins) to crystal input (rather than GPIO)
    // Since HFXT is not used, we don't need to set these pins. But for the
    // record, they are:
    //              GPIO_PIN6                            // HFXTIN on PJ.6
    //              GPIO_PIN7                            // HFXOUT on PJ.7
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_PJ,
            GPIO_PIN4 +                                  // LFXIN  on PJ.4
            GPIO_PIN5 ,                                  // LFXOUT on PJ.5
            GPIO_PRIMARY_MODULE_FUNCTION
    );

//    // Output the ACLK and SMCLK signals to their respective pins - which allows you to
//    // watch them with a logic analyzer (ACLK on P2.0, SMCLK on P3.4)
//    GPIO_setAsPeripheralModuleFunctionOutputPin(
//                    GPIO_PORT_P2,                        // Unfortunately, P2.0 is not pinned out to FR5969 Boosterpack pin
//                    GPIO_PIN0,                           // ACLK on P2.0
//                    GPIO_TERNARY_MODULE_FUNCTION
//    );
//    GPIO_setAsPeripheralModuleFunctionOutputPin(
//                    GPIO_PORT_P3,
//                    GPIO_PIN4,                           // SMCLK on P3.4
//                    GPIO_TERNARY_MODULE_FUNCTION         // Could use Secondary or Tertiary mode
//    );
}


//*****************************************************************************
// Interrupt Service Routines
//*****************************************************************************
//#pragma vector=PORT1_VECTOR
//__interrupt void pushbutton_ISR (void)
//{
//    switch( __even_in_range( P1IV, P1IV_P1IFG7 )) {
//        case P1IV_NONE:   break;                               // None
//        case P1IV_P1IFG0:                                      // Pin 0
//             __no_operation();
//             break;
//       case P1IV_P1IFG1:                                       // Pin 1 (button 2)
//            GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0 );
//            break;
//       case P1IV_P1IFG2:                                       // Pin 2
//            __no_operation();
//            break;
//       case P1IV_P1IFG3:                                       // Pin 3
//            __no_operation();
//            break;
//       case P1IV_P1IFG4:                                       // Pin 4
//            __no_operation();
//            break;
//       case P1IV_P1IFG5:                                       // Pin 5
//            __no_operation();
//            break;
//       case P1IV_P1IFG6:                                       // Pin 6
//            __no_operation();
//            break;
//       case P1IV_P1IFG7:                                       // Pin 7
//            __no_operation();
//            break;
//       default:   _never_executed();
//    }
//}

//#pragma vector=PORT4_VECTOR
//__interrupt void pushbutton1_ISR (void)
//{
//    switch( __even_in_range( P4IV, P4IV_P4IFG7 )) {
//        case P4IV_NONE:   break;                               // None
//        case P4IV_P4IFG0:                                      // Pin 0
//             __no_operation();
//             break;
//       case P4IV_P4IFG1:                                       // Pin 1
//           __no_operation();
//            break;
//       case P4IV_P4IFG2:                                       // Pin 2
//            __no_operation();
//            break;
//       case P4IV_P4IFG3:                                       // Pin 3
//            __no_operation();
//            break;
//       case P4IV_P4IFG4:                                       // Pin 4
//            __no_operation();
//            break;
//       case P4IV_P4IFG5:                                       // Pin 5 (button 1)
//           GPIO_toggleOutputOnPin( GPIO_PORT_P4, GPIO_PIN6 );
//            break;
//       case P4IV_P4IFG6:                                       // Pin 6
//            __no_operation();
//            break;
//       case P4IV_P4IFG7:                                       // Pin 7
//            __no_operation();
//            break;
//       default:   _never_executed();
//    }
//}
