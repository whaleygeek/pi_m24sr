/*******************************************************************************
 *
 *  **          thinking->binaries(ltd)
 *  **          (c) 2008
 *  ****
 *  ****
 *  **          http://www.thinkingbinaries.com
 *  **          enquiry@thinkingbinaries.com
 *  ****
 *  ****
 *
 * Project    : 
 * File       : i2c.h
 * Author     : David Whale
 * Start-Date : 14/08/2008
 * Purpose    : Drive I2C bus in master mode.
 *
 * USAGE NOTES:
 *   To make it easier to implement various device protocols, such as the
 *   protocol used by some EEPROM devices to allow continuous read, or
 *   to allow preceeding a read/write with an address set while not loosing
 *   the bus, a number of simple building-blocks are provided in the forms
 *   of functions and efficient macros. This is mainly to get the right
 *   balance between flexibility and code size.
 *
 *   i2c_Write and i2c_Read are the most obvious functions to use for
 *   reading and writing data, but note that they honour the flag settings
 *   set previously. You can choose to set and reset individual flags, or
 *   set/reset flags in one go with i2c_SetMode(). In some implementations,
 *   you might do a i2c_SetMode() at init time only, or even just rely
 *   on the default flag setup as set in i2c_Init().
 *
 *   For complex protocols, you might choose to transmit and receive bytes
 *   byte at a time by using the spi_Tx() and spi_Rx() functions, along
 *   with twiddling the various bit flags to change what they wait for and
 *   what they generate as a side effect of calling them.
 *
 *   A 4-bit speed parameter is provided as a way of modulating the generated
 *   clock speed, both so you can slow things down when initially debugging,
 *   and also so you can slow things down for slower devices. Speed 0 is the
 *   fastest speed and runs with no delays at all, and speed F is the slowest
 *   speed and runs with quite long delays. These are not calibrated speeds
 *   and they depend on the CPU_SYSCLK value as to what actual speed is
 *   achieved, but you can tweak the delays by changing the i2c.c module
 *   internals.
 ******************************************************************************/

#ifndef _I2C_H
#define _I2C_H


/***** INCLUDES *****/

#include <time.h>

typedef unsigned char U8;

/***** CONSTANTS *****/

typedef unsigned char I2C_RESULT;
#define I2C_RESULT_IS_ERROR(X)  (((X) & 0x80) != 0)
#define I2C_RESULT_OK           0
#define I2C_RESULT_E_NACK       0x81


/***** TYPEDEFS *****/

typedef struct
{
  unsigned char scl;
  unsigned char sda;
  unsigned char start;
  unsigned char stop;

  struct timespec tSettle;
  struct timespec tHold;
  struct timespec tFreq;

} I2C_CONFIG;


/***** EXTERNAL FUNCTION PROTOTYPES *****/

I2C_RESULT i2c_Init(I2C_CONFIG* pConfig);

I2C_RESULT i2c_InitDefaults(void);

I2C_RESULT i2c_Start(void);

I2C_RESULT i2c_Stop(void);

I2C_RESULT i2c_TxByte(unsigned char data);

I2C_RESULT i2c_RxByte(unsigned char* pData, unsigned char bAck);

I2C_RESULT i2c_Write(unsigned char addr, unsigned char* pData, unsigned char len);

I2C_RESULT i2c_Read(unsigned char addr, unsigned char* pData, unsigned char len);

I2C_RESULT i2c_Finished(void);

#endif

/***** END OF FILE *****/
