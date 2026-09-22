# copyright(c) Max Kolosov 2009 maxkolosov@inbox.ru
# http://vosolok2008.narod.ru
# BSD license

__version__ = '0.5.1'
__versionTime__ = '2009-11-19'
__author__ = 'Max Kolosov <maxkolosov@inbox.ru>'
__doc__ = '''
pybass.py - is ctypes python module for BASS (http://www.un4seen.com).

BASS is an audio library for use in Windows, Linux and MacOSX software. Its
purpose is to provide the most powerful and efficient (yet easy to use),
sample, stream, MOD music, and recording functions. All in a tiny DLL,
under 100KB in size.

Main Features
=============
* Samples
  supports WAV/AIFF/MP3/MP2/MP1/OGG and custom generated samples
* Sample streams
  stream any sample data in 8/16/32 bit, with both "push" and "pull" systems
* File streams
  MP3/MP2/MP1/OGG/WAV/AIFF file streaming
* Internet file streaming
  stream files from the internet, including Shout/Icecast
* User file streaming
  stream files from anywhere using any delivery method
* Multi-channel streaming
  support for more than plain stereo, including multi-channel OGG/WAV/AIFF files
* MOD music
  uses the same engine as XMPlay = best accuracy, speed, and quality
* MO3 music
  MP3/OGG compressed MOD music
* Add-on system
  support for more formats is available via add-ons (aka plugins)
* Multiple outputs
  simultaneously use multiple soundcards, and move channels between them
* Recording
  flexible recording system, with support for multiple devices
* Decode without playback
  streams and MOD musics can be outputted in any way you want
* speaker assignment
  assign streams and MOD musics to specific speakers
* High precision synchronization
  synchronize events in your software to the streams and MOD musics
* DirectX 8 effects
  chorus/compressor/distortion/echo/flanger/gargle/parameq/reverb
* User defined DSP functions
  custom effects may be applied to musics and streams
* 32 bit floating-point decoding and processing
  floating-point stream/music decoding, DSP, FX, and recording
* 3D sound
  play samples/streams/musics in any 3D position, with EAX support
'''

import os, sys, ctypes, platform
from .paths import x86_path, x64_path
from . import libloader

bass_module = libloader.load_library('bass', x86_path=x86_path, x64_path=x64_path, mode=ctypes.RTLD_GLOBAL)
func_type = libloader.get_functype()


QWORD = ctypes.c_int64

def LOBYTE(a): return (ctypes.c_byte)(a)
def HIBYTE(a): return (ctypes.c_byte)((a)>>8)
def LOWORD(a): return (ctypes.c_ushort)(a)
def HIWORD(a): return (ctypes.c_ushort)((a)>>16)
def MAKEWORD(a,b): return (ctypes.c_ushort)(((a)&0xff)|((b)<<8))
def MAKELONG(a,b): return (ctypes.c_ulong)(((a)&0xffff)|((b)<<16))

BASSVERSION = 0x204
BASSVERSIONTEXT = '2.4'

HMUSIC = ctypes.c_ulong		# MOD music handle
HSAMPLE = ctypes.c_ulong	# sample handle
HCHANNEL = ctypes.c_ulong	# playing sample's channel handle
HSTREAM = ctypes.c_ulong	# sample stream handle
HRECORD = ctypes.c_ulong	# recording handle
HSYNC = ctypes.c_ulong		# synchronizer handle
HDSP = ctypes.c_ulong		# DSP handle
HFX = ctypes.c_ulong		# DX8 effect handle
HPLUGIN = ctypes.c_ulong	# Plugin handle

# Error codes returned by BASS_ErrorGetCode
error_descriptions = {}
BASS_OK = 0 
error_descriptions[BASS_OK] = 'all is OK'
BASS_ERROR_MEM = 1
error_descriptions[BASS_ERROR_MEM] = 'memory error'
BASS_ERROR_FILEOPEN = 2
error_descriptions[BASS_ERROR_FILEOPEN] = "can't open the file"
BASS_ERROR_DRIVER = 3
error_descriptions[BASS_ERROR_DRIVER] = "can't find a free/valid driver"
BASS_ERROR_BUFLOST = 4
error_descriptions[BASS_ERROR_BUFLOST] = 'the sample buffer was lost'
BASS_ERROR_HANDLE = 5
error_descriptions[BASS_ERROR_HANDLE] = 'invalid handle'
BASS_ERROR_FORMAT = 6
error_descriptions[BASS_ERROR_FORMAT] = 'unsupported sample format'
BASS_ERROR_POSITION = 7
error_descriptions[BASS_ERROR_POSITION] = 'invalid position'
BASS_ERROR_INIT = 8
error_descriptions[BASS_ERROR_INIT] = 'BASS_Init has not been successfully called'
BASS_ERROR_START = 9
error_descriptions[BASS_ERROR_START] = 'BASS_Start has not been successfully called'
BASS_ERROR_ALREADY = 14
error_descriptions[BASS_ERROR_ALREADY] = 'already initialized/paused/whatever'
BASS_ERROR_NOCHAN = 18
error_descriptions[BASS_ERROR_NOCHAN] = "can't get a free channel"
BASS_ERROR_ILLTYPE = 19
error_descriptions[BASS_ERROR_ILLTYPE] = 'an illegal type was specified'
BASS_ERROR_ILLPARAM = 20
error_descriptions[BASS_ERROR_ILLPARAM] = 'an illegal parameter was specified'
BASS_ERROR_NO3D = 21
error_descriptions[BASS_ERROR_NO3D] = 'no 3D support'
BASS_ERROR_NOEAX = 22
error_descriptions[BASS_ERROR_NOEAX] = 'no EAX support'
BASS_ERROR_DEVICE = 23
error_descriptions[BASS_ERROR_DEVICE] = 'illegal device number'
BASS_ERROR_NOPLAY = 24
error_descriptions[BASS_ERROR_NOPLAY] = 'not playing'
BASS_ERROR_FREQ = 25
error_descriptions[BASS_ERROR_FREQ] = 'illegal sample rate'
BASS_ERROR_NOTFILE = 27
error_descriptions[BASS_ERROR_NOTFILE] = 'the stream is not a file stream'
BASS_ERROR_NOHW = 29
error_descriptions[BASS_ERROR_NOHW] = 'no hardware voices available'
BASS_ERROR_EMPTY = 31
error_descriptions[BASS_ERROR_EMPTY] = 'the MOD music has no sequence data'
BASS_ERROR_NONET = 32
error_descriptions[BASS_ERROR_NONET] = 'no internet connection could be opened'
BASS_ERROR_CREATE = 33
error_descriptions[BASS_ERROR_CREATE] = "couldn't create the file"
BASS_ERROR_NOFX = 34
error_descriptions[BASS_ERROR_NOFX] = 'effects are not available'
BASS_ERROR_NOTAVAIL = 37
error_descriptions[BASS_ERROR_NOTAVAIL] = 'requested data is not available'
BASS_ERROR_DECODE = 38
error_descriptions[BASS_ERROR_DECODE] = 'the channel is a "decoding channel"'
BASS_ERROR_DX = 39
error_descriptions[BASS_ERROR_DX] = 'a sufficient DirectX version is not installed'
BASS_ERROR_TIMEOUT = 40
error_descriptions[BASS_ERROR_TIMEOUT] = 'connection timedout'
BASS_ERROR_FILEFORM = 41
error_descriptions[BASS_ERROR_FILEFORM] = 'unsupported file format'
BASS_ERROR_SPEAKER = 42
error_descriptions[BASS_ERROR_SPEAKER] = 'unavailable speaker'
BASS_ERROR_VERSION = 43
error_descriptions[BASS_ERROR_VERSION] = 'invalid BASS version (used by add-ons)'
BASS_ERROR_CODEC = 44
error_descriptions[BASS_ERROR_CODEC] = 'codec is not available/supported'
BASS_ERROR_ENDED = 45
error_descriptions[BASS_ERROR_ENDED] = 'the channel/file has ended'
BASS_ERROR_UNKNOWN = -1
error_descriptions[BASS_ERROR_UNKNOWN] = 'some other mystery problem'

def get_error_description(error_code = -1):
	return error_descriptions.get(error_code, 'unknown BASS error code ' + str(error_code))

# BASS_SetConfig options
BASS_CONFIG_BUFFER = 0
BASS_CONFIG_UPDATEPERIOD = 1
BASS_CONFIG_GVOL_SAMPLE = 4
BASS_CONFIG_GVOL_STREAM = 5
BASS_CONFIG_GVOL_MUSIC = 6
BASS_CONFIG_CURVE_VOL = 7
BASS_CONFIG_CURVE_PAN = 8
BASS_CONFIG_FLOATDSP = 9
BASS_CONFIG_3DALGORITHM = 10
BASS_CONFIG_NET_TIMEOUT = 11
BASS_CONFIG_NET_BUFFER = 12
BASS_CONFIG_PAUSE_NOPLAY = 13
BASS_CONFIG_NET_PREBUF = 15
BASS_CONFIG_NET_PASSIVE = 18
BASS_CONFIG_REC_BUFFER = 19
BASS_CONFIG_NET_PLAYLIST = 21
BASS_CONFIG_MUSIC_VIRTUAL = 22
BASS_CONFIG_VERIFY = 23
BASS_CONFIG_UPDATETHREADS = 24
BASS_CONFIG_NET_READTIMEOUT = 37
BASS_CONFIG_SRC = 43
BASS_CONFIG_SRC_SAMPLE = 44
BASS_CONFIG_HANDLES = 41
BASS_CONFIG_UNICODE = 42
BASS_CONFIG_VISTA_SPEAKERS = 38
BASS_CONFIG_IOS_SPEAKER = 39
BASS_CONFIG_MF_DISABLE = 40
BASS_CONFIG_VERIFY_NET = 52
BASS_CONFIG_DEV_PERIOD = 53
BASS_CONFIG_AM_DISABLE = 58
BASS_CONFIG_NET_PLAYLIST_DEPTH = 59
BASS_CONFIG_NET_PREBUF_WAIT = 60
BASS_CONFIG_WASAPI_PERSIST = 65

