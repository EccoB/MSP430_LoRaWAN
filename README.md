# Low-power LoRaWAN Transmitter for MSP430FR5969

This code implements the basic functions needed for a class A compatible LoRaWAN device. It lacks support to receive data atm. Instead, this code is optimized to save power.
Compatible to TTN, it can be used as a simple data beacon.


## Getting Started

After downloading the code you have to change the configuration in the custom.h. Especially:
- Pins for SPI
- Networkkey, App_key, device_adress to get TTN working

Flashing the code:
Open in CodeComposer and flash it.
The code will send some example data to TTN after startup.

### Prerequisites

You are going to need Code Composer to compile, flash the code.
The code is tested on the MSP430FR5969, but should also work on other MSP430.

Default Pins used for the SX1276 LoTa Transmitter:
| MSP430FR5969 | RFM95/SX1276 | Description |
| --- | --- | --- |
| MOSI/P1.6 | MOSI | SPI |
| MISO/P1.7 | MISO | SPI |
| SCLK/P2.2 | SCK | SPI |
| P1.4 | NSS | Chipselect |
| P1.3 | DIO0 | TX Interrupt |


## Used libraries
- MSP430FR5xx_6xx	Driverlib of TI
- Semtech SX1276 LoRa modem registers and bits definitions
- Semtech LORAMAC_CRYPTO


## Authors


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

