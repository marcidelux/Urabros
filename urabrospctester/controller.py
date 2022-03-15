from view import View
from PyQt5 import QtWidgets
from PyQt5 import QtCore
from PyQt5.QtCore import Qt, QThread, pyqtSignal

import threading
import os
import serial
import libscrc
import time
import serial.tools.list_ports

from messageHandler import processMessage
from messageHandler import parseMessage
import messageHandler
from msg_t import msgType

#Global variables
serMaster               = serial.Serial()
serDebug                = serial.Serial()
textBrowserMaster       = QtWidgets.QTextBrowser
textBrowserDebug        = QtWidgets.QTextBrowser
msgRx                   = msgType()

#Constants
red     = "#ff0000"
green   = "#00ff00"
blue    = "#0000ff"
magenta = "#ff00ff"
cyan    = "#00ffff"
yellow  = "#ffff00"

COMMAND_GET_STATUS      = "01"
COMMAND_START           = "02"
COMMAND_DELETE          = "03"
COMMAND_SEND_DATA       = "04"
COMMAND_PAUSE           = "05"
COMMAND_RESUME          = "06"
COMMAND_EMERGENCY_STOP  = "FF"

MESSAGE_URABROS         = 255
MESSAGE_START_OF_TEXT   = 2
MESSAGE_END_OF_TEXT     = 3
MESSAGE_TEXT_MAX_LEN    = 1024

class Controller():
    def __init__(self):
        global textBrowserMaster
        global textBrowserDebug

        self.view = View()
        self.Ui = self.view.ui
        self.slot_SetAvailablePorts()
        
        #Theese were usefull for tests.
        #self.Ui.CbBaudMaster.setCurrentIndex(1)
        #self.Ui.CbComMaster.setCurrentIndex(1)

        self.serialThread = SerialThread()
        textBrowserMaster = self.Ui.MainMessages
        textBrowserDebug  = self.Ui.DebugMessages

        messageHandler.msgInitGlobals()
        
        self.Ui.PbSendTest.setDisabled(True)
        self.connectSignalsSlots()
        self.view.show()

    # SERIAL PORT FUNCTIONS #
    def slot_SetAvailablePorts(self):
        ports = list(serial.tools.list_ports.comports())
        portNames = []
        for p in ports:
            portNames.append(str(p.device))
        portNames.sort()
        self.Ui.CbComMaster.clear()
        for portName in portNames:
            self.Ui.CbComMaster.addItem(portName)

    def slot_ConnectSerialMaster(self):
        global serMaster
        port = self.Ui.CbComMaster.currentText()
        baud = self.Ui.CbBaudMaster.currentText()
        serMaster.port = str(port)
        serMaster.baudrate = int(baud)

        if serMaster.isOpen():
            PrintMaster(magenta,"Already opened")
            return
        else :
            try:
                serMaster.open()
            except:
                PrintMaster(red, "Cant open serial")
                return
                
        if serMaster.isOpen():
            self.serialThread.start()
            PrintMaster(green, "Connected to com: " + port + " baud: " + baud)
                
        if serDebug.isOpen():
            self.serialThread.start()
            PrintDebug(green, "Connected to com: " + port + " baud: " + baud)

    def slot_DisconnectSerialMaster(self):
        global serMaster
        if serMaster.isOpen():
            self.serialThread.terminate()
            serMaster.flush()
            serMaster.close()
            if not serMaster.isOpen():
                msgRx.clear()
                print("SerialMaster closed")
                PrintMaster(green, "Serial closed")
            else:
                print("Closing serialMaster failed")
                PrintMaster(red, "Closing serial failed")

    def slot_ClearMaster(self):
        global textBrowserMaster
        textBrowserMaster.clear()

    def slot_ClearDebug(self):
        global textBrowserDebug
        textBrowserDebug.clear()

    def sendHexData(self, dataString):
        global serMaster
        print("Data to send: " + dataString)
        hexDataArr = bytearray.fromhex(dataString)
        hexDataLen = bytearray((len(hexDataArr)).to_bytes(2, byteorder="big")[1:])
        crc16 = libscrc.modbus(bytes(hexDataArr))
        hexCrc16 = bytearray(crc16.to_bytes(2, byteorder="big"))
        
        print("Hex to Send: " + hexDataLen.hex() + dataString +  hexCrc16.hex())
        
        hexDataToSend = bytearray()
        hexDataToSend.extend(hexDataLen)
        hexDataToSend.extend(hexDataArr)
        hexDataToSend.extend(hexCrc16)
        if serMaster.isOpen():
            serMaster.write(hexDataToSend)
        else:
            PrintMaster(magenta, "Not connected")
    
    def slot_PrintDebug(self, color, val):
        PrintDebug(color, val)    

    def slot_PrintMaster(self, color, val):
        PrintMaster(color, val)    

    # END OF SERIAL PORT FUNCTIONS

    # DATA SENDING FUNCTIONS
    def slot_SendAddCommand(self):
        Tx =  COMMAND_START + format(self.Ui.SbIdAdd.value(), '02x')
        self.sendHexData(Tx)

    def slot_SendDeleteCommand(self):
        Tx =  COMMAND_DELETE + format(self.Ui.SbIdRemove.value(), '02x')
        self.sendHexData(Tx)
    
    def slot_SendGetStatus(self):
        self.sendHexData(COMMAND_GET_STATUS)
    
    def slot_SendTestMessage(self):
        Tx = format(self.Ui.LeDataTest.text())
        self.sendHexData(Tx)
    
    def slot_SendDataToTask(self):
        Tx = COMMAND_SEND_DATA + format(self.Ui.SbIdSendData.value(), '02x') + format(self.Ui.LeDataToTask.text())
        print(Tx)
        self.sendHexData(Tx)
    
    def slot_SendMotorCommand(self):
        Tx = COMMAND_SEND_DATA + "04" + format(self.Ui.CbMotorMode.currentIndex() + 1, '02x') \
            + format(self.Ui.CbMotorDirection.currentIndex(), '02x')
        if(self.Ui.LeMotorSteps.text() != ""):    
            Tx += format(int(self.Ui.LeMotorSteps.text()), '04x')
        if(self.Ui.LeMotorTimeout.text() != ""):
            Tx += format(int(self.Ui.LeMotorTimeout.text()), '04x')
        print(Tx)
        self.sendHexData(Tx)

    # END OF DATA SENDING FUNCTIONS

    # CHECK BOXIES
    def slot_PrintIncomingHex(self):
        messageHandler.msgInitGlobals()
        if self.Ui.CbPrintIncomingHex.isChecked() :
            messageHandler.printRecMessage = True
        else :
            messageHandler.printRecMessage = False
    
    # END OF CHECKBOXIES
    def connectSignalsSlots(self):
        # SERIAL RELEVANT CONNECTIONS
        self.Ui.PbRefreshPorts.clicked.connect(self.slot_SetAvailablePorts)
        self.Ui.PbConnectMaster.clicked.connect(self.slot_ConnectSerialMaster)
        self.Ui.PbDisconnectMaster.clicked.connect(self.slot_DisconnectSerialMaster)
        self.Ui.PbClearMaster.clicked.connect(self.slot_ClearMaster)
        self.Ui.PbClearDebug.clicked.connect(self.slot_ClearDebug)
        self.serialThread.sendMsgDebug.connect(self.slot_PrintDebug, Qt.QueuedConnection)
        self.serialThread.sendMsgMaster.connect(self.slot_PrintMaster, Qt.QueuedConnection)

        # CHECKBOX RELEVANT
        self.Ui.CbPrintIncomingHex.clicked.connect(self.slot_PrintIncomingHex)

        # COMMAND RELEVANT CONNECTIONS
        self.Ui.PbAddCommand.clicked.connect(self.slot_SendAddCommand)
        self.Ui.PbDeleteCommand.clicked.connect(self.slot_SendDeleteCommand)
        self.Ui.PbGetStatus.clicked.connect(self.slot_SendGetStatus)
        self.Ui.PbSendTest.clicked.connect(self.slot_SendTestMessage)
        self.Ui.PbSendDataToTask.clicked.connect(self.slot_SendDataToTask)
        self.Ui.PbSendMotorCommand.clicked.connect(self.slot_SendMotorCommand)

