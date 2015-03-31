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
 * File       : i2c.c
 * Author     : David Whale
 * Start-Date : 14/08/2008
 * Purpose    : Drive I2C Bus in master mode
 *
 * USAGE NOTES:
 *   Much care has gone into writing this driver so that it can be used
 *   to implement device specific protocols (e.g. ACK handing on eeproms),
 *   and also so that buffer space can be managed.
 *
 *   The best way to manage limited buffer space, is to split a read or a
 *   write into small parts - reading or writing part of the data to/from
 *   the device into a buffer smaller than the total amount of data to
 *   be transferred. This way, data can be streamed in smaller portions
 *   to/from the slave, thus making it possible to transfer a large buffer
 *   in small chunks.
 *
 *   The interface is such that value is provided by calling the read/write
 *   services, and there is little need for the higher layer software to
 *   effectively take control of the lower level bus, but can delegate
 *   part or all of the transaction to this module.
 ******************************************************************************/


/***** INCLUDES *****/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "i2c.h"
#include "gpio.h"


/***** MODULE CONFIGURATION *****/

//#define I2C_CFGEN_CALLTRACE
//#define I2C_CFGEN_STATETRACE
//#define I2C_CFGEN_PINTRACE


/* GPIO numbers on Raspberry Pi REV2 */
#define SDA 2
#define SCL 3

/* ms */
#define TSETTLE (1UL)
#define THOLD   (1UL)
#define TFREQ   (1UL)

static I2C_CONFIG config;
static I2C_CONFIG defaults = {SCL, SDA, 1, 1, {0,TSETTLE}, {0,THOLD}, {0,TFREQ}};


/***** PLATFORM CONFIGURATION
 * This section defines how to implement each of the different bus conditions
 * on this particular target. By changing the implementation of these, you
 * can adapt this driver to pretty much any hardware, including a bit-banged
 * and a hardware driven device.
 *****/


/**** Define I2C wire primitives.
 * These are the valid states of the I2C bus.
 ****/

#if !defined(I2C_CFGEN_PINTRACE)
/* Normal operation */

/* Define wire primitives for real hardware.
 * i.e. define which of the I2C bus wires are required
 * to be high and which are required to be low. Note that
 * the bus has pullup resistors, so port.h defines how
 * to drive the port so that the pin goes high or low.
 * NONE means none are driven, so they both float high.
 */

#define _INIT() do { \
  gpio_setin(config.sda); \
  gpio_write(config.sda, 0); \
  gpio_setin(config.scl); \
  gpio_write(config.scl, 0); \
} while (0)

#define _NONE() do { \
  gpio_setin(config.sda); \
  gpio_setin(config.scl); \
} while (0)

#define _SDA() do { \
  gpio_setout(config.sda); \
  gpio_setin(config.scl); \
} while (0)

#define _SCL() do { \
  gpio_setin(config.sda); \
  gpio_setout(config.scl); \
} while (0)

#define _SDASCL() do { \
  gpio_setout(config.sda); \
  gpio_setout(config.scl); \
} while (0)

#define _DELAY_SETTLE()   delay(config.tSettle)
#define _DELAY_HOLD()     delay(config.tHold)
#define _DELAY_FREQ()     delay(config.tFreq)

#define _GETSDA() gpio_read(config.sda)

#else /* I2C_CFGEN_PINTRACE */
/* trace all pin changes */

#define _INIT() do { \
  printf("INIT   DDDD----CCCC-\n"); \
  printf("INIT   ...|    ...|\n"); \
} while (0)

#define _NONE() \
  printf("NONE   ...|    ...|\n")

#define _SDA() \
  printf("SDA    |...    ...|\n")

#define _SCL() \
  printf("SCL    ...|    |...\n")

#define _SDASCL() \
  printf("SDASCL |...    |...\n")

#define _DELAY_SETTLE()
#define _DELAY_HOLD()
#define _DELAY_FREQ()

#define _GETSDA() (0)

#endif


/**** The I2C bus transition primitives. 
 * These are used to build the message protocol on top of,
 * and are built using the I2C wire primitives above.
 ****/

#if !defined(I2C_CFGEN_STATETRACE)
/* normal operation */

#define _IDLE() \
  _NONE()


/* START CONDITION:
 * Assume both float high on entry.
 * Leaves both low on exit.
 */

#define _START() \
do { \
  _SDA(); \
  _DELAY_SETTLE(); \
  _SDASCL(); \
  _DELAY_SETTLE(); \
} while (0)

