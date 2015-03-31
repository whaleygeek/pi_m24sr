/* i2c_test.c  D.J.Whale  18/07/2014
 *
 * A simple I2C port exerciser.
 */


/***** INCLUDES *****/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gpio.h"
#include "i2c.h"


/***** CONSTANTS *****/

/* GPIO numbers on Raspberry Pi REV2 */
#define SDA 2
#define SCL 3

/* ms */
#define TSETTLE (1UL) /* ns */
#define THOLD   (1UL)
#define TFREQ   (1UL)

#define TAG_DETECT        4
#define ADDRESS           0x50
#define CMD_GET_FIRMWARE  0xF0

static struct timespec TIME_10MS = {0, 10L * 1000000L};
static struct timespec TIME_50MS = {0, 50L * 1000000L};


static void delay(struct timespec time)
{
  nanosleep(&time, NULL);
}


int main(int argc, char **argv)
{
  I2C_CONFIG i2cConfig = {SCL, SDA, 1, 1, {0,TSETTLE}, {0,THOLD}, {0,TFREQ}};
  int i;

  /* INIT */

  gpio_init();
  i2c_Init(&i2cConfig);
  gpio_setin(TAG_DETECT);


  /* WAIT FOR TAG */

  printf("Card absent\n");
  while (gpio_read(TAG_DETECT) != 0)
  {
    delay(TIME_10MS);
  }
  printf("Card present\n");


  /* REPORT FIRMWARE ID */

  while (gpio_read(TAG_DETECT) == 0)
  {
    unsigned char buffer[15+1]   = {0}; /* space for terminator */
    unsigned char request[] = {1, CMD_GET_FIRMWARE};
    I2C_RESULT result;

    /* request firmware id read [tx:addr] tx:<len> tx:<cmd> */
    result = i2c_Write(ADDRESS, request, sizeof(request));
    delay(TIME_50MS);

    if (I2C_RESULT_IS_ERROR(result))
    {
      printf("request error:%d\n", (int) result);
    }
    else
    {
      /* read the firmware id back: tx:<addr> rx:<len> rx:<cmd> rx:<ver>... */
      result = i2c_Read(ADDRESS, buffer, sizeof(buffer));
      if (I2C_RESULT_IS_ERROR(result))
      {
        printf("response error:%d\n", (int) result);
      }
      else
      {
        int i;
        unsigned char len    = buffer[0];
        unsigned char cmd    = buffer[1];
        unsigned char status = buffer[2];
        buffer[len] = 0; /* terminate string */
        //printf("len %d cmd %d status %d\n", (int)len, (int)cmd, (int)status);
        printf("firmware version: %s\n", (char*)(buffer+3));
        //for (i=3; i<len; i++)
        //{
        //   printf("buffer[%d] = %02X\n", i, (int)buffer[i]);
        //}
      }
    }
  }



  /***** FINISHED *****/

  i2c_Finished();
  return 0;
}

/***** END OF FILE *****/