#if defined(__linux__) || defined (_WIN32_WCE)
BASS_CONFIG_DEV_BUFFER = 27
#endif

# BASS_SetConfigPtr options
BASS_CONFIG_NET_AGENT = 16
BASS_CONFIG_NET_PROXY = 17
BASS_CONFIG_DEV_DEFAULT = 36

# Initialization flags
BASS_DEVICE_8BITS = 1# use 8 bit resolution, else 16 bit
BASS_DEVICE_MONO = 2# use mono, else stereo
BASS_DEVICE_3D = 4# enable 3D functionality
BASS_DEVICE_LATENCY = 256# calculate device latency (BASS_INFO struct)
BASS_DEVICE_CPSPEAKERS = 1024# detect speakers via Windows control panel
BASS_DEVICE_SPEAKERS = 2048# force enabling of speaker assignment
BASS_DEVICE_NOSPEAKER = 4096# ignore speaker arrangement
#ifdef __linux__
BASS_DEVICE_DMIX = 8192# use "dmix" (shared) output

# DirectSound interfaces (for use with BASS_GetDSoundObject)
BASS_OBJECT_DS = 1# IDirectSound
BASS_OBJECT_DS3DL = 2# IDirectSound3DListener

# Device info structure
class BASS_DEVICEINFO(ctypes.Structure):
	_fields_ = [('name', ctypes.c_char_p),#description
				('driver', ctypes.c_char_p),#driver
				('flags', ctypes.c_ulong)
				]
if platform.system().lower() == 'windows':
	if sys.getwindowsversion()[3] == 3:#VER_PLATFORM_WIN32_CE
		BASS_DEVICEINFO._fields_ = [('name', ctypes.c_wchar_p),#description
					('driver', ctypes.c_wchar_p),#driver
					('flags', ctypes.c_ulong)
					]

# BASS_DEVICEINFO flags
BASS_DEVICE_ENABLED = 1
BASS_DEVICE_DEFAULT = 2
BASS_DEVICE_INIT = 4

class BASS_INFO(ctypes.Structure):
	_fields_ = [('flags', ctypes.c_ulong),#device capabilities (DSCAPS_xxx flags)
				('hwsize', ctypes.c_ulong),#size of total device hardware memory
				('hwfree', ctypes.c_ulong),#size of free device hardware memory
				('freesam', ctypes.c_ulong),#number of free sample slots in the hardware
				('free3d', ctypes.c_ulong),#number of free 3D sample slots in the hardware
				('minrate', ctypes.c_ulong),#min sample rate supported by the hardware
				('maxrate', ctypes.c_ulong),#max sample rate supported by the hardware
				('eax', ctypes.c_byte),#device supports EAX? (always FALSE if BASS_DEVICE_3D was not used)
				('minbuf', ctypes.c_ulong),#recommended minimum buffer length in ms (requires BASS_DEVICE_LATENCY)
				('dsver', ctypes.c_ulong),#DirectSound version
				('latency', ctypes.c_ulong),#delay (in ms) before start of playback (requires BASS_DEVICE_LATENCY)
				('initflags', ctypes.c_ulong),#BASS_Init "flags" parameter
				('speakers', ctypes.c_ulong),#number of speakers available
				('freq', ctypes.c_ulong)#current output rate (Vista/OSX only)
				]

# BASS_INFO flags (from DSOUND.H)
DSCAPS_CONTINUOUSRATE = 0x00000010# supports all sample rates between min/maxrate
DSCAPS_EMULDRIVER = 0x00000020# device does NOT have hardware DirectSound support
DSCAPS_CERTIFIED = 0x00000040# device driver has been certified by Microsoft
DSCAPS_SECONDARYMONO = 0x00000100# mono
DSCAPS_SECONDARYSTEREO = 0x00000200# stereo
DSCAPS_SECONDARY8BIT = 0x00000400# 8 bit
DSCAPS_SECONDARY16BIT = 0x00000800# 16 bit

# Recording device info structure
class BASS_RECORDINFO(ctypes.Structure):
	_fields_ = [('flags', ctypes.c_ulong),#DWORD flags;// device capabilities (DSCCAPS_xxx flags)
				('formats', ctypes.c_ulong),#DWORD formats;// supported standard formats (WAVE_FORMAT_xxx flags)
				('inputs', ctypes.c_ulong),#DWORD inputs;	// number of inputs
				('singlein', ctypes.c_ubyte),#BOOL singlein;// TRUE = only 1 input can be set at a time
				('freq', ctypes.c_ulong)#DWORD freq;	// current input rate (Vista/OSX only)
				]

# BASS_RECORDINFO flags (from DSOUND.H)
DSCCAPS_EMULDRIVER = DSCAPS_EMULDRIVER# device does NOT have hardware DirectSound recording support
DSCCAPS_CERTIFIED = DSCAPS_CERTIFIED# device driver has been certified by Microsoft

# defines for formats field of BASS_RECORDINFO (from MMSYSTEM.H)
WAVE_FORMAT_1M08 = 0x00000001# 11.025 kHz, Mono,   8-bit
WAVE_FORMAT_1S08 = 0x00000002# 11.025 kHz, Stereo, 8-bit
WAVE_FORMAT_1M16 = 0x00000004# 11.025 kHz, Mono,   16-bit
WAVE_FORMAT_1S16 = 0x00000008# 11.025 kHz, Stereo, 16-bit
WAVE_FORMAT_2M08 = 0x00000010# 22.05  kHz, Mono,   8-bit
WAVE_FORMAT_2S08 = 0x00000020# 22.05  kHz, Stereo, 8-bit
WAVE_FORMAT_2M16 = 0x00000040# 22.05  kHz, Mono,   16-bit
WAVE_FORMAT_2S16 = 0x00000080# 22.05  kHz, Stereo, 16-bit
WAVE_FORMAT_4M08 = 0x00000100# 44.1   kHz, Mono,   8-bit
WAVE_FORMAT_4S08 = 0x00000200# 44.1   kHz, Stereo, 8-bit
WAVE_FORMAT_4M16 = 0x00000400# 44.1   kHz, Mono,   16-bit
WAVE_FORMAT_4S16 = 0x00000800# 44.1   kHz, Stereo, 16-bit

# Sample info structure
class BASS_SAMPLE(ctypes.Structure):
	_fields_ = [('freq', ctypes.c_ulong),#DWORD freq;// default playback rate
				('volume', ctypes.c_float),#float volume;// default volume (0-1)
				('pan', ctypes.c_float),#float pan;// default pan (-1=left, 0=middle, 1=right)
				('flags', ctypes.c_ulong),#DWORD flags;// BASS_SAMPLE_xxx flags
				('length', ctypes.c_ulong),#DWORD length;// length (in bytes)
				('max', ctypes.c_ulong),#DWORD max;// maximum simultaneous playbacks
				('origres', ctypes.c_ulong),#DWORD origres;// original resolution bits
				('chans', ctypes.c_ulong),#DWORD chans;// number of channels
				('mingap', ctypes.c_ulong),#DWORD mingap;	// minimum gap (ms) between creating channels
				('mode3d', ctypes.c_ulong),#DWORD mode3d;// BASS_3DMODE_xxx mode
				('mindist', ctypes.c_float),#float mindist;// minimum distance
				('maxdist', ctypes.c_float),#float maxdist;// maximum distance
				('iangle', ctypes.c_ulong),#DWORD iangle;// angle of inside projection cone
				('oangle', ctypes.c_ulong),#DWORD oangle;// angle of outside projection cone
				('outvol', ctypes.c_float),#float outvol;// delta-volume outside the projection cone
				('vam', ctypes.c_ulong),#DWORD vam;// voice allocation/management flags (BASS_VAM_xxx)
				('priority', ctypes.c_ulong)#DWORD priority;// priority (0=lowest, 0xffffffff=highest)
				]

BASS_SAMPLE_8BITS = 1# 8 bit
BASS_SAMPLE_FLOAT = 256# 32-bit floating-point
BASS_SAMPLE_MONO = 2# mono
BASS_SAMPLE_LOOP = 4# looped
BASS_SAMPLE_3D = 8# 3D functionality
BASS_SAMPLE_SOFTWARE = 16# not using hardware mixing
BASS_SAMPLE_MUTEMAX = 32# mute at max distance (3D only)
BASS_SAMPLE_VAM = 64# DX7 voice allocation & management
BASS_SAMPLE_FX = 128# old implementation of DX8 effects
BASS_SAMPLE_OVER_VOL = 0x10000# override lowest volume
BASS_SAMPLE_OVER_POS = 0x20000# override longest playing
BASS_SAMPLE_OVER_DIST = 0x30000# override furthest from listener (3D only)
BASS_STREAM_PRESCAN = 0x20000# enable pin-point seeking/length (MP3/MP2/MP1)
BASS_MP3_SETPOS = BASS_STREAM_PRESCAN
BASS_STREAM_AUTOFREE = 0x40000# automatically free the stream when it stop/ends
BASS_STREAM_RESTRATE = 0x80000# restrict the download rate of internet file streams
BASS_STREAM_BLOCK = 0x100000# download/play internet file stream in small blocks
BASS_STREAM_DECODE = 0x200000# don't play the stream, only decode (BASS_ChannelGetData)
BASS_STREAM_STATUS = 0x800000# give server status info (HTTP/ICY tags) in DOWNLOADPROC

