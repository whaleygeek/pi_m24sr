# m24sr.py  31/03/2015  D.J.Whale
#
# Driver wrapper around I2C to access the M24SR device

#import ci2c as I2C

# DOCUMENTATION
#
# DATA SHEETS (64-KBit EEPROM, NFC Type 4 and I2C interface)
# M24SR64Y http://www.st.com/web/en/resource/technical/document/datasheet/DM00067892.pdf
#
# APPLICATION NOTES
# AN4433: www.st.com/web/en/resource/technical/document/application_NODE/DM00105043.PDF


# CONFIGURATION
#   tag type:    M24SR64Y
#   eeprom size: 64KBit
#   I2C pins:    SDA=2 SCL=3 (Pi B Rev 2)
#   I2C address: 0b1010 110x (8bit 0xAC(r) 0xAD(w)) (7bit 0x56)


# File identifiers
#   0xE101 System file
#   0xE103 CC file (card configuration)
#   0x0001 NDEF file


# SYSTEM FILE LAYOUT (useful to try to read out some known values at start)
#   0000+2 Length of system file   0x0012
#   0002+1 I2C protect             0x01
#   0003+1 I2C watchdog            0x00
#   0004+1 GPO                     0x11
#   0005+1 ST reserved             0x00
#   0006+1 RF enable               0xxxxxxxx1
#   0007+1 NDEF File number        0x00
#   0008+7 UID                     0x0284.. or 0x028C
#   000F+2 Memory size             0x1FFF
#   0011+1 Product code            0x84 or 0x8C



class NFCTag():
    I2C_ADDRESS = 0xAC
    SYSTEM = 0xE101
    CC     = 0xE103
    NDEF   = 0x0001

    def __init__(self, i2c):
        self.i2c = i2c
        self.addr = self.I2C_ADDRESS


    def killRFSelectI2C(self):
        pass # TODO


    def selectFile(self, fileId):
        pass # TODO


    def readBinary(self, offset, length):
        return 0x55AA # TODO


    def deselect(self):
        pass # TODO



# PCB means "protocol control byte",
# It's either 0x02 or 0x03, specifies format of the payload, flags to select different bits
# 0x02 is to send a command (Lc+Data)
# 0x03 is to read a location (Le)

# CLA is class byte (always 0x00 for these apps)
# P1 P2 are parameter 1 and 2,
# Lc is length of command
# Data is the payload of the command
# Le is the length of expected response
# CRC2 is the cyclic redundancy check bytes

# READ MEMORY SIZE (example)
#                                     SEL   PCB     CLA     INS   P1     P2     Lc     Data     Le   CRC2
#   kill RF session, open I2C         0xAC  0x52
#   select system file                0xAC  0x02    0x00    0xA4  0x00   0x0c   0x02   0xE101        0xCCCC
#   read length                       0xAC  0x03    0x00    0xB0  0x00   0x00                   0x02 0xCCCC
#   read memsize                      0xAC  0x03    0x00    0xB0  0x00   0x0F                   0x02 0xCCCC
#   deselect                          0xAC  0xC2                                                     0xE0 B4


# CRC CALCULATION -------------------------------------------------------------
#   calcululate, check
#
# For the I2C frames, the CRC is computed on all data bits of the frame excluding Device select and the CRC itself.
# The CRC is as defined in ISO/IEC 13239.
# The initial register content shall be 0x6363
# and the register content shall not be inverted after calculation.

class CRC():
    def __init__(self, initial=0x6363):
        self.initial = initial


    def start(self):
        self.fcs = self.initial


    def update(self, data):
        data = data ^ ((self.fcs) & 0x00FF)
        data = data ^ (data << 4)
        self.fcs = (self.fcs >> 8) \
                 ^ (data << 8)     \
                 ^ (data << 3)     \
                 ^ (data >> 4)
        return self.fcs


    def getCRC(self):
        return (self.fcs & 0xFF), ((self.fcs & 0xFF00)>>8)


    @staticmethod
    def compute(block):
        c = CRC()
        c.start()
        for i in range(len(block)):
            c.update(block[i])
        crc0, crc1 = c.getCRC()
        return crc0, crc1


def test(id, buf):
    buf_hex = ""
    for i in range(len(buf)):
        buf_hex += hex(buf[i]) + " "

    print("buf " + str(id) + ":" + str(buf_hex))
    crc0, crc1 = CRC.compute(buf)
    print("CRC:" + hex(crc0) + " " + hex(crc1))


#/* Test vectors from: st.com AN4433 */

#/* Select NFC T4 application */
buf1_tx = [0x02,0x00,0xA4,0x04,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01,0x00]
buf1_rx = [0x02,0x90,0x00]

#/* Select CC file */
buf2_tx = [0x03,0x00,0xA4,0x00,0x0C,0x02,0xE1,0x03]

#/* Read CC file length */
buf3_tx = [0x02,0x00,0xB0,0x00,0x00,0x02]

#/* Read CC file */
buf4_tx = [0x03,0x00,0xB0,0x00,0x00,0x0F]

#/* Select NDEF file */
buf5_tx = [0x02,0x00,0xA4,0x00,0x0C,0x02,0x00,0x01]

#/* Read NDEF message length */
buf6_tx = [0x03,0x00,0xB0,0x00,0x00,0x02]
#//NOTE: appnote says 0x6B 0x7D, but returns 0x40, 0x79

#/* Read NDEF message */
buf7_tx = [0x02,0x00,0xB0,0x00,0x02,0x14]
#//NOTE: appnote does not give expected CRC due to printing error

#/* Deselect */
buf8_tx = [0xC2]


def main():
  test("1tx", buf1_tx)
  test("1rx", buf1_rx)
  test("2tx", buf2_tx)
  test("3tx", buf3_tx)
  test("4tx", buf4_tx)
  test("5tx", buf5_tx)
  test("6tx", buf6_tx)
  test("7tx", buf7_tx)
  test("8tx", buf8_tx)


main()


# END

