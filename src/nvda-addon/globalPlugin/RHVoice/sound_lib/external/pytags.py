from __future__ import absolute_import
# Copyright(c) Max Kolosov 2009 maxkolosov@inbox.ru
# http://vosolok2008.narod.ru
# BSD license

__version__ = '0.1'
__versionTime__ = '2009-11-10'
__author__ = 'Max Kolosov <maxkolosov@inbox.ru>'
__doc__ = '''
pytags.py - is ctypes python module for
TAGS (Another Tags Reading Library written by Wraith).

BASS audio library has limited support for reading tags, associated with a
stream. This library extends that functionality, allowing developer/user to
extract specific song information from the stream handle used with BASS. The
extracted tag values are formatted into text ouput according to given format
string (including conditional processing).

Supported tags:
---------------
  MP3 ID3v1 and ID3v2.2/3/4
  OGG/FLAC comments
  WMA
  APE, OFR, MPC, AAC - all use APE tags
  MP4
  MOD/etc titles
'''

import sys, ctypes, platform
from .paths import x86_path, x64_path
from . import libloader
tags_module = libloader.load_library('tags', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()# Current version. Just increments each release.

TAGS_VERSION = 17

# returns description of the last error.
#const char*  _stdcall TAGS_GetLastErrorDesc();
TAGS_GetLastErrorDesc = func_type(ctypes.c_char_p)(('TAGS_GetLastErrorDesc', tags_module))

# main purpose of this library
#const char*  _stdcall TAGS_Read( DWORD dwHandle, const char* fmt );
TAGS_Read = func_type(ctypes.c_char_p, ctypes.c_ulong, ctypes.c_char_p)(('TAGS_Read', tags_module))

# retrieves the current version
#DWORD _stdcall TAGS_GetVersion();
TAGS_GetVersion = func_type(ctypes.c_ulong)(('TAGS_GetVersion', tags_module))

