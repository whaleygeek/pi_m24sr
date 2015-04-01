/* crc.c  01/04/2015  D.J.Whale
 *
 * Implements ITU-V.41 FCS algorithm.
 */

#include <stdio.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

WORD UpdateCrc(BYTE ch, WORD *pwCrc)
{
  ch = (ch^(BYTE)((*pwCrc) & 0x00FF));
  ch = (ch^(ch<<4));
  *pwCrc = (*pwCrc >> 8) 
         ^ ((WORD)ch << 8) 
         ^ ((WORD)ch<<3) 
         ^ ((WORD)ch>>4);
  return *pwCrc;
}

WORD ComputeCrc(BYTE *data, unsigned int len, BYTE *crc0, BYTE *crc1)
{
  BYTE bBlock;
  WORD wCrc;

  wCrc = 0x6363;

  do
  {
    bBlock = *data++;
    UpdateCrc(bBlock, &wCrc);
  }
  while (--len);

  *crc0 = (BYTE) (wCrc & 0xFF);
  *crc1 = (BYTE) ((wCrc >> 8) & 0xFF);
  return wCrc;
}


int main(void)
{
  unsigned int i;
  BYTE crc0, crc1;
  BYTE buf[] = {0xC2};

  printf("buf:");

  for(i=0; i<sizeof(buf); i++)
  {
    printf("%02X ",buf[i]);
  }

  //TODO return CRC as result
  ComputeCrc(buf, sizeof(buf), &crc0, &crc1);

  printf("\nCRC:%02X %02X\n", (unsigned int)crc0, (unsigned int)crc1);
  return 0;
}