BASS_MUSIC_FLOAT = BASS_SAMPLE_FLOAT
BASS_MUSIC_MONO = BASS_SAMPLE_MONO
BASS_MUSIC_LOOP = BASS_SAMPLE_LOOP
BASS_MUSIC_3D = BASS_SAMPLE_3D
BASS_MUSIC_FX = BASS_SAMPLE_FX
BASS_MUSIC_AUTOFREE = BASS_STREAM_AUTOFREE
BASS_MUSIC_DECODE = BASS_STREAM_DECODE
BASS_MUSIC_PRESCAN = BASS_STREAM_PRESCAN# calculate playback length
BASS_MUSIC_CALCLEN = BASS_MUSIC_PRESCAN
BASS_MUSIC_RAMP = 0x200# normal ramping
BASS_MUSIC_RAMPS = 0x400# sensitive ramping
BASS_MUSIC_SURROUND = 0x800# surround sound
BASS_MUSIC_SURROUND2 = 0x1000# surround sound (mode 2)
BASS_MUSIC_FT2MOD = 0x2000# play .MOD as FastTracker 2 does
BASS_MUSIC_PT1MOD = 0x4000# play .MOD as ProTracker 1 does
BASS_MUSIC_NONINTER = 0x10000# non-interpolated sample mixing
BASS_MUSIC_SINCINTER = 0x800000# sinc interpolated sample mixing
BASS_MUSIC_POSRESET = 0x8000# stop all notes when moving position
BASS_MUSIC_POSRESETEX = 0x400000# stop all notes and reset bmp/etc when moving position
BASS_MUSIC_STOPBACK = 0x80000# stop the music on a backwards jump effect
BASS_MUSIC_NOSAMPLE = 0x100000# don't load the samples

# speaker assignment flags
BASS_SPEAKER_FRONT = 0x1000000# front speakers
BASS_SPEAKER_REAR = 0x2000000# rear/side speakers
BASS_SPEAKER_CENLFE = 0x3000000# center & LFE speakers (5.1)
BASS_SPEAKER_REAR2 = 0x4000000# rear center speakers (7.1)
def BASS_SPEAKER_N(n): return ((n)<<24)# n'th pair of speakers (max 15)
BASS_SPEAKER_LEFT = 0x10000000# modifier: left
BASS_SPEAKER_RIGHT = 0x20000000# modifier: right
BASS_SPEAKER_FRONTLEFT = BASS_SPEAKER_FRONT|BASS_SPEAKER_LEFT
BASS_SPEAKER_FRONTRIGHT = BASS_SPEAKER_FRONT|BASS_SPEAKER_RIGHT
BASS_SPEAKER_REARLEFT = BASS_SPEAKER_REAR|BASS_SPEAKER_LEFT
BASS_SPEAKER_REARRIGHT = BASS_SPEAKER_REAR|BASS_SPEAKER_RIGHT
BASS_SPEAKER_CENTER = BASS_SPEAKER_CENLFE|BASS_SPEAKER_LEFT
BASS_SPEAKER_LFE = BASS_SPEAKER_CENLFE|BASS_SPEAKER_RIGHT
BASS_SPEAKER_REAR2LEFT = BASS_SPEAKER_REAR2|BASS_SPEAKER_LEFT
BASS_SPEAKER_REAR2RIGHT = BASS_SPEAKER_REAR2|BASS_SPEAKER_RIGHT
BASS_UNICODE = (-2147483648)
BASS_RECORD_PAUSE = 0x8000# start recording paused

# DX7 voice allocation & management flags
BASS_VAM_HARDWARE = 1
BASS_VAM_SOFTWARE = 2
BASS_VAM_TERM_TIME = 4
BASS_VAM_TERM_DIST = 8
BASS_VAM_TERM_PRIO = 16

# Channel info structure
class BASS_CHANNELINFO(ctypes.Structure):
	_fields_ = [('freq', ctypes.c_ulong),#DWORD freq;// default playback rate
				('chans', ctypes.c_ulong),#DWORD chans;// channels
				('flags', ctypes.c_ulong),#DWORD flags;// BASS_SAMPLE/STREAM/MUSIC/SPEAKER flags
				('ctype', ctypes.c_ulong),#DWORD ctype;// type of channel
				('origres', ctypes.c_ulong),#DWORD origres;// original resolution
				('plugin', HPLUGIN),#HPLUGIN plugin;// plugin
				('sample', HSAMPLE),#HSAMPLE sample;// sample
				('filename', ctypes.c_char_p)#const char *filename;// filename
				]

BASS_CTYPE_SAMPLE = 1
BASS_CTYPE_RECORD = 2
BASS_CTYPE_STREAM = 0x10000
BASS_CTYPE_STREAM_OGG = 0x10002
BASS_CTYPE_STREAM_MP1 = 0x10003
BASS_CTYPE_STREAM_MP2 = 0x10004
BASS_CTYPE_STREAM_MP3 = 0x10005
BASS_CTYPE_STREAM_AIFF = 0x10006
BASS_CTYPE_STREAM_CA = 0x10007
BASS_CTYPE_STREAM_WAV = 0x40000# WAVE flag, LOWORD=codec
BASS_CTYPE_STREAM_WAV_PCM = 0x50001
BASS_CTYPE_STREAM_WAV_FLOAT = 0x50003
BASS_CTYPE_MUSIC_MOD = 0x20000
BASS_CTYPE_MUSIC_MTM = 0x20001
BASS_CTYPE_MUSIC_S3M = 0x20002
BASS_CTYPE_MUSIC_XM = 0x20003
BASS_CTYPE_MUSIC_IT = 0x20004
BASS_CTYPE_MUSIC_MO3 = 0x00100# MO3 flag

class BASS_PLUGINFORM(ctypes.Structure):
	_fields_ = [('ctype', ctypes.c_ulong),#DWORD ctype;		// channel type
				('name', ctypes.c_char_p),#const char *name;	// format description
				('exts', ctypes.c_char_p)#const char *exts;	// file extension filter (*.ext1;*.ext2;etc...)
				]
if platform.system().lower() == 'windows':
	if sys.getwindowsversion()[3] == 3:#VER_PLATFORM_WIN32_CE
		BASS_PLUGINFORM._fields_ = [('ctype', ctypes.c_ulong),#DWORD ctype;		// channel type
					('name', ctypes.c_wchar_p),#const wchar_t *name;	// format description
					('exts', ctypes.c_wchar_p)#const wchar_t *exts;	// file extension filter (*.ext1;*.ext2;etc...)
					]

class BASS_PLUGININFO(ctypes.Structure):
	_fields_ = [('version', ctypes.c_ulong),#DWORD version;// version (same form as BASS_GetVersion)
				('formatc', ctypes.c_ulong),#DWORD formatc;// number of formats
				('formats', ctypes.POINTER(BASS_PLUGINFORM))#const BASS_PLUGINFORM *formats;// the array of formats
				]

# 3D vector (for 3D positions/velocities/orientations)
class BASS_3DVECTOR(ctypes.Structure):
	_fields_ = [('x', ctypes.c_float),#float x;// +=right, -=left
				('y', ctypes.c_float),#float y;// +=up, -=down
				('z', ctypes.c_float)#float z;// +=front, -=behind
				]

# 3D channel modes
BASS_3DMODE_NORMAL = 0
BASS_3DMODE_RELATIVE = 1
BASS_3DMODE_OFF = 2

# software 3D mixing algorithms (used with BASS_CONFIG_3DALGORITHM)
BASS_3DALG_DEFAULT = 0
BASS_3DALG_OFF = 1
BASS_3DALG_FULL = 2
BASS_3DALG_LIGHT = 3

# EAX environments, use with BASS_SetEAXParameters
EAX_ENVIRONMENT_GENERIC = 0
EAX_ENVIRONMENT_PADDEDCELL = 1
EAX_ENVIRONMENT_ROOM = 2
EAX_ENVIRONMENT_BATHROOM = 3
EAX_ENVIRONMENT_LIVINGROOM = 4
EAX_ENVIRONMENT_STONEROOM = 5
EAX_ENVIRONMENT_AUDITORIUM = 6
EAX_ENVIRONMENT_CONCERTHALL = 7
EAX_ENVIRONMENT_CAVE = 8
EAX_ENVIRONMENT_ARENA = 9
EAX_ENVIRONMENT_HANGAR = 10
EAX_ENVIRONMENT_CARPETEDHALLWAY = 11
EAX_ENVIRONMENT_HALLWAY = 12
EAX_ENVIRONMENT_STONECORRIDOR = 13
EAX_ENVIRONMENT_ALLEY = 14
EAX_ENVIRONMENT_FOREST = 15
EAX_ENVIRONMENT_CITY = 16
EAX_ENVIRONMENT_MOUNTAINS = 17
EAX_ENVIRONMENT_QUARRY = 18
EAX_ENVIRONMENT_PLAIN = 19
EAX_ENVIRONMENT_PARKINGLOT = 20
EAX_ENVIRONMENT_SEWERPIPE = 21
EAX_ENVIRONMENT_UNDERWATER = 22
EAX_ENVIRONMENT_DRUGGED = 23
EAX_ENVIRONMENT_DIZZY = 24
EAX_ENVIRONMENT_PSYCHOTIC = 25
EAX_ENVIRONMENT_COUNT = 26# total number of environments

