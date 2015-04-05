/* NFC_gema.ino  02/04/2015  D.J.Whale
 *
 * Please note: This program is a hardware test program only.
 * It was written under extreme time pressure, and should only
 * be used as a reference that defines a set of sequences that
 * are known to work with the boards listed below. It is not
 * in any way a representation of good programming practice...
 * ... that takes more than a couple of hours to achieve.
 *
 * Test program for Adafruit Gemma to read a URI NDEF record from M24SR.
 * Wire up a X-NUCLEO-NFC01A1 to the SCL/SCA pins.
 * Download this program and run it.
 * 
 * If you get loads of 00 or FF data back, you probably don't have
 * 4.7K pullup resistors on both of SCL and SDA.
 *
 * D2 SCL
 * D0 SDA
 * D1 LED
 */
 
//Modified for testing on Gemma.
 
//#include <Wire.h>
#include <TinyWireM.h>
#define Wire TinyWireM

// LEDs on Gemma
#define LED 1

// LED is used to put a capture region around the NDEF read, so that
// a scope or logic analyser can easily find the frames of interest
// by triggering on rising-high of that pin.
#define CAPTURE  LED
#define USER     LED

#define NFC_ADDR_7BIT 0x56

void setup() 
{                
  pinMode(LED, OUTPUT); 
  //pinMode(LED2, OUTPUT); 
  //pinMode(LED3, OUTPUT); 
  Wire.begin();
  //Serial.begin(9600);
}

byte selectI2C[]      = {0x52};
byte selectNFCApp[]   = {0x02,0x00,0xA4,0x04,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00,0x35,0xC0};
byte selectCCFile[]   = {0x03,0x00,0xA4,0x00,0x0C,0x02,0xE1,0x03,0xD2,0xAF};
byte readCCLen[]      = {0x02,0x00,0xB0,0x00,0x00,0x02,0x6B,0x7D};
byte readCCFile[]     = {0x03,0x00,0xB0,0x00,0x00,0x0F,0xA5,0xA2};
byte selectNDEFFile[] = {0x02,0x00,0xA4,0x00,0x0C,0x02,0x00,0x01,0x3E,0xFD};
byte readNDEFLen[]    = {0x03,0x00,0xB0,0x00,0x00,0x02,0x40,0x79};
byte readNDEFMsg[]    = {0x02,0x00,0xB0,0x00,0x02,0x0C,0xA5,0xA7};
byte deselectI2C[]    = {0xC2,0xE0,0xB4};

// The delays are required, to allow the M24SR time to process commands.

void loop()
{
  //Serial.println("\nstarting");
  //digitalWrite(LED1, HIGH);

  // kill RF, select I2C
  //Serial.println("selectI2C");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(selectI2C, sizeof(selectI2C));
  Wire.endTransmission();
  delay(1);
  
  //select NFC app
  //Serial.println("selectNFCApp");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(selectNFCApp, sizeof(selectNFCApp));
  Wire.endTransmission();
  delay(1);
  //readAndDisplay(5);
  
  
  //select CC file
  //Serial.println("selectCCFile");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(selectCCFile, sizeof(selectCCFile));
  Wire.endTransmission();
  delay(1);
  //readAndDisplay(5);

  //readCCLen
  //Serial.println("readCCLen");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(readCCLen, sizeof(readCCLen));
  Wire.endTransmission();
  delay(1);
  //readAndDisplay(7);

  int len=20; //TODO get from above payload, not overly critical

  //readCCFile
  //Serial.println("readCCFile");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(readCCFile, sizeof(readCCFile));
  Wire.endTransmission();
  delay(1);
  //readAndDisplay(len);
  
  
  //selectNDEFFile
  //Serial.println("selectNDEFFile");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(selectNDEFFile, sizeof(selectNDEFFile));
  Wire.endTransmission();
  delay(1);
  //readAndDisplay(5);
  
  
  //readNDEFLen
  //Serial.println("readNDEFLen");
  //digitalWrite(CAPTURE, HIGH); // so we can capture on logic analyser
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(readNDEFLen, sizeof(readNDEFLen));
  Wire.endTransmission();
  delay(1);
  
  Wire.requestFrom(NFC_ADDR_7BIT, 7);
  int idx = 0;
  while (Wire.available())
  {
    byte b = Wire.read();
    //Serial.print(b, HEX);
    //Serial.write(' ');
    if (idx == 2) // TODO max len is 255?
    {
      len = b;
    }
    idx++;
  }
  //Serial.println();


  //readNDEFMsg
  //len = full NDEF length, (5 byte prefix+actual URI)
  //Serial.println("readNDEFMsg");
  digitalWrite(CAPTURE, HIGH);  
  Wire.beginTransmission(NFC_ADDR_7BIT);
  // Patch in the length and fix the broken CRC
  readNDEFMsg[5] = len; // this is the len returned from ReadNDEFLen
  ComputeCrc(readNDEFMsg, sizeof(readNDEFMsg)-2, &(readNDEFMsg[6]), &(readNDEFMsg[7]));
  Wire.write(readNDEFMsg, sizeof(readNDEFMsg));
  Wire.endTransmission();
  delay(1);

// (PCB,SW1,SW2,CRC0,CRC1)
#define PROTO_OVERHEAD 5
// 5 bytes for NDEF header (in URI format)
#define HEADER_LEN 5
//2 chars for the language of a TEXT record ('en')
#define LANGUAGE_LEN 2

  // Indexes into the I2C response message of start and end of user data (for TEXT)
  // PCB(1)+HEADER(5)+data+SW1(1)_SW2(1)+CRCH(1)+CRCL(1)
  int msgStart = 1 + HEADER_LEN + LANGUAGE_LEN;
  int msgEnd   = 1 + HEADER_LEN + LANGUAGE_LEN + (len-msgStart);
  
  Wire.requestFrom(NFC_ADDR_7BIT, len+PROTO_OVERHEAD);
  idx = 0;
  startMsg();
  while (Wire.available())
  {
    byte b = Wire.read();
    if (idx >= msgStart && idx <= msgEnd)
    {
      processMsg(b);
    }
    idx++;
  }
  endMsg();
  //Serial.println();  
  //readAndDisplay(len+PROTO_OVERHEAD);
  
  digitalWrite(CAPTURE, LOW); // marks end of capture region for logic analyser


  //deselectI2C
  //Serial.println("deselectI2C");
  Wire.beginTransmission(NFC_ADDR_7BIT);
  Wire.write(deselectI2C, sizeof(deselectI2C));
  Wire.endTransmission();
  delay(1);
  //readAndDisplay(3); 
 
  runMsg(); 
  
  delay(1000); // for debugging, separate into phases of read/process/wait
}


