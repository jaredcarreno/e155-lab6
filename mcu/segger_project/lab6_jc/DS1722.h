// DS1722.h
// Jared Carreno
// jcarreno@hmc.edu
// 10-19-2025
// TODO: <SHORT DESCRIPTION OF WHAT THIS FILE DOES>

#ifndef DS1722_H
#define DS1722_H

#define CFG_READ (0x0UL) // config register is 8 bits long
#define CFG_WRITE (0x80UL) // config register is 8 bits long
#define TEMP_LSB (0x01UL)
#define TEMP_MSB (0x02UL)

//In the continuous conversion mode, the device continuously computes the temperature and stores the most
//recent result in the temperature register at addresses 01h (LSB) and 02h (MSB). In the one-shot
//conversion mode, the DS1722 performs one temperature conversion and then returns to the shutdown
//mode, storing temperature in the temperature register. Details on how to change the setting after power-
//up are contained in the “OPERATION-Programming” section.

///////////////////////////////////////
///// Function Prototypes
///////////////////////////////////////

// pull CE low, SPI send receive function, pull CE HIGH
void configureDS1722(void);
void setResolution(int res);
float readTemp(void);

#endif