# EAX presets, usage: BASS_SetEAXParameters(EAX_PRESET_xxx)
EAX_PRESET_GENERIC         =(EAX_ENVIRONMENT_GENERIC,0.5,1.493,0.5)
EAX_PRESET_PADDEDCELL      =(EAX_ENVIRONMENT_PADDEDCELL,0.25,0.1,0.0)
EAX_PRESET_ROOM            =(EAX_ENVIRONMENT_ROOM,0.417,0.4,0.666)
EAX_PRESET_BATHROOM        =(EAX_ENVIRONMENT_BATHROOM,0.653,1.499,0.166)
EAX_PRESET_LIVINGROOM      =(EAX_ENVIRONMENT_LIVINGROOM,0.208,0.478,0.0)
EAX_PRESET_STONEROOM       =(EAX_ENVIRONMENT_STONEROOM,0.5,2.309,0.888)
EAX_PRESET_AUDITORIUM      =(EAX_ENVIRONMENT_AUDITORIUM,0.403,4.279,0.5)
EAX_PRESET_CONCERTHALL     =(EAX_ENVIRONMENT_CONCERTHALL,0.5,3.961,0.5)
EAX_PRESET_CAVE            =(EAX_ENVIRONMENT_CAVE,0.5,2.886,1.304)
EAX_PRESET_ARENA           =(EAX_ENVIRONMENT_ARENA,0.361,7.284,0.332)
EAX_PRESET_HANGAR          =(EAX_ENVIRONMENT_HANGAR,0.5,10.0,0.3)
EAX_PRESET_CARPETEDHALLWAY =(EAX_ENVIRONMENT_CARPETEDHALLWAY,0.153,0.259,2.0)
EAX_PRESET_HALLWAY         =(EAX_ENVIRONMENT_HALLWAY,0.361,1.493,0.0)
EAX_PRESET_STONECORRIDOR   =(EAX_ENVIRONMENT_STONECORRIDOR,0.444,2.697,0.638)
EAX_PRESET_ALLEY           =(EAX_ENVIRONMENT_ALLEY,0.25,1.752,0.776)
EAX_PRESET_FOREST          =(EAX_ENVIRONMENT_FOREST,0.111,3.145,0.472)
EAX_PRESET_CITY            =(EAX_ENVIRONMENT_CITY,0.111,2.767,0.224)
EAX_PRESET_MOUNTAINS       =(EAX_ENVIRONMENT_MOUNTAINS,0.194,7.841,0.472)
EAX_PRESET_QUARRY          =(EAX_ENVIRONMENT_QUARRY,1.0,1.499,0.5)
EAX_PRESET_PLAIN           =(EAX_ENVIRONMENT_PLAIN,0.097,2.767,0.224)
EAX_PRESET_PARKINGLOT      =(EAX_ENVIRONMENT_PARKINGLOT,0.208,1.652,1.5)
EAX_PRESET_SEWERPIPE       =(EAX_ENVIRONMENT_SEWERPIPE,0.652,2.886,0.25)
EAX_PRESET_UNDERWATER      =(EAX_ENVIRONMENT_UNDERWATER,1.0,1.499,0.0)
EAX_PRESET_DRUGGED         =(EAX_ENVIRONMENT_DRUGGED,0.875,8.392,1.388)
EAX_PRESET_DIZZY           =(EAX_ENVIRONMENT_DIZZY,0.139,17.234,0.666)
EAX_PRESET_PSYCHOTIC       =(EAX_ENVIRONMENT_PSYCHOTIC,0.486,7.563,0.806)

#typedef DWORD (CALLBACK STREAMPROC)(HSTREAM handle, void *buffer, DWORD length, void *user);
STREAMPROC = func_type(ctypes.c_ulong, HSTREAM, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p) 
# User stream callback function. NOTE: A stream function should obviously be as quick
#as possible, other streams (and MOD musics) can't be mixed until it's finished.
#handle : The stream that needs writing
#buffer : Buffer to write the samples in
#length : Number of bytes to write
#user   : The 'user' parameter value given when calling BASS_StreamCreate
#RETURN : Number of bytes written. Set the BASS_STREAMPROC_END flag to end the stream.

#~ def user_stream_callback_function(handle, buffer, length, user):
	#~ b = ctypes.cast(buffer, ctypes.c_char_p)
	#~ ctypes.memset(b, 0, length)
	#~ data = ctypes.c_char_p(' ' * length)
	#~ ctypes.memmove(b, data, length)
	#~ if your_custom_function_is_eof() or your_custom_flag_is_eof:
		#~ length |= BASS_STREAMPROC_END
	#~ return length
#~ user_func = STREAMPROC(user_stream_callback_function)

BASS_STREAMPROC_END = (-2147483648)# end of user stream flag

# special STREAMPROCs
def streamproc_dummy(handle, buffer, length, user): return 0
streamproc_push = -1
STREAMPROC_DUMMY = STREAMPROC(streamproc_dummy)# "dummy" stream
STREAMPROC_PUSH = STREAMPROC(streamproc_push)# push stream

# BASS_StreamCreateFileUser file systems
STREAMFILE_NOBUFFER = 0
STREAMFILE_BUFFER = 1
STREAMFILE_BUFFERPUSH = 2

# User file stream callback functions
#typedef void (CALLBACK FILECLOSEPROC)(void *user);
FILECLOSEPROC = func_type(None, ctypes.c_void_p) 
#typedef QWORD (CALLBACK FILELENPROC)(void *user);
FILELENPROC = func_type(QWORD, ctypes.c_void_p) 
#typedef DWORD (CALLBACK FILEREADPROC)(void *buffer, DWORD length, void *user);
FILEREADPROC = func_type(ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p) 
#typedef BOOL (CALLBACK FILESEEKPROC)(QWORD offset, void *user);
FILESEEKPROC = func_type(ctypes.c_byte, QWORD, ctypes.c_void_p) 

class BASS_FILEPROCS(ctypes.Structure):
	_fields_ = [('close', FILECLOSEPROC),#FILECLOSEPROC *close;
				('length', FILELENPROC),#FILELENPROC *length;
				('read', FILEREADPROC),#FILEREADPROC *read;
				('seek', FILESEEKPROC)#FILESEEKPROC *seek;
				]

# BASS_StreamPutFileData options
BASS_FILEDATA_END = 0

# BASS_StreamGetFilePosition modes
BASS_FILEPOS_CURRENT = 0
BASS_FILEPOS_DECODE = BASS_FILEPOS_CURRENT
BASS_FILEPOS_DOWNLOAD = 1
BASS_FILEPOS_END = 2
BASS_FILEPOS_START = 3
BASS_FILEPOS_CONNECTED = 4
BASS_FILEPOS_BUFFER = 5
BASS_FILEPOS_SOCKET = 6

#typedef void (CALLBACK DOWNLOADPROC)(const void *buffer, DWORD length, void *user);
DOWNLOADPROC = func_type(ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p) 
# Internet stream download callback function.
#buffer : Buffer containing the downloaded data... NULL=end of download
#length : Number of bytes in the buffer
#user   : The 'user' parameter value given when calling BASS_StreamCreateURL

# BASS_ChannelSetSync types
BASS_SYNC_POS = 0
BASS_SYNC_END = 2
BASS_SYNC_META = 4
BASS_SYNC_SLIDE = 5
BASS_SYNC_STALL = 6
BASS_SYNC_DOWNLOAD = 7
BASS_SYNC_FREE = 8
BASS_SYNC_SETPOS = 11
BASS_SYNC_MUSICPOS = 10
BASS_SYNC_MUSICINST = 1
BASS_SYNC_MUSICFX = 3
BASS_SYNC_OGG_CHANGE = 12
BASS_SYNC_MIXTIME = 0x40000000
BASS_SYNC_ONETIME = (-2147483648)

#typedef void (CALLBACK SYNCPROC)(HSYNC handle, DWORD channel, DWORD data, void *user);
SYNCPROC = func_type(ctypes.c_void_p, HSYNC, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p) 
# Sync callback function. NOTE: a sync callback function should be very
#quick as other syncs can't be processed until it has finished. If the sync
#is a "mixtime" sync, then other streams and MOD musics can't be mixed until
#it's finished either.
#handle : The sync that has occured
#channel: Channel that the sync occured in
#data   : Additional data associated with the sync's occurance
#user   : The 'user' parameter given when calling BASS_ChannelSetSync

#typedef void (CALLBACK DSPPROC)(HDSP handle, DWORD channel, void *buffer, DWORD length, void *user);
DSPPROC = func_type(ctypes.c_void_p, HDSP, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p) 
# DSP callback function. NOTE: A DSP function should obviously be as quick as
#possible... other DSP functions, streams and MOD musics can not be processed
#until it's finished.
#handle : The DSP handle
#channel: Channel that the DSP is being applied to
#buffer : Buffer to apply the DSP to
#length : Number of bytes in the buffer
#user   : The 'user' parameter given when calling BASS_ChannelSetDSP

#typedef BOOL (CALLBACK RECORDPROC)(HRECORD handle, const void *buffer, DWORD length, void *user);
RECORDPROC = func_type(ctypes.c_byte, HRECORD, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_void_p) 
# Recording callback function.
#handle : The recording handle
#buffer : Buffer containing the recorded sample data
#length : Number of bytes
#user   : The 'user' parameter value given when calling BASS_RecordStart
#RETURN : TRUE = continue recording, FALSE = stop

# BASS_ChannelIsActive return values
BASS_ACTIVE_STOPPED = 0
BASS_ACTIVE_PLAYING = 1
BASS_ACTIVE_STALLED = 2
BASS_ACTIVE_PAUSED = 3

# Channel attributes
BASS_ATTRIB_FREQ = 1
BASS_ATTRIB_VOL = 2
BASS_ATTRIB_PAN = 3
BASS_ATTRIB_EAXMIX = 4
BASS_ATTRIB_MUSIC_AMPLIFY = 0x100
BASS_ATTRIB_MUSIC_PANSEP = 0x101
BASS_ATTRIB_MUSIC_PSCALER = 0x102
BASS_ATTRIB_MUSIC_BPM = 0x103
BASS_ATTRIB_MUSIC_SPEED = 0x104
BASS_ATTRIB_MUSIC_VOL_GLOBAL = 0x105
BASS_ATTRIB_MUSIC_VOL_CHAN = 0x200# + channel #
BASS_ATTRIB_MUSIC_VOL_INST = 0x300# + instrument #

