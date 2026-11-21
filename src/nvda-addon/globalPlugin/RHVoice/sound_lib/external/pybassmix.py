from __future__ import absolute_import
# Copyright(c) Max Kolosov 2009 maxkolosov@inbox.ru
# http://vosolok2008.narod.ru
# BSD license

__version__ = '0.1'
__versionTime__ = '2009-11-15'
__author__ = 'Max Kolosov <maxkolosov@inbox.ru>'
__doc__ = '''
pybassmix.py - is ctypes python module for
BASSmix - extension to the BASS audio library, providing the ability
to mix together multiple BASS channels, with resampling and matrix mixing
features. It also provides the ability to go the other way and split a
BASS channel into multiple channels.
'''

import os, sys, ctypes, platform, pybass

QWORD = pybass.QWORD
HSYNC = pybass.HSYNC
HSTREAM = pybass.HSTREAM
DOWNLOADPROC = pybass.DOWNLOADPROC
SYNCPROC = pybass.SYNCPROC
BASS_FILEPROCS = pybass.BASS_FILEPROCS

from .paths import x86_path, x64_path
from . import libloader

bassmix_module = libloader.load_library('bassmix', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()
	
# additional BASS_SetConfig option
BASS_CONFIG_MIXER_FILTER = 0x10600
BASS_CONFIG_MIXER_BUFFER = 0x10601
BASS_CONFIG_SPLIT_BUFFER = 0x10610

# BASS_Mixer_StreamCreate flags
BASS_MIXER_END = 0x10000# end the stream when there are no sources
BASS_MIXER_NONSTOP = 0x20000# don't stall when there are no sources
BASS_MIXER_RESUME = 0x1000# resume stalled immediately upon new/unpaused source

# source flags
BASS_MIXER_FILTER = 0x1000# resampling filter
BASS_MIXER_BUFFER = 0x2000# buffer data for BASS_Mixer_ChannelGetData/Level
BASS_MIXER_LIMIT = 0x4000# limit mixer processing to the amount available from this source
BASS_MIXER_MATRIX = 0x10000# matrix mixing
BASS_MIXER_PAUSE = 0x20000# don't process the source
BASS_MIXER_DOWNMIX = 0x400000# downmix to stereo/mono
BASS_MIXER_NORAMPIN = 0x800000# don't ramp-in the start

# envelope node
class BASS_MIXER_NODE(ctypes.Structure):
	_fields_ = [('pos', ctypes.c_ulong),#QWORD pos;
				('value', ctypes.c_float)#float value;
				]

# envelope types
BASS_MIXER_ENV_FREQ = 1
BASS_MIXER_ENV_VOL = 2
BASS_MIXER_ENV_PAN = 3
BASS_MIXER_ENV_LOOP = 0x10000# FLAG: loop

# additional sync type
BASS_SYNC_MIXER_ENVELOPE = 0x10200

# BASS_CHANNELINFO type
BASS_CTYPE_STREAM_MIXER = 0x10800
BASS_CTYPE_STREAM_SPLIT = 0x10801


#DWORD BASSMIXDEF(BASS_Mixer_GetVersion)();
BASS_Mixer_GetVersion = func_type(ctypes.c_ulong)(('BASS_Mixer_GetVersion', bassmix_module))

#HSTREAM BASSMIXDEF(BASS_Mixer_StreamCreate)(DWORD freq, DWORD chans, DWORD flags);
BASS_Mixer_StreamCreate = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_Mixer_StreamCreate', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_StreamAddChannel)(HSTREAM handle, DWORD channel, DWORD flags);
BASS_Mixer_StreamAddChannel = func_type(ctypes.c_byte, HSTREAM, ctypes.c_ulong, ctypes.c_ulong)(('BASS_Mixer_StreamAddChannel', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_StreamAddChannelEx)(HSTREAM handle, DWORD channel, DWORD flags, QWORD start, QWORD length);
BASS_Mixer_StreamAddChannelEx = func_type(ctypes.c_byte, HSTREAM, ctypes.c_ulong, ctypes.c_ulong, QWORD, QWORD)(('BASS_Mixer_StreamAddChannelEx', bassmix_module))

#HSTREAM BASSMIXDEF(BASS_Mixer_ChannelGetMixer)(DWORD handle);
BASS_Mixer_ChannelGetMixer = func_type(HSTREAM, ctypes.c_ulong)(('BASS_Mixer_ChannelGetMixer', bassmix_module))
#DWORD BASSMIXDEF(BASS_Mixer_ChannelFlags)(DWORD handle, DWORD flags, DWORD mask);
BASS_Mixer_ChannelFlags = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_Mixer_ChannelFlags', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelRemove)(DWORD handle);
BASS_Mixer_ChannelRemove = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_Mixer_ChannelRemove', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelSetPosition)(DWORD handle, QWORD pos, DWORD mode);
BASS_Mixer_ChannelSetPosition = func_type(ctypes.c_byte, ctypes.c_ulong, QWORD, ctypes.c_ulong)(('BASS_Mixer_ChannelSetPosition', bassmix_module))
#QWORD BASSMIXDEF(BASS_Mixer_ChannelGetPosition)(DWORD handle, DWORD mode);
BASS_Mixer_ChannelGetPosition = func_type(QWORD, ctypes.c_ulong, ctypes.c_ulong)(('BASS_Mixer_ChannelGetPosition', bassmix_module))
#DWORD BASSMIXDEF(BASS_Mixer_ChannelGetLevel)(DWORD handle);
BASS_Mixer_ChannelGetLevel = func_type(ctypes.c_ulong, ctypes.c_ulong)(('BASS_Mixer_ChannelGetLevel', bassmix_module))
#DWORD BASSMIXDEF(BASS_Mixer_ChannelGetData)(DWORD handle, void *buffer, DWORD length);
BASS_Mixer_ChannelGetData = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong)(('BASS_Mixer_ChannelGetData', bassmix_module))
#HSYNC BASSMIXDEF(BASS_Mixer_ChannelSetSync)(DWORD handle, DWORD type, QWORD param, SYNCPROC *proc, void *user);
BASS_Mixer_ChannelSetSync = func_type(HSYNC, ctypes.c_ulong, ctypes.c_ulong, QWORD, SYNCPROC, ctypes.c_void_p)(('BASS_Mixer_ChannelSetSync', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelRemoveSync)(DWORD channel, HSYNC sync);
BASS_Mixer_ChannelRemoveSync = func_type(ctypes.c_byte, ctypes.c_ulong, HSYNC)(('BASS_Mixer_ChannelRemoveSync', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelSetMatrix)(DWORD handle, const float *matrix);
BASS_Mixer_ChannelSetMatrix = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(ctypes.c_float))(('BASS_Mixer_ChannelSetMatrix', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelGetMatrix)(DWORD handle, float *matrix);
BASS_Mixer_ChannelGetMatrix = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(ctypes.c_float))(('BASS_Mixer_ChannelGetMatrix', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelSetEnvelope)(DWORD handle, DWORD type, const BASS_MIXER_NODE *nodes, DWORD count);
BASS_Mixer_ChannelSetEnvelope = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(BASS_MIXER_NODE), ctypes.c_ulong)(('BASS_Mixer_ChannelSetEnvelope', bassmix_module))
#BOOL BASSMIXDEF(BASS_Mixer_ChannelSetEnvelopePos)(DWORD handle, DWORD type, QWORD pos);
BASS_Mixer_ChannelSetEnvelopePos = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong, QWORD)(('BASS_Mixer_ChannelSetEnvelopePos', bassmix_module))
#QWORD BASSMIXDEF(BASS_Mixer_ChannelGetEnvelopePos)(DWORD handle, DWORD type, float *value);
BASS_Mixer_ChannelGetEnvelopePos = func_type(QWORD, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_float)(('BASS_Mixer_ChannelGetEnvelopePos', bassmix_module))

#HSTREAM BASSMIXDEF(BASS_Split_StreamCreate)(DWORD channel, DWORD flags, int *chanmap);
BASS_Split_StreamCreate = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_int)(('BASS_Split_StreamCreate', bassmix_module))
#DWORD BASSMIXDEF(BASS_Split_StreamGetSource)(HSTREAM handle);
BASS_Split_StreamGetSource = func_type(ctypes.c_ulong, HSTREAM)(('BASS_Split_StreamGetSource', bassmix_module))
#BOOL BASSMIXDEF(BASS_Split_StreamReset)(DWORD handle);
BASS_Split_StreamReset = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_Split_StreamReset', bassmix_module))

