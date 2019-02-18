#include <msp430.h>
#include <stdint.h>
#include "mcu.h"

void mcu_delayms(uint32_t ms) {
  while (ms) {
    __delay_cycles(8 * 998);
  	ms--;
  }
}

void mcu_delayus(uint32_t us) {
	while (us) {
		__delay_cycles(7); //for 8MHz
		us--;
  }
}

void mcu_memcpy1(uint8_t *dst, const uint8_t *src, uint16_t size) {
    while(size--) *dst++ = *src++;
}