# BASS_ChannelGetData flags
BASS_DATA_AVAILABLE = 0# query how much data is buffered
BASS_DATA_FLOAT = 0x40000000# flag: return floating-point sample data
BASS_DATA_FFT256 = (-2147483648)# 256 sample FFT
BASS_DATA_FFT512 = (-2147483647)# 512 FFT
BASS_DATA_FFT1024 = (-2147483646)# 1024 FFT
BASS_DATA_FFT2048 = (-2147483645)# 2048 FFT
BASS_DATA_FFT4096 = (-2147483644)# 4096 FFT
BASS_DATA_FFT8192 = (-2147483643)# 8192 FFT
BASS_DATA_FFT_INDIVIDUAL = 0x10# FFT flag: FFT for each channel, else all combined
BASS_DATA_FFT_NOWINDOW = 0x20# FFT flag: no Hanning window

# BASS_ChannelGetTags types : what's returned
BASS_TAG_ID3 = 0# ID3v1 tags : TAG_ID3 structure
BASS_TAG_ID3V2 = 1# ID3v2 tags : variable length block
BASS_TAG_OGG = 2# OGG comments : series of null-terminated UTF-8 strings
BASS_TAG_HTTP = 3# HTTP headers : series of null-terminated ANSI strings
BASS_TAG_ICY = 4# ICY headers : series of null-terminated ANSI strings
BASS_TAG_META = 5# ICY metadata : ANSI string
BASS_TAG_VENDOR = 9# OGG encoder : UTF-8 string
BASS_TAG_LYRICS3 = 10# Lyric3v2 tag : ASCII string
BASS_TAG_CA_CODEC = 11# CoreAudio codec info : TAG_CA_CODEC structure
BASS_TAG_RIFF_INFO = 0x100# RIFF "INFO" tags : series of null-terminated ANSI strings
BASS_TAG_RIFF_BEXT = 0x101# RIFF/BWF "bext" tags : TAG_BEXT structure
BASS_TAG_RIFF_CART = 0x102# RIFF/BWF "cart" tags : TAG_CART structure
BASS_TAG_MUSIC_NAME = 0x10000# MOD music name : ANSI string
BASS_TAG_MUSIC_MESSAGE = 0x10001# MOD message : ANSI string
BASS_TAG_MUSIC_ORDERS = 0x10002# MOD order list : BYTE array of pattern numbers
BASS_TAG_MUSIC_INST = 0x10100# + instrument #, MOD instrument name : ANSI string
BASS_TAG_MUSIC_SAMPLE = 0x10300# + sample #, MOD sample name : ANSI string

# ID3v1 tag structure
class TAG_ID3(ctypes.Structure):
	_fields_ = [('id', ctypes.c_char*3),#char id[3];
				('title', ctypes.c_char*30),#char title[30];
				('artist', ctypes.c_char*30),#char artist[30];
				('album', ctypes.c_char*30),#char album[30];
				('year', ctypes.c_char*4),#char year[4];
				('comment', ctypes.c_char*30),#char comment[30];
				('genre', ctypes.c_byte)#BYTE genre;
				]

# BWF "bext" tag structure
class TAG_BEXT(ctypes.Structure):
	_fields_ = [('Description', ctypes.c_char*256),#char Description[256];// description
				('Originator', ctypes.c_char*32),#char Originator[32];// name of the originator
				('OriginatorReference', ctypes.c_char*32),#char OriginatorReference[32];// reference of the originator
				('OriginationDate', ctypes.c_char*10),#char OriginationDate[10];// date of creation (yyyy-mm-dd)
				('OriginationTime', ctypes.c_char*8),#char OriginationTime[8];// time of creation (hh-mm-ss)
				('TimeReference', QWORD),#QWORD TimeReference;// first sample count since midnight (little-endian)
				('Version', ctypes.c_ushort),#WORD Version;// BWF version (little-endian)
				('UMID', ctypes.c_byte*64),#BYTE UMID[64];// SMPTE UMID
				('Reserved', ctypes.c_byte*190),#BYTE Reserved[190];
				('CodingHistory', ctypes.c_char_p)#char CodingHistory[];// history
				]

# BWF "cart" tag structures
class TAG_CART_TIMER(ctypes.Structure):
	_fields_ = [('dwUsage', ctypes.c_ulong),#DWORD dwUsage;// FOURCC timer usage ID
				('dwValue', ctypes.c_ulong)#DWORD dwValue;// timer value in samples from head
				]

class TAG_CART(ctypes.Structure):
	_fields_ = [('Version', ctypes.c_char*4),#char Version[4];// version of the data structure
				('Title', ctypes.c_char*64),#char Title[64];// title of cart audio sequence
				('Artist', ctypes.c_char*64),#char Artist[64];// artist or creator name
				('CutID', ctypes.c_char*64),#char CutID[64];// cut number identification
				('ClientID', ctypes.c_char*64),#char ClientID[64];// client identification
				('Category', ctypes.c_char*64),#char Category[64];// category ID, PSA, NEWS, etc
				('Classification', ctypes.c_char*64),#char Classification[64];// classification or auxiliary key
				('OutCue', ctypes.c_char*64),#char OutCue[64];// out cue text
				('StartDate', ctypes.c_char*10),#char StartDate[10];// yyyy-mm-dd
				('StartTime', ctypes.c_char*8),#char StartTime[8];// hh:mm:ss
				('EndDate', ctypes.c_char*10),#char EndDate[10];// yyyy-mm-dd
				('EndTime', ctypes.c_char*8),#char EndTime[8];// hh:mm:ss
				('ProducerAppID', ctypes.c_char*64),#char ProducerAppID[64];// name of vendor or application
				('ProducerAppVersion', ctypes.c_char*64),#char ProducerAppVersion[64];// version of producer application
				('UserDef', ctypes.c_char*64),#char UserDef[64];// user defined text
				('dwLevelReference', ctypes.c_ulong),#DWORD dwLevelReference;// sample value for 0 dB reference
				('PostTimer', TAG_CART_TIMER*8),#TAG_CART_TIMER PostTimer[8];// 8 time markers after head
				('Reserved', ctypes.c_char*276),#char Reserved[276];
				('URL', ctypes.c_char*1024),#char URL[1024];// uniform resource locator
				('TagText', ctypes.c_char_p)#char TagText[];// free form text for scripts or tags
				]

# CoreAudio codec info structure
class TAG_CA_CODEC(ctypes.Structure):
	_fields_ = [('ftype', ctypes.c_ulong),#DWORD ftype;// file format
				('atype', ctypes.c_ulong),#DWORD atype;// audio format
				('name', ctypes.c_char_p)#const char *name;// description
				]

# BASS_ChannelGetLength/GetPosition/SetPosition modes
BASS_POS_BYTE = 0# byte position
BASS_POS_MUSIC_ORDER = 1# order.row position, MAKELONG(order,row)
BASS_POS_DECODE = 0x10000000# flag: get the decoding (not playing) position

# BASS_RecordSetInput flags
BASS_INPUT_OFF = 0x10000
BASS_INPUT_ON = 0x20000
BASS_INPUT_TYPE_MASK = (-16777216)
BASS_INPUT_TYPE_UNDEF = 0x00000000
BASS_INPUT_TYPE_DIGITAL = 0x01000000
BASS_INPUT_TYPE_LINE = 0x02000000
BASS_INPUT_TYPE_MIC = 0x03000000
BASS_INPUT_TYPE_SYNTH = 0x04000000
BASS_INPUT_TYPE_CD = 0x05000000
BASS_INPUT_TYPE_PHONE = 0x06000000
BASS_INPUT_TYPE_SPEAKER = 0x07000000
BASS_INPUT_TYPE_WAVE = 0x08000000
BASS_INPUT_TYPE_AUX = 0x09000000
BASS_INPUT_TYPE_ANALOG = 0x0a000000

# DX8 effect types, use with BASS_ChannelSetFX
(
 BASS_FX_DX8_CHORUS,
 BASS_FX_DX8_COMPRESSOR,
 BASS_FX_DX8_DISTORTION,
 BASS_FX_DX8_ECHO,
 BASS_FX_DX8_FLANGER,
 BASS_FX_DX8_GARGLE,
 BASS_FX_DX8_I3DL2REVERB,
 BASS_FX_DX8_PARAMEQ,
 BASS_FX_DX8_REVERB,
 ) = range(9)


class BASS_DX8_CHORUS(ctypes.Structure):
	_fields_ = [('fWetDryMix', ctypes.c_float),#float       fWetDryMix;
				('fDepth', ctypes.c_float),#float       fDepth;
				('fFeedback', ctypes.c_float),#float       fFeedback;
				('fFrequency', ctypes.c_float),#float       fFrequency;
				('lWaveform', ctypes.c_ulong),#DWORD       lWaveform;// 0=triangle, 1=sine
				('fDelay', ctypes.c_float),#float       fDelay;
				('lPhase', ctypes.c_ulong)#DWORD       lPhase;// BASS_DX8_PHASE_xxx
				]

class BASS_DX8_COMPRESSOR(ctypes.Structure):
	_fields_ = [('fGain', ctypes.c_float),#float   fGain;
				('fAttack', ctypes.c_float),#float   fAttack;
				('fRelease', ctypes.c_float),#float   fRelease;
				('fThreshold', ctypes.c_float),#float   fThreshold;
				('fRatio', ctypes.c_float),#float   fRatio;
				('fPredelay', ctypes.c_float)#float   fPredelay;
				]

class BASS_DX8_DISTORTION(ctypes.Structure):
	_fields_ = [('fGain', ctypes.c_float),#float   fGain;
				('fEdge', ctypes.c_float),#float   fEdge;
				('fPostEQCenterFrequency', ctypes.c_float),#float   fPostEQCenterFrequency;
				('fPostEQBandwidth', ctypes.c_float),#float   fPostEQBandwidth;
				('fPreLowpassCutoff', ctypes.c_float)#float   fPreLowpassCutoff;
				]

