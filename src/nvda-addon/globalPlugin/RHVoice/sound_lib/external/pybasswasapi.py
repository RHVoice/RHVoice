# Copyright(c) Andrew Evans 2013
# BSD license

__version__ = '0.1'
__versionTime__ = '2013-03-28'
__author__ = 'Andrew Evans <themindflows@gmail.com>'
__doc__ = '''
pybasswasapi.py - is ctypes python module for WASAPI (http://www.un4seen.com).

BASSWASAPI is basically a wrapper for WASAPI drivers
BASSWASAPI requires a soundcard with WASAPI drivers. 
'''

import sys, ctypes, platform

from . import pybass
from .paths import x86_path, x64_path
from . import libloader


HSTREAM = pybass.HSTREAM
BASS_FILEPROCS = pybass.BASS_FILEPROCS


basswasapi_module = libloader.load_library('basswasapi', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()

# Additional error codes returned by BASS_ErrorGetCode
BASS_ERROR_WASAPI = 5000

# Device info structure
class BASS_WASAPI_DEVICEINFO(ctypes.Structure):
    _fields_ = [
        ('name', ctypes.c_char_p),
        ('id', ctypes.c_char_p),
        ('type', ctypes.c_ulong),
        ('flags', ctypes.c_ulong),
        ('minperiod', ctypes.c_float),
        ('defperiod', ctypes.c_float),
        ('mixfreq', ctypes.c_ulong),
        ('mixchans', ctypes.c_ulong)      
    ]

class BASS_WASAPI_INFO(ctypes.Structure):
    _fields_ = [
        ('initflag', ctypes.c_ulong),
        ('freq', ctypes.c_ulong),
        ('chans', ctypes.c_ulong),
        ('format', ctypes.c_ulong),
        ('buflen', ctypes.c_ulong),
        ('volmax', ctypes.c_float),
        ('volmin', ctypes.c_float),
        ('volstep', ctypes.c_float)
    ]

# BASS_WASAPI_DEVICEINFO "type"
BASS_WASAPI_TYPE_NETWORKDEVICE = 0
BASS_WASAPI_TYPE_SPEAKERS = 1
BASS_WASAPI_TYPE_LINELEVEL = 2
BASS_WASAPI_TYPE_HEADPHONES = 3
BASS_WASAPI_TYPE_MICROPHONE = 4
BASS_WASAPI_TYPE_HEADSET = 5
BASS_WASAPI_TYPE_HANDSET = 6
BASS_WASAPI_TYPE_DIGITAL = 7
BASS_WASAPI_TYPE_SPDIF = 8
BASS_WASAPI_TYPE_HDMI = 9
BASS_WASAPI_TYPE_UNKNOWN = 10

# BASS_WASAPI_DEVICEINFO flags
BASS_DEVICE_ENABLED = 1
BASS_DEVICE_DEFAULT = 2
BASS_DEVICE_INIT = 4
BASS_DEVICE_LOOPBACK = 8
BASS_DEVICE_INPUT = 16
BASS_DEVICE_UNPLUGGED = 32
BASS_DEVICE_DISABLED = 64

# BASS_WASAPI_Init flags
BASS_WASAPI_EXCLUSIVE = 1
BASS_WASAPI_AUTOFORMAT = 2
BASS_WASAPI_BUFFER = 4
BASS_WASAPI_SESSIONVOL = 8
BASS_WASAPI_EVENT = 16

# BASS_WASAPI_INFO "format"
BASS_WASAPI_FORMAT_FLOAT = 0
BASS_WASAPI_FORMAT_8BIT = 1
BASS_WASAPI_FORMAT_16BIT = 2
BASS_WASAPI_FORMAT_24BIT = 3
BASS_WASAPI_FORMAT_32BIT = 4

# BASS_WASAPI_Set/GetVolume "curve"
BASS_WASAPI_CURVE_DB = 0
BASS_WASAPI_CURVE_LINEAR = 1
BASS_WASAPI_CURVE_WINDOWS = 2

#typedef DWORD (CALLBACK WASAPIPROC)(void *buffer, DWORD length, void *user);
WASAPIPROC = func_type(ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p)

#typedef void (CALLBACK WASAPINOTIFYPROC)(DWORD notify, DWORD device, void *user);
WASAPINOTIFYPROC = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p)

# Device notifications
BASS_WASAPI_NOTIFY_ENABLED = 0
BASS_WASAPI_NOTIFY_DISABLED = 1
BASS_WASAPI_NOTIFY_DEFOUTPUT = 2
BASS_WASAPI_NOTIFY_DEFINPUT = 3


