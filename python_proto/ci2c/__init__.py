# ci2c  19/07/2014  D.J.Whale
# 
# a C based I2C driver, with a python wrapper

import ctypes

from os import path
mydir = path.dirname(path.abspath(__file__))

libi2c               = ctypes.cdll.LoadLibrary(mydir + "/i2c.so")
gpio_init_fn         = libi2c["gpio_init"]
i2c_InitDefaults_fn  = libi2c["i2c_InitDefaults"]
i2c_Init_fn          = libi2c["i2c_Init"]
i2c_Start_fn         = libi2c["i2c_Start"]
i2c_Stop_fn          = libi2c["i2c_Stop"]
i2c_TxByte_fn        = libi2c["i2c_TxByte"]
i2c_RxByte_fn        = libi2c["i2c_RxByte"]
i2c_Write_fn         = libi2c["i2c_Write"]
i2c_Read_fn          = libi2c["i2c_Read"]
i2c_Finished_fn      = libi2c["i2c_Finished"]

def trace(msg):
  print("i2c:" + msg)

def initDefaults():
  # void i2c_InitDefaults(void);
  #trace("calling gpio_init")
  gpio_init_fn()

  #trace("calling InitDefaults")
  result = i2c_InitDefaults_fn()
  #trace("returned from InitDefaults")
  return result

#def init():
#  # void i2c_Init(I2C_CONFIG* pConfig);
#  trace("###will call Init")
#  #build config as a structure
#  #result = i2c_Init_fn(byref(config))
#  pass
  
def start():
  # I2CRESULT i2c_Start(void);
  #trace("calling start")
  result = i2c_Start_fn()
  return result
  
def stop():
  # I2CRESULT i2c_Stop(void);
  #trace("calling stop")
  result = i2c_Stop_fn()
  return result
  
def txByte(data):
  # I2CRESULT i2c_TxByte(unsigned char data);
  #trace("calling TxByte")
  p_data = ctypes.c_ubyte(data)
  result = i2c_TxByte_fn(p_data)
  return result
  
def rxByte(ack):
  # I2CRESULT i2c_RxByte(unsigned char* pData);
  #trace("calling RxByte")
  p_data = ctypes.c_ubyte()
  p_ack  = ctypes.c_ubyte(ack)
  result = i2c_RxByte_fn(ctypes.byref(p_data), p_ack)
  return result, p_data.value

def write(addr, buf):
  # I2C_RESULT i2c_Write(unsigned char addr, unsigned char* pData,
  #   unsigned char len
  #print("addr:" + str(addr))
  #print("buf:" + str(buf))

  p_addr = ctypes.c_ubyte(addr)
  Buffer = ctypes.c_ubyte * len(buf)
  p_buf  = Buffer(*buf)
  p_len  = ctypes.c_ubyte(len(buf))

  #print(p_addr)
  #print(p_buf)
  #print(p_len)

  #trace("calling i2c_Write")
  result = i2c_Write_fn(p_addr, ctypes.byref(p_buf), p_len)
  return result
  
def read(addr, maxlen):
  # I2C_RESULT i2c_Read(unsigned char addr, unsigned char* pData,
  # unsigned char len)
  #trace("#### will call Read")
  p_addr = ctypes.c_ubyte(addr)
  Buffer = ctypes.c_ubyte * maxlen
  p_buf = Buffer()
  p_len  = ctypes.c_ubyte(maxlen)
  result = i2c_Read_fn(p_addr, ctypes.byref(p_buf), p_len)

  rxlist = []
  for i in range(maxlen):
    rxlist.append(p_buf[i])

  #print("read result:" + str(result))
  #print("read buf:" + str(rxlist))
  return result, rxlist
  
def finished():
  # I2CRESULT i2c_Finished(void);
  #trace("calling finished")
  result = i2c_Finished_fn()
  return result
  
# END OF FILE
