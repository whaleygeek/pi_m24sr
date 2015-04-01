/* based on http://codepad.org/vf3r1EmJ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define CRC_A 1
#define CRC_B 2
#define BYTE unsigned char

unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc)
{
  ch = (ch^(unsigned char)((*lpwCrc) & 0x00FF));
  ch = (ch^(ch<<4));
  *lpwCrc = (*lpwCrc >> 8) ^ ((unsigned short)ch << 8) ^ ((unsigned short)ch<<3) ^ ((unsigned short)ch>>4);
  return(*lpwCrc);
}

void ComputeCrc(int CRCType, char *Data, int Length,
BYTE *TransmitFirst, BYTE *TransmitSecond)
{
  unsigned char chBlock;
  unsigned short wCrc;
  switch(CRCType)
  {
    case CRC_A:
      wCrc = 0x6363; /* ITU-V.41 */
    break;

    case CRC_B:
      wCrc = 0xFFFF; /* ISO/IEC 13239 (formerly ISO/IEC 3309) */
    break;
  }

  do
  {
    chBlock = *Data++;
    UpdateCrc(chBlock, &wCrc);
  }
  while (--Length);

  if (CRCType == CRC_B)
  {
    wCrc = ~wCrc; /* ISO/IEC 13239 (formerly ISO/IEC 3309) */
  }
  *TransmitFirst = (BYTE) (wCrc & 0xFF);
  *TransmitSecond = (BYTE) ((wCrc >> 8) & 0xFF);
  return;
}

BYTE BuffCRC_A[10] = {0x12, 0x34};
BYTE BuffCRC_B[10] = {0x0A, 0x12, 0x34, 0x56};
unsigned short Crc;
BYTE First, Second;
FILE *OutFd;
int i;

int main(void)
{
  printf("CRC-16 reference results ISO/IEC 14443-3\n");
  printf("Crc-16 G(x) = x^16 + x^12 + x^5 + 1\n\n");
  printf("CRC_A of [ ");
  for(i=0; i<2; i++)
  {
    printf("%02X ",BuffCRC_A[i]);
  }
  ComputeCrc(CRC_A, BuffCRC_A, 2, &First, &Second);

  printf("] Transmitted: %02X then %02X.\n", First, Second);
  printf("CRC_B of [ ");

  for(i=0; i<4; i++)
  {
    printf("%02X ",BuffCRC_B[i]);
  }
  ComputeCrc(CRC_B, BuffCRC_B, 4, &First, &Second);

  printf("] Transmitted: %02X then %02X.\n", First, Second);
  return 0;
}