def PrintMaster(color, inp):
    global textBrowserMaster
    tempStringList = inp.split('\n')
    for splitMsg in tempStringList:
            msg = "<span style=\" font-size:10pt; font-weight:600; color:" + color + ";\">" + splitMsg + "</span>"
            textBrowserMaster.append(msg)
            time.sleep(0.01) #Need a small time because append sometimes makes errors
    textBrowserMaster.verticalScrollBar().setValue(textBrowserMaster.verticalScrollBar().maximum())

def PrintDebug(color, inp):
    global textBrowserDebug
    tempStringList = inp.split('\n')
    for splitMsg in tempStringList:
            if splitMsg == "" :
                continue
            msg = "<span style=\" font-size:10pt; font-weight:600; color:" + color + ";\">" + splitMsg + "</span>"
            textBrowserDebug.append(msg)
            time.sleep(0.01) #Need a small time because append sometimes makes errors
    textBrowserDebug.verticalScrollBar().setValue(textBrowserDebug.verticalScrollBar().maximum())

# Thread Class
class SerialThread (QThread):
    sendMsgMaster = pyqtSignal(str, str)
    sendMsgDebug  = pyqtSignal(str, str)
    global msgRx

    firstArrived = False
    messageType = 0
    msgLenCntr = 0

    def run(self):
        global serMaster
        while True:
            if serMaster.isOpen():
                    messageType = int.from_bytes(serMaster.read(), byteorder="big")
                    if messageType == MESSAGE_START_OF_TEXT :
                        tempData        = b''
                        tempTextBuff    = bytearray()
                        msgLenCntr      = 0
                        while msgLenCntr < MESSAGE_TEXT_MAX_LEN:
                            tempData = serMaster.read()
                            if tempData != b'\x03' :
                                tempTextBuff += tempData
                                msgLenCntr += 1
                            else:
                                break
                        print("--- TEXT MSG -->")
                        print(tempTextBuff.decode('utf-8'), end="")
                        print("<-- TEXT MSG ---")
                        self.sendMsgDebug.emit(green, tempTextBuff.decode('utf-8'))

                    elif messageType == MESSAGE_URABROS :
                        tempTextBuff        = bytearray()
                        msgRx.datalength    = int.from_bytes(serMaster.read(), byteorder="big")
                        tempTextBuff        = serMaster.read(size=msgRx.datalength + 2)
                        msgRx.buffer        = tempTextBuff[0:-2]
                        msgRx.crc16         = (tempTextBuff[-2] << 8) + tempTextBuff[-1] 
                        print("--- URAB MSG -->")
                        print(parseMessage(msgRx))
                        print("<-- URAB MSG ---")
                        if msgRx.datalength > 0 :
                            self.sendMsgMaster.emit(green , processMessage(msgRx))
                        
                        msgRx.clear()