/* STOP CONDITION:
 * Assume both driven low on entry.
 * Both float high on exit.
 */

#define _STOP() \
do { \
  _SDA(); \
  _DELAY_SETTLE(); \
  _NONE(); \
  _DELAY_SETTLE(); \
} while (0)

/* WRITE DATA BIT 0:
 * Assumes SCL driven low on entry
 * Leaves SCL driven low on exit
 * Leaves SDA floating high on exit (ready for ack)
 * Rising edge of SCL captures SDA
 * falling edge of SCL, nothing important happens
 */

#define _WRITE0() \
do { \
  _SDASCL(); \
  _DELAY_SETTLE(); \
  _SDA(); \
  _DELAY_SETTLE(); \
  _SDASCL(); \
  _DELAY_SETTLE(); \
} while (0)

/* WRITE DATA BIT 1:
 * Assumes SCL driven on entry
 * Leaves SCL driven low on exit.
 * Leaves SDA floating high on exit (ready for ack)
 */

#define _WRITE1() \
do { \
  _SCL(); \
  _DELAY_SETTLE(); \
  _NONE(); /* rising edge of SCL captures SDA */ \
  _DELAY_SETTLE(); \
  _SCL(); /* nothing samples on the falling edge of SCL */ \
  _DELAY_SETTLE(); \
} while (0)


/* READ A BIT:
 * Assumes SCL driven low on entry
 * SDA could be floating, but not assumed
 * Floats SDA in the middle, so external device can pull low
 * Leaves SDA SCL driven low on exit
 * Stores the bit, for later access by _VALUE() macro.
 */

#define _READ(B) \
do { \
  _SCL(); \
  _DELAY_SETTLE(); \
  _NONE(); /* rising edge of SCL samples SDA */ \
  _DELAY_SETTLE(); \
  if (_GETSDA()) \
  { \
    B = 1; \
  } \
  else \
  { \
    B = 0; \
  } \
  _SCL(); /* falling edge of SCL, nothing samples */ \
  _DELAY_SETTLE(); \
} while (0)

#else /* I2C_CFGEN_STATETRACE */

#define _IDLE() \
  printf("IDLE\n")

#define _START() \
  printf("START\n")

#define _STOP() \
  printf("STOP\n")

#define _WRITE0() \
  printf("WRITE0\n")

#define _WRITE1() \
  printf("WRITE1\n")

#define _READ(B) do { \
  printf("READ\n"); \
  B = 0; \
} while (0)

#endif


/***** LOCAL FUNCTION PROTOTYPES *****/

static void delay(struct timespec time);


/*******************************************************************************
 * i2c_Init - put the bus into a state ready to be used.
 ******************************************************************************/

I2C_RESULT i2c_Init(I2C_CONFIG* pConfig)
{
  memcpy(&config, pConfig, sizeof(I2C_CONFIG));
  _INIT();
  return I2C_RESULT_OK;
}

I2C_RESULT i2c_InitDefaults(void)
{
  //printf("i2c:InitDefaults\n");
  memcpy(&config, &defaults, sizeof(I2C_CONFIG));
  //printf("calling INIT\n");
  _INIT();
  //printf("finished INIT\n");
  return I2C_RESULT_OK;
}


/*******************************************************************************
 * i2c_Start - generate a START condition
 ******************************************************************************/

I2C_RESULT i2c_Start(void)
{
#if !defined(I2C_CFGEN_CALLTRACE)
  _START();
#else
  printf("i2c_Start\n");
#endif
  return I2C_RESULT_OK;
}


/*******************************************************************************
 * i2c_Stop - generate a STOP condition
 ******************************************************************************/

I2C_RESULT i2c_Stop(void)
{
#if !defined(I2C_CFGEN_CALLTRACE)
  _STOP();
#else
  printf("i2c_Stop\n");
#endif
  return I2C_RESULT_OK;
}


/*******************************************************************************
 * i2c_TxByte - transmit a single byte
 *
 * No bus START or STOP or ACK conditions are handled by this function, they must
 * be done externally.
 ******************************************************************************/

I2C_RESULT i2c_TxByte(unsigned char data)
{
#if !defined(I2C_CFGEN_CALLTRACE)
  unsigned char bit;

  /* Send the byte in data, MSB first */

  for (bit=0; bit<8; bit++)
  {
    if (data & 0x80)
	  {
	    _WRITE1();
	  }
	  else
  	{
	    _WRITE0();
	  }
	  data <<= 1;
  }

  /* Read and interpret 9th ACK bit */
  _READ(bit);
  if (!bit)
  { /* Valid ACK */
    return I2C_RESULT_OK;
  }
  else
  { /* bus floats, so no ACK */
    return I2C_RESULT_E_NACK;
  }
#else
  printf("i2c_TxByte(%d)\n", (int) data);
  return I2C_RESULT_OK;
#endif
}


