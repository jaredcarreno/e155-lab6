/*
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stm32l432xx.h>
#include "main.h"
#include "STM32L432KC_SPI.h"
#include "DS1722.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";

char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";

char* resStr = "<p>Temperature Resolution Control:</p><form action=\"8-bit\"><input type=\"submit\" value=\"8-bit Resolution\"></form>\
    </p><form action=\"9-bit\"><input type=\"submit\" value=\"9-bit Resolution\"></form>\
    </p><form action=\"10-bit\"><input type=\"submit\" value=\"10-bit Resolution\"></form>\
    </p><form action=\"11-bit\"><input type=\"submit\" value=\"11-bit Resolution\"></form>\
    </p><form action=\"12-bit\"><input type=\"submit\" value=\"12-bit Resolution\"></form>";

char* webpageEnd   = "</body></html>";

int res_status;

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

int updateResStatus(char request[]) {
        // The request has been received. now process to determine whether to turn the LED on or off
        if (inString(request, "8-bit")==1) {
          res_status = 8;
          setResolution(8);
        }

        else if (inString(request, "9-bit")==1) {
          res_status = 9;
          setResolution(9);
        } 
        
        else if (inString(request, "10-bit")==1) {
          res_status = 10;
          setResolution(10);
                }  

        else if (inString(request, "11-bit")==1) {
          res_status = 11;
          setResolution(11);
                }     
   
        else if (inString(request, "12-bit")==1) {
          res_status = 12;
          setResolution(12);
                }
        
	return res_status;
}


// update resolution (updateResStatus()) --> read temperature data (readTemp()) --> binary to decimal (tempConvertDecimal())

// The data binary output representing temperature is structured as follows:
// MSB: SIGN | 2^6 | 2^5 | 2^4 | 2^3 | 2^2 | 2^1 | 2^0
// LSB: 2^-1 | 2^-2 | 2^-3 | 2^-4 | 0 | 0 | 0 | 0 |
// 
// The last 7 bits of 0x02 (most significant byte) correspond to the whole values
// The first 4 bits of 0x01 (least significant byte) correspond to decimal values

// the largest number range that can represented by an 8-bit signed number is -128 to 127:
// Looking at the binary representation for -55 degrees C, 1100 1001 0000 0000, ignoring the most signficant bit, the decimal value is 73.
// If we subtract 128, we get the desired value -55. 

// 1100 1001 is also just the two's complement of 55.

// floats smaller in size (8 bytes), but doubles more precise (16 bytes)
float tempConvertDecimal(int tempBinary) {
  uint8_t MSB = (tempBinary >> 8);
  uint8_t LSB = tempBinary;  

  double tempDecimal = 0.0;
  double tempWhole = 0.0;
  
  if ((MSB >> 7) == 1) {
    tempWhole = -128 + (MSB &= 127);
  }

  else {
    tempWhole = (MSB &= 127);
  }
  
  if ((LSB &= 0x80)) {
    tempDecimal = tempDecimal + 0.5; // 2^-1 = 0.5
  }

  else if ((LSB &= 0x40)) {
    tempDecimal = tempDecimal + 0.25; // 2^-2 = 0.25
  }

  else if ((LSB &= 0x20)) {
    tempDecimal = tempDecimal + 0.125; // 2^-3 = 0.125
  }

  else if ((LSB &= 0x10)) {
    tempDecimal = tempDecimal + 0.0625; // 2^-4 = 0.0625
  }

  return tempDecimal + tempWhole; 
}

/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////

int main(void) {
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(LED_PIN, GPIO_OUTPUT); // set output to control LED PA6
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

   //Initializing SPI
  initSPI(7, 0, 1);

  float tempData;
  float tempDataDecimal;

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }

    // SPI code for reading temperature
    int res_status = updateResStatus(request);
    tempData = readTemp();
    tempDataDecimal = tempData;
    //delay_millis(TIM15, 100);
    
    // Update string with current LED state
  
    int led_status = updateLEDStatus(request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    char resStatusStr[50];
    if (res_status == 8) {
      sprintf(resStatusStr, "Temperature: %.0f C", tempDataDecimal);
    } else if (res_status == 9) {
      sprintf(resStatusStr, "Temperature: %.1f C", tempDataDecimal);
    } else if (res_status == 10) {
      sprintf(resStatusStr, "Temperature: %.2f C", tempDataDecimal);
    } else if (res_status == 11) {
      sprintf(resStatusStr, "Temperature: %.3f C", tempDataDecimal);
    } else if (res_status == 12) {
      sprintf(resStatusStr, "Temperature: %.4f C", tempDataDecimal);
    }
    

    // finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED
    sendString(USART, "<h2>LED Status</h2>");
    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    //sendString(USART, resStr);
    sendString(USART, "<h2>Temp. Status</h2>");
    sendString(USART, "<p>");
    sendString(USART, resStatusStr);
    sendString(USART, "</p>");
  
    sendString(USART, "<h2>Temp. Resolution</h2>");
    sendString(USART, resStr);

    sendString(USART, webpageEnd);
  }
}


// provided code:

///*
//File: Lab_6_JHB.c
//Author: Josh Brake
//Email: jbrake@hmc.edu
//Date: 9/14/19
//*/


