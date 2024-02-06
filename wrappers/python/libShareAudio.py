from ctypes import *
import time
import os
kill = False
if os.name == 'nt':
    execDll = cdll.LoadLibrary ("./libShareAudio.dll")
else:
    execDll = cdll.LoadLibrary ("./libShareAudio.so")

def SA_Setup(device = -1, host = None, mode = 0, port = 9950, testMode = 0, channel =2, volMod = -1, waveSize = 2048, sampleRate = -1):
    execDll.SA_Setup.argtypes = [c_int,c_char_p,c_int,c_int,c_int,c_int,c_float,c_int,c_double]
    execDll.SA_Setup.restype = c_void_p
    return execDll.SA_Setup(device, host, mode, port, testMode, channel, volMod, waveSize, sampleRate)
    
def SA_ListDevices():
    execDll.SA_ListAllAudioDevices()

def SA_GetVolume(conn):
    execDll.SA_GetVolumeModifier.argtypes = [c_void_p]
    execDll.SA_GetVolumeModifier.restype = c_float
    return execDll.SA_GetVolumeModifier(conn)

def SA_SetVolume(conn, vol):
    execDll.SA_SetVolumeModifier.argtypes = [c_float,c_void_p]
    execDll.SA_SetVolumeModifier(vol,conn)

def SA_Close(conn):
    execDll.SA_Close.argtypes = [c_void_p]
    execDll.SA_Close(conn)

def SA_Shutdown(conn):
    execDll.SA_Shutdown.argtypes = [c_void_p]
    execDll.SA_Shutdown(conn)

def SA_ClientConnect(conn):
    execDll.SA_Client.argtypes = [c_void_p]
    execDll.SA_Client(conn)

def SA_Server(conn):
    execDll.SA_Server.argtypes = [c_void_p]
    execDll.SA_Server(conn)

def SA_Init(conn):
    execDll.SA_Init.argtypes = [c_void_p]
    execDll.SA_Init(conn)

def SA_Test():
    execDll.SA_TestDLL.argtypes = [c_int]
    return execDll.SA_TestDLL()

def SA_GetStats(conn):
    execDll.SA_GetStats.argtypes = [c_void_p]
    execDll.SA_GetStats.restype = c_char_p
    data =  execDll.SA_GetStats(conn).decode("UTF-8")
    dataList = data.split(",");
    dataList[0] = int(dataList[0])
    dataList[1] = int(dataList[1])
    dataList[2] = int(dataList[2])
    dataList[3] = float(dataList[3])
    dataList[4] = int(dataList[4])
    dataList[5] = str(dataList[5])
    dataList[6] = str(dataList[6])
    dataList[7] = int(dataList[7])
    return dataList

def SA_SetLogNULL():
    execDll.SA_SetLogNULL()

def SA_SetLogCONSOLE(debug = 0):
    execDll.SA_SetLogCONSOLE.argtypes = [c_int]
    execDll.SA_SetLogCONSOLE(debug)

def SA_SetLogFILE(filename = "log.txt",debug = 0):
    filename = filename.encode("UTF-8")
    execDll.SA_SetLogFILE.argtypes = [c_char_p,c_int]
    execDll.SA_SetLogFILE(bytes(filename),debug)
    
def SA_SendMsg(msg):
    execDll.SA_SendMsg.argtypes = [c_char_p]
    execDll.SA_SendMsg.restype = c_int
    execDll.SA_SendMsg(msg)
    time.sleep(0.1)

def SA_ReadLastMsg():
    execDll.SA_ReadLastMsg.restype = c_char_p   
    return execDll.SA_ReadLastMsg()

def SA_Kill():
    global kill
    kill = True

def SA_AsyncReadLastMsg():
    msg = SA_ReadLastMsg()
    global kill
    while not kill:
        localMsg = SA_ReadLastMsg()
        time.sleep(0.1)
        if msg != localMsg:
            msg = localMsg
            print(msg.decode("UTF-8") , end="")