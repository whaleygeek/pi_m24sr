# test_read.py  31/03/2015  D.J.Whale
#
# Test reading a field from the SYSTEM file ona M24SR Dynamic NFC tag, over I2C
#
# The purpose of this is to check that we can initiate a session with the
# tag reader and get some known bytes of data back. This will verify that
# the I2C is correctly configured and the correct command sequences can
# be sent. It will also check that enough of the physical interface is
# configured correctly (e.g. pullup resistors)

import m24sr
#import ci2c as i2c

# temporary scaffolding
class I2C():
    def initDefaults(self):
        pass

    def finished(self):
        pass

i2c = I2C()


# CONFIGURATION ---------------------------------------------------------------

def wait(msg):
    raw_input(msg)


# MAIN PROGRAM ================================================================

def test():

    wait("create tag object")
    tag = m24sr.NFCTag(i2c)

    # READ MEMORY SIZE REGISTER FROM SYSTEM FILE

    # kill off any RF session, and open an I2C session, claiming the token.
    wait("select I2C")
    tag.killRFSelectI2C()

    # Select the SYSTEM INFO file
    wait("select system file")
    tag.selectFile(tag.SYSTEM)

    # Read the length of the SYSTEM file (should be 0x0012)
    wait("read system file length")
    len = tag.readBinary(0x00, 0x02)
    print("len:" + str(len))

    # Read the memory size field in the SYSTEM file (shoudl be 0x1FFF eeprom size)
    wait("read eeprom size from system file")
    memsize = tag.readBinary(0x000F, 0x02)
    print("memsize:" + str(memsize))

    # Deselect the tag and release the token (RF can now use it)
    wait("deselect")
    tag.deselect()

try:
    wait("init I2C")
    i2c.initDefaults()

    # Loop around repeatedly running the test
    # This makes it easy to see things on the logic analyser
    while True:
        test()

finally:
    # Clean up I2C before exit
    wait("cleaning up")
    i2c.finished()

# END