//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include "main.h"

///////////////////////////////////////////////////////////////////
//// Provided Constants and Functions
///////////////////////////////////////////////////////////////////

////Defining the web page in two chunks: everything before the current time, and everything after the current time
//char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
//	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
//	</head>\
//	<body><h1>E155 Web Server Demo Webpage</h1>";
//char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
//	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
//char* webpageEnd   = "</body></html>";

////determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
//int inString(char request[], char des[]) {
//	if (strstr(request, des) != NULL) {return 1;}
//	return -1;
//}

//int updateLEDStatus(char request[])
//{
//	int led_status = 0;
//	// The request has been received. now process to determine whether to turn the LED on or off
//	if (inString(request, "ledoff")==1) {
//		digitalWrite(LED_PIN, PIO_LOW);
//		led_status = 0;
//	}
//	else if (inString(request, "ledon")==1) {
//		digitalWrite(LED_PIN, PIO_HIGH);
//		led_status = 1;
//	}

//	return led_status;
//}

///////////////////////////////////////////////////////////////////
//// Solution Functions
///////////////////////////////////////////////////////////////////

//int main(void) {
//  configureFlash();
//  configureClock();

//  gpioEnable(GPIO_PORT_A);
//  gpioEnable(GPIO_PORT_B);
//  gpioEnable(GPIO_PORT_C);

//  pinMode(PB3, GPIO_OUTPUT);
  
//  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
//  initTIM(TIM15);
  
//  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

//  // TODO: Add SPI initialization code

//  while(1) {
//    /* Wait for ESP8266 to send a request.
//    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
//    Therefore the request[] array must be able to contain 18 characters.
//    */

//    // Receive web request from the ESP
//    char request[BUFF_LEN] = "                  "; // initialize to known value
//    int charIndex = 0;
  
//    // Keep going until you get end of line character
//    while(inString(request, "\n") == -1) {
//      // Wait for a complete request to be transmitted before processing
//      while(!(USART->ISR & USART_ISR_RXNE));
//      request[charIndex++] = readChar(USART);
//    }

//    // TODO: Add SPI code here for reading temperature
  
//    // Update string with current LED state
  
//    int led_status = updateLEDStatus(request);

//    char ledStatusStr[20];
//    if (led_status == 1)
//      sprintf(ledStatusStr,"LED is on!");
//    else if (led_status == 0)
//      sprintf(ledStatusStr,"LED is off!");

//    // finally, transmit the webpage over UART
//    sendString(USART, webpageStart); // webpage header code
//    sendString(USART, ledStr); // button for controlling LED

//    sendString(USART, "<h2>LED Status</h2>");


//    sendString(USART, "<p>");
//    sendString(USART, ledStatusStr);
//    sendString(USART, "</p>");

  
//    sendString(USART, webpageEnd);
//  }
//}