class BASS_DX8_ECHO(ctypes.Structure):
	_fields_ = [('fWetDryMix', ctypes.c_float),#float   fWetDryMix;
				('fFeedback', ctypes.c_float),#float   fFeedback;
				('fLeftDelay', ctypes.c_float),#float   fLeftDelay;
				('fRightDelay', ctypes.c_float),#float   fRightDelay;
				('lPanDelay', ctypes.c_byte)#BOOL    lPanDelay;
				]

class BASS_DX8_FLANGER(ctypes.Structure):
	_fields_ = [('fWetDryMix', ctypes.c_float),#float       fWetDryMix;
				('fDepth', ctypes.c_float),#float       fDepth;
				('fFeedback', ctypes.c_float),#float       fFeedback;
				('fFrequency', ctypes.c_float),#float       fFrequency;
				('lWaveform', ctypes.c_ulong),#DWORD lWaveform;// 0=triangle, 1=sine
				('fDelay', ctypes.c_float),#float       fDelay;
				('lPhase', ctypes.c_ulong)#DWORD       lPhase;// BASS_DX8_PHASE_xxx
				]

class BASS_DX8_GARGLE(ctypes.Structure):
	_fields_ = [('dwRateHz', ctypes.c_ulong),#DWORD dwRateHz;// Rate of modulation in hz
				('dwWaveShape', ctypes.c_ulong)#DWORD dwWaveShape;// 0=triangle, 1=square
				]

class BASS_DX8_I3DL2REVERB(ctypes.Structure):
	_fields_ = [('lRoom', ctypes.c_int),#int    lRoom;// [-10000, 0]      default: -1000 mB
				('lRoomHF', ctypes.c_int),#int    lRoomHF;// [-10000, 0]      default: 0 mB
				('flRoomRolloffFactor', ctypes.c_float),#float  flRoomRolloffFactor;// [0.0, 10.0]      default: 0.0
				('flDecayTime', ctypes.c_float),#float  flDecayTime;// [0.1, 20.0]      default: 1.49s
				('flDecayHFRatio', ctypes.c_float),#float  flDecayHFRatio;// [0.1, 2.0]       default: 0.83
				('lReflections', ctypes.c_int),#int    lReflections;// [-10000, 1000]   default: -2602 mB
				('flReflectionsDelay', ctypes.c_float),#float  flReflectionsDelay;// [0.0, 0.3]       default: 0.007 s
				('lReverb', ctypes.c_int),#int    lReverb;// [-10000, 2000]   default: 200 mB
				('flReverbDelay', ctypes.c_float),#float  flReverbDelay;// [0.0, 0.1]       default: 0.011 s
				('flDiffusion', ctypes.c_float),#float  flDiffusion;// [0.0, 100.0]     default: 100.0 %
				('flDensity', ctypes.c_float),#float  flDensity;// [0.0, 100.0]     default: 100.0 %
				('flHFReference', ctypes.c_float)#float  flHFReference;// [20.0, 20000.0]  default: 5000.0 Hz
				]

class BASS_DX8_PARAMEQ(ctypes.Structure):
	_fields_ = [('fCenter', ctypes.c_float),#float   fCenter;
				('fBandwidth', ctypes.c_float),#float   fBandwidth;
				('fGain', ctypes.c_float)#float   fGain;
				]

class BASS_DX8_REVERB(ctypes.Structure):
	_fields_ = [('fInGain', ctypes.c_float),#float fInGain;// [-96.0,0.0]            default: 0.0 dB
				('fReverbMix', ctypes.c_float),#float fReverbMix;// [-96.0,0.0]         default: 0.0 db
				('fReverbTime', ctypes.c_float),#float fReverbTime;// [0.001,3000.0]     default: 1000.0 ms
				('fHighFreqRTRatio', ctypes.c_float)#float fHighFreqRTRatio;// [0.001,0.999] default: 0.001
				]

BASS_DX8_PHASE_NEG_180 = 0
BASS_DX8_PHASE_NEG_90 = 1
BASS_DX8_PHASE_ZERO = 2
BASS_DX8_PHASE_90 = 3
BASS_DX8_PHASE_180 = 4

#BOOL BASSDEF(BASS_SetConfig)(DWORD option, DWORD value);
BASS_SetConfig = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)(('BASS_SetConfig', bass_module))
#DWORD BASSDEF(BASS_GetConfig)(DWORD option);
BASS_GetConfig = func_type(ctypes.c_ulong, ctypes.c_ulong)(('BASS_GetConfig', bass_module))
#BOOL BASSDEF(BASS_SetConfigPtr)(DWORD option, void *value);
BASS_SetConfigPtr = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_void_p)(('BASS_SetConfigPtr', bass_module))
#void *BASSDEF(BASS_GetConfigPtr)(DWORD option);
BASS_GetConfigPtr = func_type(ctypes.c_void_p, ctypes.c_ulong)(('BASS_GetConfigPtr', bass_module))
#DWORD BASSDEF(BASS_GetVersion)();
BASS_GetVersion = func_type(ctypes.c_ulong)(('BASS_GetVersion', bass_module))
#int BASSDEF(BASS_ErrorGetCode)();
BASS_ErrorGetCode = func_type(ctypes.c_int)(('BASS_ErrorGetCode', bass_module))
#BOOL BASSDEF(BASS_GetDeviceInfo)(DWORD device, BASS_DEVICEINFO *info);
BASS_GetDeviceInfo = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(BASS_DEVICEINFO))(('BASS_GetDeviceInfo', bass_module))
#BOOL BASSDEF(BASS_Init)(int device, DWORD freq, DWORD flags, void *win, void *dsguid);
BASS_Init = func_type(ctypes.c_byte, ctypes.c_int, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_void_p)(('BASS_Init', bass_module))
#BOOL BASSDEF(BASS_SetDevice)(DWORD device);
BASS_SetDevice = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_SetDevice', bass_module))
#DWORD BASSDEF(BASS_GetDevice)();
BASS_GetDevice = func_type(ctypes.c_ulong)(('BASS_GetDevice', bass_module))
#BOOL BASSDEF(BASS_Free)();
BASS_Free = func_type(ctypes.c_byte)(('BASS_Free', bass_module))
#BOOL BASSDEF(BASS_GetInfo)(BASS_INFO *info);
BASS_GetInfo = func_type(ctypes.c_byte, ctypes.POINTER(BASS_INFO))(('BASS_GetInfo', bass_module))
#BOOL BASSDEF(BASS_Update)(DWORD length);
BASS_Update = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_Update', bass_module))
#float BASSDEF(BASS_GetCPU)();
BASS_GetCPU = func_type(ctypes.c_float)(('BASS_GetCPU', bass_module))
#BOOL BASSDEF(BASS_Start)();
BASS_Start = func_type(ctypes.c_byte)(('BASS_Start', bass_module))
#BOOL BASSDEF(BASS_Stop)();
BASS_Stop = func_type(ctypes.c_byte)(('BASS_Stop', bass_module))
#BOOL BASSDEF(BASS_Pause)();
BASS_Pause = func_type(ctypes.c_byte)(('BASS_Pause', bass_module))
#BOOL BASSDEF(BASS_SetVolume)(float volume);
BASS_SetVolume = func_type(ctypes.c_byte, ctypes.c_float)(('BASS_SetVolume', bass_module))
#float BASSDEF(BASS_GetVolume)();
BASS_GetVolume = func_type(ctypes.c_float)(('BASS_GetVolume', bass_module))

#HPLUGIN BASSDEF(BASS_PluginLoad)(const char *file, DWORD flags);
_BASS_PluginLoad = func_type(HPLUGIN, ctypes.c_char_p, ctypes.c_ulong)(('BASS_PluginLoad', bass_module))
def BASS_PluginLoad(file, flags):
    if type(file) != bytes:
        file = file.encode(sys.getfilesystemencoding())
    return _BASS_PluginLoad(file, flags)


#BOOL BASSDEF(BASS_PluginFree)(HPLUGIN handle);
BASS_PluginFree = func_type(ctypes.c_byte, HPLUGIN)(('BASS_PluginFree', bass_module))
#const BASS_PLUGININFO *BASSDEF(BASS_PluginGetInfo)(HPLUGIN handle);
BASS_PluginGetInfo = func_type(ctypes.POINTER(BASS_PLUGININFO), HPLUGIN)(('BASS_PluginGetInfo', bass_module))

#BOOL BASSDEF(BASS_Set3DFactors)(float distf, float rollf, float doppf);
BASS_Set3DFactors = func_type(ctypes.c_byte, ctypes.c_float, ctypes.c_float, ctypes.c_float)(('BASS_Set3DFactors', bass_module))
#BOOL BASSDEF(BASS_Get3DFactors)(float *distf, float *rollf, float *doppf);
BASS_Get3DFactors = func_type(ctypes.c_byte, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float))(('BASS_Get3DFactors', bass_module))
#BOOL BASSDEF(BASS_Set3DPosition)(const BASS_3DVECTOR *pos, const BASS_3DVECTOR *vel, const BASS_3DVECTOR *front, const BASS_3DVECTOR *top);
BASS_Set3DPosition = func_type(ctypes.c_byte, ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR))(('BASS_Set3DPosition', bass_module))
#BOOL BASSDEF(BASS_Get3DPosition)(BASS_3DVECTOR *pos, BASS_3DVECTOR *vel, BASS_3DVECTOR *front, BASS_3DVECTOR *top);
BASS_Get3DPosition = func_type(ctypes.c_byte, ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR))(('BASS_Get3DPosition', bass_module))
#void BASSDEF(BASS_Apply3D)();
BASS_Apply3D = func_type(None)(('BASS_Apply3D', bass_module))

