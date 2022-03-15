from msg_t import msgType
import libscrc

COMMAND_HEX_GET_STATUS      = 0x01
COMMAND_HEX_START           = 0x02
COMMAND_HEX_DELETE          = 0x03
COMMAND_HEX_SEND_DATA       = 0x04
COMMAND_HEX_PAUSE           = 0x05
COMMAND_HEX_RESUME          = 0x06
COMMAND_HEX_DATA_FROM_TASK  = 0x07
COMMAND_HEX_RECEIVE_ERROR   = 0xFE
COMMAND_HEX_EMERGENCY_STOP  = 0xFF

Command_Hex_Ok              = 0x00
Command_Hex_Added           = 0x01
Command_Hex_NotFinished     = 0x02
Command_Hex_NotFound        = 0x03
Command_Hex_Deleted         = 0x04
Command_Hex_TimedOut        = 0x05
Command_Hex_OwerFlow        = 0x06
Command_Hex_IdAlreadyUsed   = 0x07
Command_Hex_IdOutOfRange    = 0x08
Command_Hex_CantRecData     = 0x09
Command_Hex_Error           = 0xFF

Msg_Hex_CrcError            = 0x03
MSg_Hex_IdleError           = 0x04
Msg_Hex_DataLenError        = 0x05


TaskStatus_Hex_Setup                    = 0x00
TaskStatus_Hex_Running                  = 0x01
TaskStatus_Hex_WaitingForStartSignal    = 0x02
TaskStatus_Hex_WaitingForACKSignal      = 0x03
TaskStatus_Hex_WaitingForInnerSignal    = 0x04
TaskStatus_Hex_Stopped                  = 0x05
TaskStatus_Hex_Error                    = 0x06


def msgInitGlobals():
    global printRecMessage
    printRecMessage = False

def processMessage(msgRx):
    global printRecMessage
    retStr = ""

    calcedCrc16 = libscrc.modbus(msgRx.buffer)
    if(calcedCrc16 != msgRx.crc16):
        return "CRC ERROR"
    
    if printRecMessage == True :
        retStr = parseMessage(msgRx)
        retStr += "\n"

    if msgRx.buffer[0] == COMMAND_HEX_GET_STATUS :
        if msgRx.datalength == 1 :
            retStr += "List is empty"
        else :                
            targetIdx = 1
            print(int((msgRx.datalength - 1) / 2))
            retStr += "| ID | MainStatus | Minor Status |\n"
            for id in range(int((msgRx.datalength - 1) / 2)) :
                mainStatus = msgRx.buffer[targetIdx + 1] >> 5
                minorStatus = msgRx.buffer[targetIdx + 1] & 0b00011111
                retStr += "| ID: " + str(msgRx.buffer[targetIdx])
                if mainStatus == TaskStatus_Hex_Setup:
                    retStr += " | Setup |"    
                elif mainStatus == TaskStatus_Hex_Running:
                    retStr += " | Running |"
                elif mainStatus == TaskStatus_Hex_WaitingForStartSignal:
                    retStr += " | Waiting for Start |"
                elif mainStatus == TaskStatus_Hex_WaitingForACKSignal:
                    retStr += " | Waiting for ACK |"
                elif mainStatus == TaskStatus_Hex_WaitingForInnerSignal:
                    retStr += " | Waiting for Inner |"
                elif mainStatus == TaskStatus_Hex_Stopped:
                    retStr += " | Paused | "
                elif mainStatus == TaskStatus_Hex_Error:
                    retStr += " | Error | "
                retStr +=  str(minorStatus) + " |\n"

                targetIdx += 2

    if msgRx.buffer[0] == COMMAND_HEX_START :
        retStr +=  "Adding Task - Id: " + str(msgRx.buffer[1]) + " - "
        if msgRx.buffer[2] == Command_Hex_Added:
            retStr += "Added"
        elif msgRx.buffer[2] == Command_Hex_NotFinished:
            retStr += "Not Finished"
        elif msgRx.buffer[2] == Command_Hex_TimedOut:
            retStr += "Timeout"
        elif msgRx.buffer[2] == Command_Hex_OwerFlow:
            retStr += "Overflow"
        elif msgRx.buffer[2] == Command_Hex_IdAlreadyUsed:
            retStr += "Already used task ID"
        elif msgRx.buffer[2] == Command_Hex_IdOutOfRange:
            retStr += "Task Id out of Range"
        elif msgRx.buffer[2] == Command_Hex_Error:
            retStr += "Error"

    elif msgRx.buffer[0] == COMMAND_HEX_DELETE :
        retStr +=  "Removing Task - Id: " + str(msgRx.buffer[1]) + " - "
        if msgRx.buffer[2] == Command_Hex_Deleted:
            retStr += "Deleted"
        elif msgRx.buffer[2] == Command_Hex_NotFinished:
            retStr += "Not Finished"
        elif msgRx.buffer[2] == Command_Hex_TimedOut:
            retStr += "Timeout"
        elif msgRx.buffer[2] == Command_Hex_NotFound:
            retStr += "Not Found"
        elif msgRx.buffer[2] == Command_Hex_Error:
            retStr += "Error"

    elif msgRx.buffer[0] == COMMAND_HEX_SEND_DATA :
        retStr += "Send data to Task - Id: " + str(msgRx.buffer[1]) + " - "
        if msgRx.buffer[2] == Command_Hex_IdOutOfRange:
            retStr += "ID out of range"
        elif msgRx.buffer[2] == Command_Hex_OwerFlow:
            retStr += "Task queue is full"
        elif msgRx.buffer[2] == Command_Hex_CantRecData:
            retStr += "Cant receive data"
        elif msgRx.buffer[2] == Command_Hex_Ok:
            retStr += "Ok"

    #elif msgRx.buffer[0] == COMMAND_HEX_PAUSE :
    #elif msgRx.buffer[0] == COMMAND_HEX_RESUME :
    #elif msgRx.buffer[0] == COMMAND_HEX_EMERGENCY_STOP :

    elif msgRx.buffer[0] == COMMAND_HEX_RECEIVE_ERROR :
        if msgRx.buffer[1] == Msg_Hex_CrcError:
            retStr += "CRC Error"
        elif msgRx.buffer[1] == MSg_Hex_IdleError:
            retStr += "Idle error"
        elif msgRx.buffer[1] == Msg_Hex_DataLenError:
            retStr += "Data length error"

    elif msgRx.buffer[0] == COMMAND_HEX_DATA_FROM_TASK :
        retStr += "Data from Task ID: " + str(msgRx.buffer[1]) + " : "
        for data in range(2, int(msgRx.datalength)) :
            retStr += hex(msgRx.buffer[data]).upper() + "|"

    return retStr

def parseMessage(msgRx):
    testStr = "ID: " + str(msgRx.buffer[0]) + "\nLen: " + str(msgRx.datalength) + "\nData: "  + msgRx.buffer.hex() + "\nCRC: " + hex(msgRx.crc16)
    return testStr 