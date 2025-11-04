// STM32L432KC_SPI.c
// Jared Carreno
// jcarreno@hmc.edu
// 10/16/2025
// TODO: <SHORT DESCRIPTION OF WHAT THIS FILE DOES>

#include "STM32L432KC.h"
#include <stm32l432xx.h>
#include "STM32L432KC_SPI.h"

void initSPI(int br, int cpol, int cpha) {

// based on "Configuration of SPI" section on page 1313 of reference manual

  // enable clocks
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN);
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

  // set pin modes
    pinMode(SPI1_SCK, GPIO_ALT); // SPI1_SCK
    pinMode(SPI1_MISO, GPIO_ALT); // SPI1_MISO
    pinMode(SPI1_MOSI, GPIO_ALT); // SPI1_MOSI
    pinMode(SPI1_CE, GPIO_OUTPUT); //  Manual CE/NSS

    // Set output speed type to high for SCK
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED3);

    //Setting to AF05
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL3, 5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL4, 5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 5);

    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br); // set baud rate divisor

    SPI1->CR1 |= (SPI_CR1_MSTR); // MCU is master
    // resetting CPOL, CPHA bits, making data send with MSB first, and disabling software slave management
    SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_LSBFIRST | SPI_CR1_SSM); 
  
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha); // set CPHA to inputted value
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol); // set CPOL to inputted value
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111); // only acccepting 8 bits of data

    // setting FIFO flag to 1/4 (8-bit), enabling SS in master mode while SPI active
    SPI1->CR2 |= (SPI_CR2_FRXTH | SPI_CR2_SSOE);

    SPI1->CR1 |= (SPI_CR1_SPE); // Enable SPI
}

char spiSendReceive(char send) {
    while(!(SPI1->SR & SPI_SR_TXE)); // wait until transmit buffer is empty (TXE HIGH)
    *(volatile char *) (&SPI1->DR) = send; // shove character into data register
    while(!(SPI1->SR & SPI_SR_RXNE)); // wait until receive buffer empty (RXNE HIGH)
    char rec = (volatile char) SPI1->DR; // shove received data in data register to read
    return rec; // function outputs character that was sent
}