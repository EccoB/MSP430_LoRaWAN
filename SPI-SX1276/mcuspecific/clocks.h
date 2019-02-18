/*
 * myClocks.h
 * Inits the clocks of the MCU
 *
 */

#ifndef CLOCKS_H_
#define CLOCKS_H_

//***** Prototypes ************************************************************
void initClocks(void);

//***** Defines ***************************************************************
#define LF_CRYSTAL_FREQUENCY_IN_HZ     32768
#define HF_CRYSTAL_FREQUENCY_IN_HZ     0                                        // FR5969 Launchpad does not ship with HF Crystal populated

#define myMCLK_FREQUENCY_IN_HZ         8000000
#define mySMCLK_FREQUENCY_IN_HZ        8000000
#define myACLK_FREQUENCY_IN_HZ         312


#endif /* CLOCKS_H_ */

