// ----------------------------------------------------------------------------
// myClocks.c  ('FR5969 Launchpad)
//
// This routine sets up the Very Low Frequency Oscillator (VLO) and high-freq
// internal clock source (DCO). Then configures ACLK, SMCLK, and MCLK:
//    ACLK  = 312.5Hz
//    SMCLK =  8MHz
//    MCLK  =  8MHz
// ----------------------------------------------------------------------------

//***** Header Files **********************************************************
#include "def.h"
#include <clocks.h>
#include <driverlib.h>


//***** Defines ***************************************************************
// See additional #defines in 'myClocks.h'


//***** Global Variables ******************************************************
uint32_t myACLK  = 0;
uint32_t mySMCLK = 0;
uint32_t myMCLK  = 0;


//***** initClocks ************************************************************
void initClocks(void) {

    //**************************************************************************
    // Configure Oscillators
    //**************************************************************************
    // Set the LFXT and HFXT crystal frequencies being used so that driverlib
    //   knows how fast they are (needed for the clock 'get' functions)
    CS_setExternalClockSource(
            LF_CRYSTAL_FREQUENCY_IN_HZ,
            HF_CRYSTAL_FREQUENCY_IN_HZ
    );

    // Verify if the default clock settings are as expected
    myACLK  = CS_getACLK();
    mySMCLK = CS_getSMCLK();
    myMCLK  = CS_getMCLK();

    // Initialize LFXT crystal oscillator without a timeout. In case of failure
    //   the code remains 'stuck' in this function.
    // For time-out instead of code hang use CS_turnOnLFXTWithTimeout(); see an
    //   example of this in lab_04c_crystals.
    //CS_turnOnLFXT( CS_LFXT_DRIVE_0 ); //###

    // Configure one FRAM wait-state as required by the device datasheet for MCLK
    // operation beyond 8MHz before configuring the clock system.
    //FRCTL0 = FRCTLPW | NWAITS_1; //###

    // Set DCO to run at 8MHz
    CS_setDCOFreq(
            CS_DCORSEL_1,                                                       // Set Frequency range (DCOR)
            CS_DCOFSEL_3                                                        // Set Frequency (DCOF)
    );

    //**************************************************************************
    // Configure Clocks
    //**************************************************************************
    // Set ACLK to use VLO as its oscillator source (~10KHz)
    // With ~10KHz VLO as source and a divide by 32, ACLK should run at that rate of ~312.5Hz
    CS_initClockSignal(
            CS_ACLK,                                                            // Clock you're configuring
            CS_VLOCLK_SELECT,                                                   // Clock source
            CS_CLOCK_DIVIDER_32                                                 // Divide down clock source by this much
    );

    // Set SMCLK to use DCO as its oscillator source (DCO was configured earlier in this function for 8MHz)
    CS_initClockSignal(
            CS_SMCLK,                                                           // Clock you're configuring
            CS_DCOCLK_SELECT,                                                   // Clock source
            CS_CLOCK_DIVIDER_1                                                  // Divide down clock source by this much
    );

    // Set MCLK to use DCO as its oscillator source (DCO was configured earlier in this function for 8MHz)
    CS_initClockSignal(
            CS_MCLK,                                                            // Clock you're configuring
            CS_DCOCLK_SELECT,                                                   // Clock source
#ifndef DMA
            CS_CLOCK_DIVIDER_1                                                  // MCLK runs at 8MHz
#else
            CS_CLOCK_DIVIDER_4                                                  // Divide down clock source so that DMA is slower than SPI (MCLK = 2MHz)
#endif
    );

    // Verify that the modified clock settings are as expected
    myACLK  = CS_getACLK();
    mySMCLK = CS_getSMCLK();
    myMCLK  = CS_getMCLK();
}