/*******************************************************************************
 * i2c_RxByte - receive a single byte
 *
 * The received byte is put into our buffer - use the macro i2c_GetLastRx() to
 * actually read the value. This is so that we can still return a result code
 * as part of the reading process - without needing to use a resource hungry
 * pointer to store the data in.
 *
 * No bus START or STOP or ACK conditions are handled by this function, they must
 * be done externally.
 ******************************************************************************/

I2C_RESULT i2c_RxByte(unsigned char* pData, unsigned char bAck)
{
#if !defined(I2C_CFGEN_CALLTRACE)
  unsigned char bit;
  unsigned char data;

  /* Read the byte into pData, MSB first */
  data = 0;

  for (bit=0; bit<8; bit++)
  {
    unsigned char b;
    data <<= 1;
    _READ(b);
    data += b;
  }
  *pData = data;

  if (bAck)
  {
    /* Write the ack bit */
    _WRITE0();
  }
#else
  printf("i2c_RxByte(%d,%d)\n", (int) *pData, (int)bAck);
#endif
  return I2C_RESULT_OK;
}


/*******************************************************************************
 * i2c_Write - write a whole block to the I2C at a specified address.
 ******************************************************************************/

I2C_RESULT i2c_Write(unsigned char addr, unsigned char* pData, unsigned char len)
{
  //printf("i2c_write(%d %d %d)\n", (int)addr, (int)*pData, (int)len);

#if !defined(I2C_CFGEN_CALLTRACE)
  I2C_RESULT result;

  if (config.start)
  {
    _START();
  }

  addr <<= 1; /* address is in high 7 bits */
  /* low bit 0 is now zero, indicating a write */

  //printf("send addr %d\n", (int) addr);
  result = i2c_TxByte(addr);
  if (I2C_RESULT_IS_ERROR(result))
  { /* probably a NACK */
    //printf("nack addr\n");
    if (config.stop)
    {
      _STOP();
    }
    return result;
  }

  while (len != 0)
  {
    //printf("send data %d\n", (int)*pData);

    result = i2c_TxByte(*pData);
    if (I2C_RESULT_IS_ERROR(result))
    {
      //printf("nak data\n");
      if (config.stop)
      {
        _STOP();
      }
      return result;
    }
    pData++;
    len--;
  }

  if (config.stop)
  {
    //printf("STOP\n");
    _STOP();
  }
#else
  printf("i2c_Write(%d,%d,%d)\n",(int)addr, (int)*pData, (int)len);
#endif
  return I2C_RESULT_OK;
}



/*******************************************************************************
 * i2c_Read - read a whole block from the I2C at a specified address.
 ******************************************************************************/

I2C_RESULT i2c_Read(unsigned char addr, unsigned char* pData, unsigned char len)
{
#if !defined(I2C_CFGEN_CALLTRACE)
  I2C_RESULT result = I2C_RESULT_OK;

  if (config.start)
  {
    _START();
  }

  addr <<= 1;   /* address goes into high 7 bits */
  addr |= 0x01; /* READ */

  result = i2c_TxByte(addr);
  if (I2C_RESULT_IS_ERROR(result))
  {
    if (config.stop)
    {
      _STOP();
    }
    return result;
  }
  
  while (len != 0)
  {
    /* Don't send ACK on last byte */
    result = i2c_RxByte(pData, (len != 1));
    if (I2C_RESULT_IS_ERROR(result))
	  {
      if (config.stop)
      {
	      _STOP();
      }
	    return result;
	  }
    pData++;
    len--;
  }

  if (config.stop)
  {
    _STOP();
  }
#else
  printf("i2c_Read(%d,%d,%d)\n",(int)addr, (int)*pData,(int)len);
#endif
  return I2C_RESULT_OK;
}


/*******************************************************************************
 * i2c_Finished
 ******************************************************************************/

I2C_RESULT i2c_Finished(void)
{
#if !defined(I2C_CFGEN_CALLTRACE)
  _NONE();
#else
  printf("i2c_Finished\n");
#endif
  return I2C_RESULT_OK;
}


/*******************************************************************************
 * delay
 ******************************************************************************/

static void delay(struct timespec time)
{
  nanosleep(&time, NULL);
}


/***** END OF FILE *****/