# DWORD BASSWASAPIDEF(BASS_WASAPI_GetVersion)();
BASS_WASAPI_GetVersion = func_type(HSTREAM)(('BASS_WASAPI_GetVersion', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_SetNotify)(WASAPINOTIFYPROC *proc, void *user);
BASS_WASAPI_SetNotify = func_type(HSTREAM, ctypes.POINTER(WASAPINOTIFYPROC), ctypes.c_void_p)(('BASS_WASAPI_SetNotify', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_GetDeviceInfo)(DWORD device, BASS_WASAPI_DEVICEINFO *info);
BASS_WASAPI_GetDeviceInfo = func_type(HSTREAM, ctypes.c_ulong, ctypes.POINTER(BASS_WASAPI_DEVICEINFO))(('BASS_WASAPI_GetDeviceInfo', basswasapi_module))
# float BASSDEF(BASS_WASAPI_GetDeviceLevel)(DWORD device, int chan);
BASS_WASAPI_GetDeviceLevel = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_int)(('BASS_WASAPI_GetDeviceLevel', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_SetDevice)(DWORD device);
BASS_WASAPI_SetDevice = func_type(HSTREAM, ctypes.c_ulong)(('BASS_WASAPI_SetDevice', basswasapi_module))
# DWORD BASSWASAPIDEF(BASS_WASAPI_GetDevice)();
BASS_WASAPI_GetDevice = func_type(HSTREAM)(('BASS_WASAPI_GetDevice', basswasapi_module))
# DWORD BASSWASAPIDEF(BASS_WASAPI_CheckFormat)(DWORD device, DWORD freq, DWORD chans, DWORD flags);
BASS_WASAPI_CheckFormat = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_WASAPI_CheckFormat', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_Init)(int device, DWORD freq, DWORD chans, DWORD flags, float buffer, float period, WASAPIPROC *proc, void *user);
BASS_WASAPI_Init = func_type(HSTREAM, ctypes.c_int, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_float, ctypes.c_float, WASAPIPROC, ctypes.c_void_p)(('BASS_WASAPI_Init', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_Free)();
BASS_WASAPI_Free = func_type(HSTREAM)(('BASS_WASAPI_Free', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_GetInfo)(BASS_WASAPI_INFO *info);
BASS_WASAPI_GetInfo = func_type(HSTREAM, ctypes.POINTER(BASS_WASAPI_INFO))(('BASS_WASAPI_GetInfo', basswasapi_module))
# float BASSWASAPIDEF(BASS_WASAPI_GetCPU)();
BASS_WASAPI_GetCPU = func_type(HSTREAM)(('BASS_WASAPI_GetCPU', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_Lock)(BOOL lock);
BASS_WASAPI_Lock = func_type(HSTREAM, ctypes.c_bool)(('BASS_WASAPI_Lock', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_Start)();
BASS_WASAPI_Start = func_type(HSTREAM)(('BASS_WASAPI_Start', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_Stop)(BOOL reset);
BASS_WASAPI_Stop = func_type(ctypes.c_bool, ctypes.c_bool)(('BASS_WASAPI_Stop', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_IsStarted)();
BASS_WASAPI_IsStarted = func_type(HSTREAM)(('BASS_WASAPI_IsStarted', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_SetVolume)(DWORD curve, float volume);
BASS_WASAPI_SetVolume = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_float)(('BASS_WASAPI_SetVolume', basswasapi_module))
# float BASSWASAPIDEF(BASS_WASAPI_GetVolume)(DWORD curve);
BASS_WASAPI_GetVolume = func_type(HSTREAM, ctypes.c_ulong)(('BASS_WASAPI_GetVolume', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_SetMute)(BOOL mute);
BASS_WASAPI_SetMute = func_type(HSTREAM, ctypes.c_bool)(('BASS_WASAPI_SetMute', basswasapi_module))
# BOOL BASSWASAPIDEF(BASS_WASAPI_GetMute)();
BASS_WASAPI_GetMute = func_type(HSTREAM)(('BASS_WASAPI_GetMute', basswasapi_module))
# DWORD BASSWASAPIDEF(BASS_WASAPI_PutData)(void *buffer, DWORD length);
BASS_WASAPI_PutData = func_type(HSTREAM, ctypes.c_void_p, ctypes.c_ulong)(('BASS_WASAPI_PutData', basswasapi_module))
# DWORD BASSDEF(BASS_WASAPI_GetData)(void *buffer, DWORD length);
BASS_WASAPI_GetData = func_type(HSTREAM, ctypes.c_void_p, ctypes.c_ulong)(('BASS_WASAPI_GetData', basswasapi_module))
# DWORD BASSDEF(BASS_WASAPI_GetLevel)();
BASS_WASAPI_GetLevel = func_type(HSTREAM)(('BASS_WASAPI_GetLevel', basswasapi_module))