#HMUSIC BASSDEF(BASS_MusicLoad)(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD flags, DWORD freq);
BASS_MusicLoad = func_type(HMUSIC, ctypes.c_byte, ctypes.c_void_p, QWORD, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_MusicLoad', bass_module))
#BOOL BASSDEF(BASS_MusicFree)(HMUSIC handle);
BASS_MusicFree = func_type(ctypes.c_byte, HMUSIC)(('BASS_MusicFree', bass_module))

#HSAMPLE BASSDEF(BASS_SampleLoad)(BOOL mem, const void *file, QWORD offset, DWORD length, DWORD max, DWORD flags);
BASS_SampleLoad = func_type(HSAMPLE, ctypes.c_byte, ctypes.c_void_p, QWORD, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_SampleLoad', bass_module))
#HSAMPLE BASSDEF(BASS_SampleCreate)(DWORD length, DWORD freq, DWORD chans, DWORD max, DWORD flags);
BASS_SampleCreate = func_type(HSAMPLE, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_SampleCreate', bass_module))
#BOOL BASSDEF(BASS_SampleFree)(HSAMPLE handle);
BASS_SampleFree = func_type(ctypes.c_byte, HSAMPLE)(('BASS_SampleFree', bass_module))
#BOOL BASSDEF(BASS_SampleSetData)(HSAMPLE handle, const void *buffer);
BASS_SampleSetData = func_type(ctypes.c_byte, HSAMPLE, ctypes.c_void_p)(('BASS_SampleSetData', bass_module))
#BOOL BASSDEF(BASS_SampleGetData)(HSAMPLE handle, void *buffer);
BASS_SampleGetData = func_type(ctypes.c_byte, HSAMPLE, ctypes.c_void_p)(('BASS_SampleGetData', bass_module))
#BOOL BASSDEF(BASS_SampleGetInfo)(HSAMPLE handle, BASS_SAMPLE *info);
BASS_SampleGetInfo = func_type(ctypes.c_byte, HSAMPLE, ctypes.POINTER(BASS_SAMPLE))(('BASS_SampleGetInfo', bass_module))
#BOOL BASSDEF(BASS_SampleSetInfo)(HSAMPLE handle, const BASS_SAMPLE *info);
BASS_SampleSetInfo = func_type(ctypes.c_byte, HSAMPLE, ctypes.POINTER(BASS_SAMPLE))(('BASS_SampleSetInfo', bass_module))
#HCHANNEL BASSDEF(BASS_SampleGetChannel)(HSAMPLE handle, BOOL onlynew);
BASS_SampleGetChannel = func_type(HCHANNEL, HSAMPLE, ctypes.c_byte)(('BASS_SampleGetChannel', bass_module))
#DWORD BASSDEF(BASS_SampleGetChannels)(HSAMPLE handle, HCHANNEL *channels);
BASS_SampleGetChannels = func_type(ctypes.c_ulong, HSAMPLE, ctypes.POINTER(HCHANNEL))(('BASS_SampleGetChannels', bass_module))
#BOOL BASSDEF(BASS_SampleStop)(HSAMPLE handle);
BASS_SampleStop = func_type(ctypes.c_byte, HSAMPLE)(('BASS_SampleStop', bass_module))

#HSTREAM BASSDEF(BASS_StreamCreate)(DWORD freq, DWORD chans, DWORD flags, STREAMPROC *proc, void *user);
BASS_StreamCreate = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, STREAMPROC, ctypes.c_void_p)(('BASS_StreamCreate', bass_module))
#HSTREAM BASSDEF(BASS_StreamCreateFile)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags);
BASS_StreamCreateFile = func_type(HSTREAM, ctypes.c_byte, ctypes.c_void_p, QWORD, QWORD, ctypes.c_ulong)(('BASS_StreamCreateFile', bass_module))
#HSTREAM BASSDEF(BASS_StreamCreateURL)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user);
BASS_StreamCreateURL = func_type(HSTREAM, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, DOWNLOADPROC, ctypes.c_void_p)(('BASS_StreamCreateURL', bass_module))
#HSTREAM BASSDEF(BASS_StreamCreateFileUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *proc, void *user);
BASS_StreamCreateFileUser = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(BASS_FILEPROCS), ctypes.c_void_p)(('BASS_StreamCreateFileUser', bass_module))
#BOOL BASSDEF(BASS_StreamFree)(HSTREAM handle);
BASS_StreamFree = func_type(ctypes.c_byte, HSTREAM)(('BASS_StreamFree', bass_module))
#QWORD BASSDEF(BASS_StreamGetFilePosition)(HSTREAM handle, DWORD mode);
BASS_StreamGetFilePosition = func_type(QWORD, HSTREAM, ctypes.c_ulong)(('BASS_StreamGetFilePosition', bass_module))
#DWORD BASSDEF(BASS_StreamPutData)(HSTREAM handle, const void *buffer, DWORD length);
BASS_StreamPutData = func_type(ctypes.c_ulong, HSTREAM, ctypes.c_void_p, ctypes.c_ulong)(('BASS_StreamPutData', bass_module))
#DWORD BASSDEF(BASS_StreamPutFileData)(HSTREAM handle, const void *buffer, DWORD length);
BASS_StreamPutFileData = func_type(ctypes.c_ulong, HSTREAM, ctypes.c_void_p, ctypes.c_ulong)(('BASS_StreamPutFileData', bass_module))

#BOOL BASSDEF(BASS_RecordGetDeviceInfo)(DWORD device, BASS_DEVICEINFO *info);
BASS_RecordGetDeviceInfo = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(BASS_DEVICEINFO))(('BASS_RecordGetDeviceInfo', bass_module))
#BOOL BASSDEF(BASS_RecordInit)(int device);
BASS_RecordInit = func_type(ctypes.c_byte, ctypes.c_int)(('BASS_RecordInit', bass_module))
#BOOL BASSDEF(BASS_RecordSetDevice)(DWORD device);
BASS_RecordSetDevice = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_RecordSetDevice', bass_module))
#DWORD BASSDEF(BASS_RecordGetDevice)();
BASS_RecordGetDevice = func_type(ctypes.c_ulong)(('BASS_RecordGetDevice', bass_module))
#BOOL BASSDEF(BASS_RecordFree)();
BASS_RecordFree = func_type(ctypes.c_byte)(('BASS_RecordFree', bass_module))
#BOOL BASSDEF(BASS_RecordGetInfo)(BASS_RECORDINFO *info);
BASS_RecordGetInfo = func_type(ctypes.c_byte, ctypes.POINTER(BASS_RECORDINFO))(('BASS_RecordGetInfo', bass_module))
#const char *BASSDEF(BASS_RecordGetInputName)(int input);
BASS_RecordGetInputName = func_type(ctypes.c_char_p, ctypes.c_int)(('BASS_RecordGetInputName', bass_module))
#BOOL BASSDEF(BASS_RecordSetInput)(int input, DWORD flags, float volume);
BASS_RecordSetInput = func_type(ctypes.c_byte, ctypes.c_int, ctypes.c_ulong, ctypes.c_float)(('BASS_RecordSetInput', bass_module))
#DWORD BASSDEF(BASS_RecordGetInput)(int input, float *volume);
BASS_RecordGetInput = func_type(ctypes.c_ulong, ctypes.c_int, ctypes.c_float)(('BASS_RecordGetInput', bass_module))
#HRECORD BASSDEF(BASS_RecordStart)(DWORD freq, DWORD chans, DWORD flags, RECORDPROC *proc, void *user);
BASS_RecordStart = func_type(HRECORD, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, RECORDPROC, ctypes.c_void_p)(('BASS_RecordStart', bass_module))