//void readAndDisplay(int len)
//{
//  Wire.requestFrom(NFC_ADDR_7BIT, len);
//  while (Wire.available())
//  {
//    Serial.print(Wire.read(), HEX);
//    Serial.write(' ');
//  }
//  Serial.println();
//}


word UpdateCrc(byte data, word *pwCrc)
{
  data = (data^(byte)((*pwCrc) & 0x00FF));
  data = (data^(data << 4));
  *pwCrc = (*pwCrc >> 8) 
         ^ ((word)data << 8) 
         ^ ((word)data << 3) 
         ^ ((word)data >> 4);
  return *pwCrc;
}


word ComputeCrc(byte *data, unsigned int len, byte *crc0, byte *crc1)
{
  byte bBlock;
  word wCrc;

  wCrc = 0x6363;

  do
  {
    bBlock = *data++;
    UpdateCrc(bBlock, &wCrc);
  }
  while (--len);

  *crc0 = (byte) (wCrc & 0xFF);
  *crc1 = (byte) ((wCrc >> 8) & 0xFF);
  return wCrc;
}


/* This is an example of how to extract an NDEF text message.
 * The code calls startMsg() when it thinks it is receiving a message. 
 * This is the point to clear any buffers or reset any index counters.
 * Then it calls processMsg() for every byte received. You should
 * process the byte then increment your indexes.
 * Finally it calls endMsg() when finished.
 * You should now tidy anything up such as terminate buffers.
 * calling runMsg() should playback the sequence.
 */
 
/* This example stores delay lengths in a buffer. Odd indexes are the ON time in ms,
 * event indexes are the OFF time in ms. 0 marks the end of the buffer.
 * ASCII encoding is as follows:
 * '0' => don't use
 * '1' => 100ms
 * '2' => 200ms
 * ...
 * '9' => 900ms
 *
 * You can just about flash morse code using this.
 */
 
#define MSG_MAX 32
int msgIdx;
char msg[MSG_MAX+1];

void startMsg()
{
  msgIdx = 0;
}

void processMsg(byte b)
{
  char ch = (char) b;
  if (msgIdx<MSG_MAX)
  {
    msg[msgIdx++] = ch;
  }
}

void endMsg()
{
  msg[msgIdx] = 0; /* Make sure buffer is terminated */
}

void runMsg()
{
  for (int i=0; i<MSG_MAX; i++)
  {
    char ch = msg[i];
    if (ch == 0)
    {
      break; /* early terminated message */
    }
    if (ch < '1' || ch > '9')
    {
      ch = '1';
    }
    
    int time =  ((int)(ch - '0')) * 100;
    if ((i & 0x01) == 0) // odd message, turn LED on
    {
      digitalWrite(USER, HIGH);
    }
    else // even message, turn LED off
    {
      digitalWrite(USER, LOW);
    }
    delay(time);
  }
  // Always leave LED off at end
  digitalWrite(USER, LOW);
}

/* END OF FILE */

