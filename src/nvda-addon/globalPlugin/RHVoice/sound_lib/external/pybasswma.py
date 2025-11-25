from __future__ import absolute_import
# Copyright(c) Max Kolosov 2009 maxkolosov@inbox.ru
# http://vosolok2008.narod.ru
# BSD license

__version__ = '0.1'
__versionTime__ = '2009-11-15'
__author__ = 'Max Kolosov <maxkolosov@inbox.ru>'
__doc__ = '''
pybasswma.py - is ctypes python module for
BASSWMA - extension to the BASS audio library,
enabling the playback of WMA files and network streams.
The audio tracks of WMV files can also be played.
WMA file encoding and network broadcasting functions are also provided.

Requirements
============
BASS 2.4 is required. The Windows Media Format modules (v9 or above) are
also required to be installed on the user's system. They are installed with
Windows Media player, so will already be on most users' systems, but they
can also be installed separately (WMFDIST.EXE is available from the BASS website).
'''


import os, sys, ctypes
from . import pybass
from .paths import x86_path, x64_path
from . import libloader

QWORD = pybass.QWORD
HSTREAM = pybass.HSTREAM
BASS_FILEPROCS = pybass.BASS_FILEPROCS

HWMENCODE = ctypes.c_ulong# WMA encoding handle

basswma_module = libloader.load_library('basswma', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()
#Register the plugin with the Bass plugin system.
pybass.BASS_PluginLoad(libloader.find_library_path('basswma', x86_path=x86_path, x64_path=x64_path), 0)


# Additional error codes returned by BASS_ErrorGetCode
BASS_ERROR_WMA_LICENSE = 1000# the file is protected
BASS_ERROR_WMA = 1001# Windows Media (9 or above) is not installed
BASS_ERROR_WMA_WM9 = BASS_ERROR_WMA
BASS_ERROR_WMA_DENIED = 1002# access denied (user/pass is invalid)
BASS_ERROR_WMA_INDIVIDUAL = 1004# individualization is needed
BASS_ERROR_WMA_PUBINIT = 1005# publishing point initialization problem

# Additional BASS_SetConfig options
BASS_CONFIG_WMA_PRECHECK = 0x10100
BASS_CONFIG_WMA_PREBUF = 0x10101
BASS_CONFIG_WMA_BASSFILE = 0x10103
BASS_CONFIG_WMA_NETSEEK = 0x10104
BASS_CONFIG_WMA_VIDEO = 0x10105

# additional WMA sync types
BASS_SYNC_WMA_CHANGE = 0x10100
BASS_SYNC_WMA_META = 0x10101

# additional BASS_StreamGetFilePosition WMA mode
BASS_FILEPOS_WMA_BUFFER = 1000# internet buffering progress (0-100%)

# Additional flags for use with BASS_WMA_EncodeOpen/File/Network/Publish
BASS_WMA_ENCODE_STANDARD = 0x2000# standard WMA
BASS_WMA_ENCODE_PRO = 0x4000# WMA Pro
BASS_WMA_ENCODE_24BIT = 0x8000# 24-bit
BASS_WMA_ENCODE_SCRIPT = 0x20000# set script (mid-stream tags) in the WMA encoding

# Additional flag for use with BASS_WMA_EncodeGetRates
BASS_WMA_ENCODE_RATES_VBR = 0x10000# get available VBR quality settings

#typedef void (CALLBACK CLIENTCONNECTPROC)(HWMENCODE handle, BOOL connect, const char *ip, void *user);
CLIENTCONNECTPROC = func_type(None, HWMENCODE, ctypes.c_byte, ctypes.c_char_p, ctypes.c_void_p) 
# Client connection notification callback function.
#handle : The encoder
#connect: TRUE=client is connecting, FALSE=disconnecting
#ip     : The client's IP (xxx.xxx.xxx.xxx:port)
#user   : The 'user' parameter value given when calling BASS_WMA_EncodeSetNotify

#typedef void (CALLBACK WMENCODEPROC)(HWMENCODE handle, DWORD type, const void *buffer, DWORD length, void *user);
WMENCODEPROC = func_type(None, HWMENCODE, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p) 
# Encoder callback function.
#handle : The encoder handle
#type   : The type of data, one of BASS_WMA_ENCODE_xxx values
#buffer : The encoded data
#length : Length of the data
#user   : The 'user' parameter value given when calling BASS_WMA_EncodeOpen

# WMENCODEPROC "type" values
BASS_WMA_ENCODE_HEAD = 0
BASS_WMA_ENCODE_DATA = 1
BASS_WMA_ENCODE_DONE = 2

# BASS_WMA_EncodeSetTag "form" values
BASS_WMA_TAG_ANSI = 0
BASS_WMA_TAG_UNICODE = 1
BASS_WMA_TAG_UTF8 = 2
BASS_WMA_TAG_BINARY = 0x100# FLAG: binary tag (HIWORD=length)

# BASS_CHANNELINFO type
BASS_CTYPE_STREAM_WMA = 0x10300
BASS_CTYPE_STREAM_WMA_MP3 = 0x10301

# Additional BASS_ChannelGetTags types
BASS_TAG_WMA = 8# WMA header tags : series of null-terminated UTF-8 strings
BASS_TAG_WMA_META = 11# WMA mid-stream tag : UTF-8 string


#HSTREAM BASSWMADEF(BASS_WMA_StreamCreateFile)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags);
BASS_WMA_StreamCreateFile = func_type(HSTREAM, ctypes.c_byte, ctypes.c_void_p, QWORD, QWORD, ctypes.c_ulong)(('BASS_WMA_StreamCreateFile', basswma_module))
#HSTREAM BASSWMADEF(BASS_WMA_StreamCreateFileAuth)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags, const char *user, const char *pass);
BASS_WMA_StreamCreateFileAuth = func_type(HSTREAM, ctypes.c_byte, ctypes.c_void_p, QWORD, QWORD, ctypes.c_ulong, ctypes.c_char_p, ctypes.c_char_p)(('BASS_WMA_StreamCreateFileAuth', basswma_module))
#HSTREAM BASSWMADEF(BASS_WMA_StreamCreateFileUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user);
BASS_WMA_StreamCreateFileUser = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(BASS_FILEPROCS), ctypes.c_void_p)(('BASS_WMA_StreamCreateFileUser', basswma_module))