#double BASSDEF(BASS_ChannelBytes2Seconds)(DWORD handle, QWORD pos);
BASS_ChannelBytes2Seconds = func_type(ctypes.c_double, ctypes.c_ulong, QWORD)(('BASS_ChannelBytes2Seconds', bass_module))
#QWORD BASSDEF(BASS_ChannelSeconds2Bytes)(DWORD handle, double pos);
BASS_ChannelSeconds2Bytes = func_type(QWORD, ctypes.c_ulong, ctypes.c_double)(('BASS_ChannelSeconds2Bytes', bass_module))
#DWORD BASSDEF(BASS_ChannelGetDevice)(DWORD handle);
BASS_ChannelGetDevice = func_type(ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelGetDevice', bass_module))
#BOOL BASSDEF(BASS_ChannelSetDevice)(DWORD handle, DWORD device);
BASS_ChannelSetDevice = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelSetDevice', bass_module))
#DWORD BASSDEF(BASS_ChannelIsActive)(DWORD handle);
BASS_ChannelIsActive = func_type(ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelIsActive', bass_module))
#BOOL BASSDEF(BASS_ChannelGetInfo)(DWORD handle, BASS_CHANNELINFO *info);
BASS_ChannelGetInfo = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(BASS_CHANNELINFO))(('BASS_ChannelGetInfo', bass_module))
#const char *BASSDEF(BASS_ChannelGetTags)(DWORD handle, DWORD tags);
#~ BASS_ChannelGetTags = func_type(ctypes.c_char_p, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelGetTags', bass_module))
BASS_ChannelGetTags = func_type(ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelGetTags', bass_module))
#DWORD BASSDEF(BASS_ChannelFlags)(DWORD handle, DWORD flags, DWORD mask);
BASS_ChannelFlags = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelFlags', bass_module))
#BOOL BASSDEF(BASS_ChannelUpdate)(DWORD handle, DWORD length);
BASS_ChannelUpdate = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelUpdate', bass_module))
#BOOL BASSDEF(BASS_ChannelLock)(DWORD handle, BOOL lock);
BASS_ChannelLock = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_byte)(('BASS_ChannelLock', bass_module))
#BOOL BASSDEF(BASS_ChannelFree)(DWORD handle);
BASS_ChannelFree = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_ChannelFree', bass_module))
#BOOL BASSDEF(BASS_ChannelPlay)(DWORD handle, BOOL restart);
BASS_ChannelPlay = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_byte)(('BASS_ChannelPlay', bass_module))
#BOOL BASSDEF(BASS_ChannelStop)(DWORD handle);
BASS_ChannelStop = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_ChannelStop', bass_module))
#BOOL BASSDEF(BASS_ChannelPause)(DWORD handle);
BASS_ChannelPause = func_type(ctypes.c_byte, ctypes.c_ulong)(('BASS_ChannelPause', bass_module))
#BOOL BASSDEF(BASS_ChannelSetAttribute)(DWORD handle, DWORD attrib, float value);
BASS_ChannelSetAttribute = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_float)(('BASS_ChannelSetAttribute', bass_module))
#BOOL BASSDEF(BASS_ChannelGetAttribute)(DWORD handle, DWORD attrib, float *value);
BASS_ChannelGetAttribute = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(ctypes.c_float))(('BASS_ChannelGetAttribute', bass_module))
#BOOL BASSDEF(BASS_ChannelSlideAttribute)(DWORD handle, DWORD attrib, float value, DWORD time);
BASS_ChannelSlideAttribute = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_float, ctypes.c_ulong)(('BASS_ChannelSlideAttribute', bass_module))
#BOOL BASSDEF(BASS_ChannelIsSliding)(DWORD handle, DWORD attrib);
BASS_ChannelIsSliding = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelIsSliding', bass_module))
#BOOL BASSDEF(BASS_ChannelSet3DAttributes)(DWORD handle, int mode, float min, float max, int iangle, int oangle, float outvol);
BASS_ChannelSet3DAttributes = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_int, ctypes.c_float, ctypes.c_float, ctypes.c_int, ctypes.c_int, ctypes.c_float)(('BASS_ChannelSet3DAttributes', bass_module))
#BOOL BASSDEF(BASS_ChannelGet3DAttributes)(DWORD handle, DWORD *mode, float *min, float *max, DWORD *iangle, DWORD *oangle, float *outvol);
BASS_ChannelGet3DAttributes = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(ctypes.c_ulong), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_ulong), ctypes.POINTER(ctypes.c_ulong), ctypes.POINTER(ctypes.c_float))(('BASS_ChannelGet3DAttributes', bass_module))
#BOOL BASSDEF(BASS_ChannelSet3DPosition)(DWORD handle, const BASS_3DVECTOR *pos, const BASS_3DVECTOR *orient, const BASS_3DVECTOR *vel);
BASS_ChannelSet3DPosition = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR))(('BASS_ChannelSet3DPosition', bass_module))
#BOOL BASSDEF(BASS_ChannelGet3DPosition)(DWORD handle, BASS_3DVECTOR *pos, BASS_3DVECTOR *orient, BASS_3DVECTOR *vel);
BASS_ChannelGet3DPosition = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR), ctypes.POINTER(BASS_3DVECTOR))(('BASS_ChannelGet3DPosition', bass_module))
#QWORD BASSDEF(BASS_ChannelGetLength)(DWORD handle, DWORD mode);
BASS_ChannelGetLength = func_type(QWORD, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelGetLength', bass_module))
#BOOL BASSDEF(BASS_ChannelSetPosition)(DWORD handle, QWORD pos, DWORD mode);
BASS_ChannelSetPosition = func_type(ctypes.c_byte, ctypes.c_ulong, QWORD, ctypes.c_ulong)(('BASS_ChannelSetPosition', bass_module))
#QWORD BASSDEF(BASS_ChannelGetPosition)(DWORD handle, DWORD mode);
BASS_ChannelGetPosition = func_type(QWORD, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelGetPosition', bass_module))
#DWORD BASSDEF(BASS_ChannelGetLevel)(DWORD handle);
BASS_ChannelGetLevel = func_type(ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelGetLevel', bass_module))
#DWORD BASSDEF(BASS_ChannelGetData)(DWORD handle, void *buffer, DWORD length);
BASS_ChannelGetData = func_type(ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong)(('BASS_ChannelGetData', bass_module))
#HSYNC BASSDEF(BASS_ChannelSetSync)(DWORD handle, DWORD type, QWORD param, SYNCPROC *proc, void *user);
BASS_ChannelSetSync = func_type(HSYNC, ctypes.c_ulong, ctypes.c_ulong, QWORD, SYNCPROC, ctypes.c_void_p)(('BASS_ChannelSetSync', bass_module))
#BOOL BASSDEF(BASS_ChannelRemoveSync)(DWORD handle, HSYNC sync);
BASS_ChannelRemoveSync = func_type(ctypes.c_byte, ctypes.c_ulong, HSYNC)(('BASS_ChannelRemoveSync', bass_module))
#HDSP BASSDEF(BASS_ChannelSetDSP)(DWORD handle, DSPPROC *proc, void *user, int priority);
BASS_ChannelSetDSP = func_type(HDSP, ctypes.c_ulong, DSPPROC, ctypes.c_void_p, ctypes.c_int)(('BASS_ChannelSetDSP', bass_module))
#BOOL BASSDEF(BASS_ChannelRemoveDSP)(DWORD handle, HDSP dsp);
BASS_ChannelRemoveDSP = func_type(ctypes.c_byte, ctypes.c_ulong, HDSP)(('BASS_ChannelRemoveDSP', bass_module))
#BOOL BASSDEF(BASS_ChannelSetLink)(DWORD handle, DWORD chan);
BASS_ChannelSetLink = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelSetLink', bass_module))
#BOOL BASSDEF(BASS_ChannelRemoveLink)(DWORD handle, DWORD chan);
BASS_ChannelRemoveLink = func_type(ctypes.c_byte, ctypes.c_ulong, ctypes.c_ulong)(('BASS_ChannelRemoveLink', bass_module))
#HFX BASSDEF(BASS_ChannelSetFX)(DWORD handle, DWORD type, int priority);
BASS_ChannelSetFX = func_type(HFX, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_int)(('BASS_ChannelSetFX', bass_module))
#BOOL BASSDEF(BASS_ChannelRemoveFX)(DWORD handle, HFX fx);
BASS_ChannelRemoveFX = func_type(ctypes.c_byte, ctypes.c_ulong, HFX)(('BASS_ChannelRemoveFX', bass_module))

#BOOL BASSDEF(BASS_FXSetParameters)(HFX handle, const void *params);
BASS_FXSetParameters = func_type(ctypes.c_byte, HFX, ctypes.c_void_p)(('BASS_FXSetParameters', bass_module))
#BOOL BASSDEF(BASS_FXGetParameters)(HFX handle, void *params);
BASS_FXGetParameters = func_type(ctypes.c_byte, HFX, ctypes.c_void_p)(('BASS_FXGetParameters', bass_module))
#BOOL BASSDEF(BASS_FXReset)(HFX handle);
BASS_FXReset = func_type(ctypes.c_byte, HFX)(('BASS_FXReset', bass_module))


if platform.system().lower() == 'windows':
	#BOOL BASSDEF(BASS_Init)(int device, DWORD freq, DWORD flags, HWND win, const GUID *dsguid); = func_type()(('', bass_module))
	BASS_Init = func_type(ctypes.c_byte, ctypes.c_int, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p)(('BASS_Init', bass_module))
	#void *BASSDEF(BASS_GetDSoundObject)(DWORD object);
	BASS_GetDSoundObject = func_type(ctypes.c_void_p, ctypes.c_ulong)(('BASS_GetDSoundObject', bass_module))
	#BOOL BASSDEF(BASS_SetEAXParameters)(int env, float vol, float decay, float damp);
	BASS_SetEAXParameters = func_type(ctypes.c_byte, ctypes.c_int, ctypes.c_float, ctypes.c_float, ctypes.c_float)(('BASS_SetEAXParameters', bass_module))
	#BOOL BASSDEF(BASS_GetEAXParameters)(DWORD *env, float *vol, float *decay, float *damp);
	BASS_GetEAXParameters = func_type(ctypes.c_byte, ctypes.POINTER(ctypes.c_ulong), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float))(('BASS_GetEAXParameters', bass_module))

def seconds_to_string(value):
	str_seconds = '00'
	str_minutes = '00'
	str_hours = '00'
	seconds = int(value)
	if seconds > 0:
		if seconds < 10:
			str_seconds = '0' + str(seconds)
		elif seconds > 60:
			str_seconds = str(seconds%60)
		else:
			str_seconds = str(seconds)
	minutes = int(value/60)
	if minutes > 0:
		if minutes < 10:
			str_minutes = '0' + str(minutes)
		elif minutes > 60:
			str_minutes = str(minutes%60)
		else:
			str_minutes = str(minutes)
	hours = int(minutes/60)
	if hours > 0:
		if hours < 10:
			str_hours = '0' + str(hours)
		elif hours < 60:
			str_hours = str(hours)
	return str_hours + ':' + str_minutes + ':' + str_seconds

def stream_length_as_hms(handle, mode = BASS_POS_BYTE):
	return seconds_to_string(BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetLength(handle, mode)))

def get_tags(handle, tags = BASS_TAG_OGG):
	result = []
	addr = BASS_ChannelGetTags(handle, tags)
	res = ''
	while isinstance(res, str):
		res = ctypes.string_at(addr)
		#~ res = ctypes.wstring_at(addr)
		addr += len(res) + 1
		if res:
			if 32 < ord(res[0]) < 256:
				result.append(res)
			else:
				res = None
	return result

def get_tags_as_dict(handle, tags = BASS_TAG_OGG):
	result_as_dict = {}
	addr = BASS_ChannelGetTags(handle, tags)
	str_tag = ''
	while isinstance(str_tag, str):
		str_tag = ctypes.string_at(addr)
		addr += len(str_tag) + 1
		if str_tag:
			if 32 < ord(str_tag[0]) < 256:
				key, value = str_tag.split('=')
				result_as_dict[key] = value
			else:
				str_tag = None
	return result_as_dict
