/* crc.c  01/04/2015  D.J.Whale
 *
 * Implements ITU-V.41 FCS algorithm.
 */

#include <stdio.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

WORD UpdateCrc(BYTE data, WORD *pwCrc)
{
  data = (data^(BYTE)((*pwCrc) & 0x00FF));
  data = (data^(data << 4));
  *pwCrc = (*pwCrc >> 8) 
         ^ ((WORD)data << 8) 
         ^ ((WORD)data << 3) 
         ^ ((WORD)data >> 4);
  return *pwCrc;
}

WORD ComputeCrc(BYTE *data, unsigned int len, BYTE *crc0, BYTE *crc1)
{
  BYTE bBlock;
  WORD wCrc;

  wCrc = 0x6363;
  //printf("fcs %04X:\n", (unsigned int)wCrc);

  do
  {
    bBlock = *data++;
    UpdateCrc(bBlock, &wCrc);
    //printf("update(%02X) fcs:%04X\n", (unsigned int)bBlock, (unsigned int)wCrc);
  }
  while (--len);

  *crc0 = (BYTE) (wCrc & 0xFF);
  *crc1 = (BYTE) ((wCrc >> 8) & 0xFF);
  return wCrc;
}


void test(char *id, BYTE* buf, unsigned int len)
{
  int i;
  BYTE crc0, crc1;

  printf("buf %s:", id);

  for(i=0; i<len; i++)
  {
    printf("%02X ",buf[i]);
  }

  ComputeCrc(buf, len, &crc0, &crc1);

  printf("\nCRC:%02X %02X\n", (unsigned int)crc0, (unsigned int)crc1);
}


/* Test vectors from: st.com AN4433 */

/* Select NFC T4 application */
BYTE buf1_tx[] = {0x02,0x00,0xA4,0x04,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00};
BYTE buf1_rx[] = {0x02,0x90,0x00};

/* Select CC file */
BYTE buf2_tx[] = {0x03,0x00,0xA4,0x00,0x0C,0x02,0xE1,0x03};

/* Read CC file length */
BYTE buf3_tx[] = {0x02,0x00,0xB0,0x00,0x00,0x02};

/* Read CC file */
BYTE buf4_tx[] = {0x03,0x00,0xB0,0x00,0x00,0x0F};

/* Select NDEF file */
BYTE buf5_tx[] = {0x02,0x00,0xA4,0x00,0x0C,0x02,0x00,0x01};

/* Read NDEF message length */
BYTE buf6_tx[] = {0x03,0x00,0xB0,0x00,0x00,0x02};
//NOTE: appnote says 0x6B 0x7D, but returns 0x40, 0x79

/* Read NDEF message */
BYTE buf7_tx[] = {0x02,0x00,0xB0,0x00,0x02,0x14};
//NOTE: appnote does not give expected CRC due to printing error

/* Deselect */
BYTE buf8_tx[] = {0xC2};


int main(void)
{
  test("1tx", buf1_tx, sizeof(buf1_tx));
  test("1rx", buf1_rx, sizeof(buf1_rx));

  test("2tx", buf2_tx, sizeof(buf2_tx));
  test("3tx", buf3_tx, sizeof(buf3_tx));
  test("4tx", buf4_tx, sizeof(buf4_tx));
  test("5tx", buf5_tx, sizeof(buf5_tx));
  test("6tx", buf6_tx, sizeof(buf6_tx));
  test("7tx", buf7_tx, sizeof(buf7_tx));
  test("8tx", buf8_tx, sizeof(buf8_tx));

  return 0;
}
