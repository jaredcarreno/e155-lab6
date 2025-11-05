// DS1722.c
// Jared Carreno
// jcarreno@hmc.edu
// 10/20/2025
// This file contains code for reading temperature and setting temperature resolution from/to the DS1722 temperature sensor

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "STM32L432KC.h"
#include <stm32l432xx.h>
#include "DS1722.h"

//switch (expression) {
//    case value1:
//        // code to execute if expression equals value1
//        break;
//    case value2:
//        // code to execute if expression equals value2
//        break;
//    // ... more cases
//    default:
//        // code to execute if no case matches
//}

//void configureDS1722(void) {
//  initSPI(100, 0, 1);
//  digitalWrite(SPI1_CE, 1); // set CE high
//  spiSendReceive(0x80); // send data
//  spiSendReceive(0b11100000); // default config
//  digitalWrite(SPI1_CE, 0); // set CE low
//}

void setResolution(int res){

  uint8_t config; // config reg only 8 bits

  switch(res) {
    case 8:
      config = 0b11100000;
      break;

    case 9:
      config = 0b11100010;
      break;

    case 10:
      config = 0b11100100;
      break;

    case 11:
      config = 0b11100110;
      break;
  
    case 12:
      config = 0b11101110;
      break;
  }

  digitalWrite(SPI1_CE, 1);
  spiSendReceive(CFG_WRITE); // CFG_WRITE = 0x80
  spiSendReceive(config);
  digitalWrite(SPI1_CE, 0);
}


float readTemp(void) {
  uint8_t tempLSB = 0;
  uint8_t tempMSB = 0;

  digitalWrite(SPI1_CE, 1);
  spiSendReceive(TEMP_MSB);
  tempMSB = spiSendReceive(0x00); // dummy bytes to grab data from MISO line
  digitalWrite(SPI1_CE, 0);

  digitalWrite(SPI1_CE, 1);
  spiSendReceive(TEMP_LSB);
  tempLSB = spiSendReceive(0x00); // dummy bytes to grab data from MISO line
  digitalWrite(SPI1_CE, 0);

  float temperature = (tempMSB << 8) | tempLSB; // concatenate MSB and LSB bytes
  temperature = (float)temperature/(float)256;
  return temperature;
}