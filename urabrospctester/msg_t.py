import libscrc

class msgType():
    def __init__(self):
        self.datalength = 0
        self.dataCounter = 0
        self.buffer = b""
        self.crc16  = 0
        self.dataArrived = False
    
    def clear(self):
        self.dataArrived = False
        self.datalength = 0
        self.dataCounter = 0
        self.crc16 = 0
        self.buffer = b""

    def checkDatalen(self):
        if(self.datalength == len(self.buffer)):
            return True
        else:
            return False

    def checkCrc(self):
        crc16 = libscrc.modbus(self.buffer)
        print("Calc: " + hex(crc16) + " msg.crc: " + hex(self.crc16))
        if crc16 == self.crc16 :
            return True
        else :
            return False
