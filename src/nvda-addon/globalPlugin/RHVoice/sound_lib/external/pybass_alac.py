from __future__ import absolute_import
"BASS_ALAC wrapper by Christopher Toth"""

import ctypes
import os
from . import pybass
from .paths import x86_path, x64_path
from . import libloader

bass_alac_module = libloader.load_library('bass_alac', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()

pybass.BASS_PluginLoad(libloader.find_library_path('bass_alac', x86_path=x86_path, x64_path=x64_path), 0)

BASS_TAG_MP4 = 7
BASS_CTYPE_STREAM_ALAC = 0x10e00


#HSTREAM BASSALACDEF(BASS_ALAC_StreamCreateFile)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags);
BASS_ALAC_StreamCreateFile = func_type(pybass.HSTREAM, ctypes.c_byte, ctypes.c_void_p, pybass.QWORD, pybass.QWORD, ctypes.c_ulong)

#HSTREAM BASSALACDEF(BASS_ALAC_StreamCreateFileUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *procs, void *user);
BASS_ALAC_StreamCreateFileUser = func_type(pybass.HSTREAM, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p, ctypes.c_void_p)