#const char *BASSWMADEF(BASS_WMA_GetTags)(const void *file, DWORD flags);
BASS_WMA_GetTags = func_type(ctypes.c_char_p, ctypes.c_void_p, ctypes.c_ulong)(('BASS_WMA_GetTags', basswma_module))

#const DWORD *BASSWMADEF(BASS_WMA_EncodeGetRates)(DWORD freq, DWORD chans, DWORD flags);
BASS_WMA_EncodeGetRates = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_WMA_EncodeGetRates', basswma_module))
#HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpen)(DWORD freq, DWORD chans, DWORD flags, DWORD bitrate, WMENCODEPROC *proc, void *user);
BASS_WMA_EncodeOpen = func_type(HWMENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, WMENCODEPROC, ctypes.c_void_p)(('BASS_WMA_EncodeOpen', basswma_module))
#HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenFile)(DWORD freq, DWORD chans, DWORD flags, DWORD bitrate, const char *file);
BASS_WMA_EncodeOpenFile = func_type(HWMENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_char_p)(('BASS_WMA_EncodeOpenFile', basswma_module))
#HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenNetwork)(DWORD freq, DWORD chans, DWORD flags, DWORD bitrate, DWORD port, DWORD clients);
BASS_WMA_EncodeOpenNetwork = func_type(HWMENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_WMA_EncodeOpenNetwork', basswma_module))
#HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenNetworkMulti)(DWORD freq, DWORD chans, DWORD flags, const DWORD *bitrates, DWORD port, DWORD clients);
BASS_WMA_EncodeOpenNetworkMulti = func_type(HWMENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(ctypes.c_ulong), ctypes.c_ulong, ctypes.c_ulong)(('BASS_WMA_EncodeOpenNetworkMulti', basswma_module))
#HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenPublish)(DWORD freq, DWORD chans, DWORD flags, DWORD bitrate, const char *url, const char *user, const char *pass);
BASS_WMA_EncodeOpenPublish = func_type(HWMENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p)(('BASS_WMA_EncodeOpenPublish', basswma_module))
#HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenPublishMulti)(DWORD freq, DWORD chans, DWORD flags, const DWORD *bitrates, const char *url, const char *user, const char *pass);
BASS_WMA_EncodeOpenPublishMulti = func_type(HWMENCODE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(ctypes.c_ulong), ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p)(('BASS_WMA_EncodeOpenPublishMulti', basswma_module))
#DWORD BASSWMADEF(BASS_WMA_EncodeGetPort)(HWMENCODE handle);
BASS_WMA_EncodeGetPort = func_type(ctypes.c_ulong, HWMENCODE)(('BASS_WMA_EncodeGetPort', basswma_module))
#BOOL BASSWMADEF(BASS_WMA_EncodeSetNotify)(HWMENCODE handle, CLIENTCONNECTPROC *proc, void *user);
BASS_WMA_EncodeSetNotify = func_type(ctypes.c_byte, HWMENCODE, CLIENTCONNECTPROC, ctypes.c_void_p)(('BASS_WMA_EncodeSetNotify', basswma_module))
#DWORD BASSWMADEF(BASS_WMA_EncodeGetClients)(HWMENCODE handle);
BASS_WMA_EncodeGetClients = func_type(ctypes.c_ulong, HWMENCODE)(('BASS_WMA_EncodeGetClients', basswma_module))
#BOOL BASSWMADEF(BASS_WMA_EncodeSetTag)(HWMENCODE handle, const char *tag, const char *text, DWORD form);
BASS_WMA_EncodeSetTag = func_type(ctypes.c_byte, HWMENCODE, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_ulong)(('BASS_WMA_EncodeSetTag', basswma_module))
#BOOL BASSWMADEF(BASS_WMA_EncodeWrite)(HWMENCODE handle, const void *buffer, DWORD length);
BASS_WMA_EncodeWrite = func_type(ctypes.c_byte, HWMENCODE, ctypes.c_void_p, ctypes.c_ulong)(('BASS_WMA_EncodeWrite', basswma_module))
#BOOL BASSWMADEF(BASS_WMA_EncodeClose)(HWMENCODE handle);
BASS_WMA_EncodeClose = func_type(ctypes.c_byte, HWMENCODE)(('BASS_WMA_EncodeClose', basswma_module))

#void *BASSWMADEF(BASS_WMA_GetWMObject)(DWORD handle);
BASS_WMA_GetWMObject = func_type(ctypes.c_void_p, ctypes.c_ulong)(('BASS_WMA_GetWMObject', basswma_module))

