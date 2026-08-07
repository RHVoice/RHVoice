from __future__ import absolute_import
"BASSENC wrapper by Christopher Toth"""

import ctypes
import os
import platform
from . import pybass
from .paths import x86_path, x64_path
from . import libloader

bassenc_module = libloader.load_library('bassenc', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()

HENCODE = ctypes.c_ulong #encoder handle

#Additional error codes returned by BASS_ErrorGetCode
BASS_ERROR_ACM_CANCEL = 2000 #ACM codec selection cancelled
pybass.error_descriptions[BASS_ERROR_ACM_CANCEL] = "ACM codec selection cancelled"
BASS_ERROR_CAST_DENIED = 2100 #access denied (invalid password)
pybass.error_descriptions[BASS_ERROR_CAST_DENIED] = "access denied (invalid password)"

#Additional BASS_SetConfig options
BASS_CONFIG_ENCODE_PRIORITY = 0x10300
BASS_CONFIG_ENCODE_QUEUE = 0x10301
BASS_CONFIG_ENCODE_CAST_TIMEOUT = 0x10310

#Additional BASS_SetConfigPtr options
BASS_CONFIG_ENCODE_CAST_PROXY = 0x10311

#BASS_Encode_Start flags
BASS_ENCODE_NOHEAD = 1 #don't send a WAV header to the encoder
BASS_ENCODE_FP_8BIT = 2	#convert floating-point sample data to 8-bit integer
BASS_ENCODE_FP_16BIT = 4 #convert floating-point sample data to 16-bit integer
BASS_ENCODE_FP_24BIT = 6#convert floating-point sample data to 24-bit integer
BASS_ENCODE_FP_32BIT = 8 #convert floating-point sample data to 32-bit integer
BASS_ENCODE_BIGEND = 16 #big-endian sample data
BASS_ENCODE_PAUSE = 32	#start encoding paused
BASS_ENCODE_PCM = 64 #write PCM sample data (no encoder)
BASS_ENCODE_RF64 = 128 #send an RF64 header
BASS_ENCODE_MONO = 256 #convert to mono (if not already)
BASS_ENCODE_QUEUE = 512 #queue data to feed encoder asynchronously
BASS_ENCODE_CAST_NOLIMIT = 0x1000	#don't limit casting data rate
BASS_ENCODE_LIMIT = 0x2000	#limit data rate to real-time
BASS_ENCODE_AUTOFREE = 0x40000	#free the encoder when the channel is freed

#BASS_Encode_GetACMFormat flags
BASS_ACM_DEFAULT = 1 #use the format as default selection
BASS_ACM_RATE = 2	#only list formats with same sample rate as the source channel
BASS_ACM_CHANS = 4 #only list formats with same number of channels (eg. mono/stereo)
BASS_ACM_SUGGEST = 8 #suggest a format (HIWORD=format tag)

#BASS_Encode_GetCount counts

(
 BASS_ENCODE_COUNT_IN, #sent to encoder
 BASS_ENCODE_COUNT_OUT, #received from encoder
 BASS_ENCODE_COUNT_CAST,	#sent to cast server
 BASS_ENCODE_COUNT_QUEUE,	#queued
 BASS_ENCODE_COUNT_QUEUE_LIMIT, #queue limit
 BASS_ENCODE_COUNT_QUEUE_FAIL, #failed to queue
) = range(6)

#BASS_Encode_CastInit content MIME types

BASS_ENCODE_TYPE_MP3 = "audio/mpeg"
BASS_ENCODE_TYPE_OGG = "application/ogg"
BASS_ENCODE_TYPE_AAC = "audio/aacp"

#BASS_Encode_CastGetStats types

BASS_ENCODE_STATS_SHOUT = 0	#Shoutcast stats
BASS_ENCODE_STATS_ICE = 1	#Icecast mount-point stats
BASS_ENCODE_STATS_ICESERV = 2	#Icecast server stats

#typedef void (CALLBACK ENCODEPROC)(HENCODE handle, DWORD channel, const void *buffer, DWORD length, void *user);
ENCODEPROC = func_type(None, HENCODE, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p)

#typedef void (CALLBACK ENCODEPROCEX)(HENCODE handle, DWORD channel, const void *buffer, DWORD length, QWORD offset, void *user);
ENCODEPROCEX = func_type(None, HENCODE, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, pybass.QWORD, ctypes.c_void_p)

#typedef BOOL (CALLBACK ENCODECLIENTPROC)(HENCODE handle, BOOL connect, const char *client, char *headers, void *user);
ENCODECLIENTPROC = func_type(ctypes.c_byte, HENCODE, ctypes.c_byte, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_void_p)

#typedef void (CALLBACK ENCODENOTIFYPROC)(HENCODE handle, DWORD status, void *user);
ENCODENOTIFYPROC = func_type(None, HENCODE, ctypes.c_ulong, ctypes.c_void_p)

#Encoder notifications
BASS_ENCODE_NOTIFY_ENCODER = 1 #encoder died
BASS_ENCODE_NOTIFY_CAST = 2 #cast server connection died
BASS_ENCODE_NOTIFY_CAST_TIMEOUT = 0x10000 #cast timeout
BASS_ENCODE_NOTIFY_QUEUE_FULL = 0x10001	#queue is out of space

#BASS_Encode_ServerInit flags
BASS_ENCODE_SERVER_NOHTTP = 1	#no HTTP headers

#DWORD BASSENCDEF(BASS_Encode_GetVersion)();
BASS_Encode_GetVersion = func_type(ctypes.c_ulong)(('BASS_Encode_GetVersion', bassenc_module))

#HENCODE BASSENCDEF(BASS_Encode_Start)(DWORD handle, const char *cmdline, DWORD flags, ENCODEPROC *proc, void *user);
BASS_Encode_Start = func_type(HENCODE, ctypes.c_ulong, ctypes.c_char_p, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_void_p)(('BASS_Encode_Start', bassenc_module))

#HENCODE BASSENCDEF(BASS_Encode_StartLimit)(DWORD handle, const char *cmdline, DWORD flags, ENCODEPROC *proc, void *user, DWORD limit);
BASS_Encode_StartLimit = func_type(HENCODE, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ENCODEPROC, ctypes.c_void_p, ctypes.c_ulong)(('BASS_Encode_StartLimit', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_AddChunk)(HENCODE handle, const char *id, const void *buffer, DWORD length);
BASS_Encode_AddChunk = func_type(ctypes.c_byte, HENCODE, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_ulong)(('BASS_Encode_AddChunk', bassenc_module))

#DWORD BASSENCDEF(BASS_Encode_IsActive)(DWORD handle);
BASS_Encode_IsActive = func_type(ctypes.c_ulong, ctypes.c_ulong)(('BASS_Encode_IsActive', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_Stop)(DWORD handle);
BASS_Encode_Stop = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_Encode_Stop', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_StopEx)(DWORD handle, BOOL queue);
BASS_Encode_StopEx = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_byte)

#BOOL BASSENCDEF(BASS_Encode_SetPaused)(DWORD handle, BOOL paused);
BASS_Encode_SetPaused = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_byte)(('BASS_Encode_SetPaused', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_Write)(DWORD handle, const void *buffer, DWORD length);
BASS_Encode_Write = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong)(('BASS_Encode_Write', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_SetNotify)(DWORD handle, ENCODENOTIFYPROC *proc, void *user);
BASS_Encode_SetNotify = func_type(ctypes.c_byte, ctypes.c_ulong, ENCODENOTIFYPROC, ctypes.c_void_p)(('BASS_Encode_SetNotify', bassenc_module))

#QWORD BASSENCDEF(BASS_Encode_GetCount)(DWORD handle, DWORD count);
BASS_Encode_GetCount = func_type(pybass.QWORD, ctypes.c_ulong, ctypes.c_ulong)(('BASS_Encode_GetCount', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_SetChannel)(DWORD handle, DWORD channel);
BASS_Encode_SetChannel = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)

#DWORD BASSENCDEF(BASS_Encode_GetChannel)(HENCODE handle);
BASS_Encode_GetChannel = func_type(ctypes.c_ulong, HENCODE)(('BASS_Encode_GetChannel', bassenc_module))

if platform.system() == 'Windows':
 #DWORD BASSENCDEF(BASS_Encode_GetACMFormat)(DWORD handle, void *form, DWORD formlen, const char *title, DWORD flags);
 BASS_Encode_GetACMFormat = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_char_p, ctypes.c_ulong)(('BASS_Encode_GetACMFormat', bassenc_module))

 #HENCODE BASSENCDEF(BASS_Encode_StartACM)(DWORD handle, const void *form, DWORD flags, ENCODEPROC *proc, void *user);
 BASS_Encode_StartACM = func_type(HENCODE, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ENCODEPROC, ctypes.c_void_p)(('BASS_Encode_StartACM', bassenc_module))

 #HENCODE BASSENCDEF(BASS_Encode_StartACMFile)(DWORD handle, const void *form, DWORD flags, const char *file);
 BASS_Encode_StartACMFile = func_type(HENCODE, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_char_p)(('BASS_Encode_StartACMFile', bassenc_module))


if platform.system() == 'Darwin':
 #HENCODE BASSENCDEF(BASS_Encode_StartCA)(DWORD handle, DWORD ftype, DWORD atype, DWORD flags, DWORD bitrate, ENCODEPROCEX *proc, void *user);
 BASS_Encode_StartCA = func_type(HENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ENCODEPROCEX, ctypes.c_void_p)(('ENCODEPROCEX ', bassenc_module))
#HENCODE BASSENCDEF(BASS_Encode_StartCAFile)(DWORD handle, DWORD ftype, DWORD atype, DWORD flags, DWORD bitrate, const char *file);
 BASS_Encode_StartCAFile = func_type(HENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_char_p)(('BASS_Encode_StartCAFile', bassenc_module))
#Broadcasting

#BOOL BASSENCDEF(BASS_Encode_CastInit)(HENCODE handle, const char *server, const char *pass, const char *content, const char *name, const char *url, const char *genre, const char *desc, const char *headers, DWORD bitrate, BOOL pub);
BASS_Encode_CastInit = func_type(ctypes.c_byte, HENCODE, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_ulong, ctypes.c_byte)(('BASS_Encode_CastInit', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_CastSetTitle)(HENCODE handle, const char *title, const char *url);
BASS_Encode_CastSetTitle = func_type(ctypes.c_byte, HENCODE, ctypes.c_char_p, ctypes.c_char_p)(('BASS_Encode_CastSetTitle', bassenc_module))

#const char *BASSENCDEF(BASS_Encode_CastGetStats)(HENCODE handle, DWORD type, const char *pass);
BASS_Encode_CastGetStats = func_type(ctypes.c_char_p, HENCODE, ctypes.c_ulong, ctypes.c_char_p)(('BASS_Encode_CastGetStats', bassenc_module))

#Local audio server

#DWORD BASSENCDEF(BASS_Encode_ServerInit)(HENCODE handle, const char *port, DWORD buffer, DWORD burst, DWORD flags, ENCODECLIENTPROC *proc, void *user);
BASS_Encode_ServerInit = func_type(ctypes.c_ulong, HENCODE, ctypes.c_char_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_void_p)(('BASS_Encode_ServerInit', bassenc_module))

#BOOL BASSENCDEF(BASS_Encode_ServerKick)(HENCODE handle, const char *client);
BASS_Encode_ServerKick = func_type(ctypes.c_byte, HENCODE, ctypes.c_char_p)(('BASS_Encode_ServerKick', bassenc_module))

