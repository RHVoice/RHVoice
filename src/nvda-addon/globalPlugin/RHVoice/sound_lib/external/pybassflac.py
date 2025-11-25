from __future__ import absolute_import
# Copyright(c) Max Kolosov 2009 maxkolosov@inbox.ru
# http://vosolok2008.narod.ru
# BSD license

__version__ = '0.1'
__versionTime__ = '2009-11-15'
__author__ = 'Max Kolosov <maxkolosov@inbox.ru>'
__doc__ = '''
pybassflac.py - is ctypes python module for
BASSFLAC - extension to the BASS audio library,
enabling the playing of FLAC (Free Lossless Audio Codec) encoded files.
'''

import os, sys, ctypes
from . import pybass
from . paths import x86_path, x64_path
from . import libloader

bassflac_module = libloader.load_library('bassflac', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()
#Register the plugin with the Bass plugin system.
pybass.BASS_PluginLoad(libloader.find_library_path('bassflac', x86_path=x86_path, x64_path=x64_path), 0)

QWORD = pybass.QWORD
HSTREAM = pybass.HSTREAM
DOWNLOADPROC = pybass.DOWNLOADPROC
BASS_FILEPROCS = pybass.BASS_FILEPROCS

# BASS_CHANNELINFO type
BASS_CTYPE_STREAM_FLAC = 0x10900
BASS_CTYPE_STREAM_FLAC_OGG = 0x10901


#HSTREAM BASSFLACDEF(BASS_FLAC_StreamCreateFile)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags);
BASS_FLAC_StreamCreateFile = func_type(HSTREAM, ctypes.c_byte, ctypes.c_void_p, QWORD, QWORD, ctypes.c_ulong)(('BASS_FLAC_StreamCreateFile', bassflac_module))
#HSTREAM BASSFLACDEF(BASS_FLAC_StreamCreateURL)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user);
BASS_FLAC_StreamCreateURL = func_type(HSTREAM, ctypes.c_char_p, ctypes.c_ulong, ctypes.c_ulong, DOWNLOADPROC, ctypes.c_void_p)(('BASS_FLAC_StreamCreateURL', bassflac_module))
#HSTREAM BASSFLACDEF(BASS_FLAC_StreamCreateFileUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user);
BASS_FLAC_StreamCreateFileUser = func_type(HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.POINTER(BASS_FILEPROCS), ctypes.c_void_p)(('BASS_FLAC_StreamCreateFileUser', bassflac_module))

