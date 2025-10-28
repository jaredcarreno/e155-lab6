// STM32L432KC_SPI.h
// Jared Carreno
// jcarreno@hmc.edu
// 10/16/2025
// Enables SPI (setting clock speed, polarity, phase) and tests it by sending a character.

#ifndef STM32L4_SPI_H
#define STM32L4_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>

// defining pins for MISO, MOSI, SCK, NSS/CE (Table 14 of datasheet)
// Logic Analyzer Guideline:
// Plug correct pins listed below into L.A.
// Set decode mode to SPI, set correct SCLK, MOSI, MISO, set correct CE/CS
// Ensure polarity is 1

#define SPI1_MISO PB4 // D2
#define SPI1_MOSI PB5 // D3
#define SPI1_SCK PB3 // D0
#define SPI1_CE PB1 // D6

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Enables the SPI peripheral and intializes its clock speed (baud rate), polarity, and phase.
 *    -- br: (0b000 - 0b111). The SPI clk will be the master clock / 2^(BR+1).
 *    -- cpol: clock polar ity (0: inactive state is logical 0, 1: inactive state is logical 1).
 *    -- cpha: clock phase (0: data captured on leading edge of clk and changed on next edge, 
 *          1: data changed on leading edge of clk and captured on next edge)
 * Refer to the datasheet for more low-level details. */ 
void initSPI(int br, int cpol, int cpha);

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send);

#endif