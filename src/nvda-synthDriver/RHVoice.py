# -*- coding: utf-8 -*-
# Copyright (C) 2010, 2011, 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import sys

from ctypes import CDLL, CFUNCTYPE, POINTER, Structure, c_char_p, c_double
from ctypes import c_int, c_uint, c_short, c_void_p


# --- bindings ---

class RHVoice_tts_engine_struct(Structure):
    pass
RHVoice_tts_engine=POINTER(RHVoice_tts_engine_struct)

class RHVoice_message_struct(Structure):
    pass
RHVoice_message=POINTER(RHVoice_message_struct)


class RHVoice_callback_types:
    set_sample_rate=CFUNCTYPE(c_int,c_int,c_void_p)
    play_speech=CFUNCTYPE(c_int,POINTER(c_short),c_uint,c_void_p)
    process_mark=CFUNCTYPE(c_int,c_char_p,c_void_p)
    word_starts=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    word_ends=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    sentence_starts=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    sentence_ends=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    play_audio=CFUNCTYPE(c_int,c_char_p,c_void_p)
    done=CFUNCTYPE(None,c_void_p)

class RHVoice_callbacks(Structure):
    _fields_=[("set_sample_rate",RHVoice_callback_types.set_sample_rate),
              ("play_speech",RHVoice_callback_types.play_speech),
              ("process_mark",RHVoice_callback_types.process_mark),
              ("word_starts",RHVoice_callback_types.word_starts),
              ("word_ends",RHVoice_callback_types.word_ends),
              ("sentence_starts",RHVoice_callback_types.sentence_starts),
              ("sentence_ends",RHVoice_callback_types.sentence_ends),
              ("play_audio",RHVoice_callback_types.play_audio),
              ("done",RHVoice_callback_types.done)]

class RHVoice_init_params(Structure):
    _fields_=[("data_path",c_char_p),
              ("config_path",c_char_p),
              ("resource_paths",POINTER(c_char_p)),
              ("callbacks",RHVoice_callbacks),
              ("options",c_uint)]


class RHVoice_voice_info(Structure):
    _fields_=[("language",c_char_p),
              ("name",c_char_p),
              ("gender",c_int),
              ("country",c_char_p)]

class RHVoice_synth_params(Structure):
    _fields_=[("voice_profile",c_char_p),
              ("absolute_rate",c_double),
              ("absolute_pitch",c_double),
              ("absolute_volume",c_double),
              ("relative_rate",c_double),
              ("relative_pitch",c_double),
              ("relative_volume",c_double),
              ("punctuation_mode",c_int),
              ("punctuation_list",c_char_p),
              ("capitals_mode",c_int),
              ("flags",c_int)]

# --- main code ---

try:
	module_dir=os.path.dirname(__file__.decode("mbcs"))
except AttributeError:
	module_dir=os.path.dirname(__file__)
RHVoice_lib_path=os.path.join(module_dir,"RHVoice.dll")

def load_tts_library():
    try:
        lib=ctypes.CDLL(RHVoice_lib_path.encode("mbcs"))
    except TypeError:
        lib=ctypes.CDLL(RHVoice_lib_path)
    lib.RHVoice_get_version.restype=c_char_p
    lib.RHVoice_new_tts_engine.argtypes=(POINTER(RHVoice_init_params),)
    lib.RHVoice_new_tts_engine.restype=RHVoice_tts_engine
    lib.RHVoice_delete_tts_engine.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_delete_tts_engine.restype=None
    lib.RHVoice_get_number_of_voices.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_get_number_of_voices.restype=c_uint
    lib.RHVoice_get_voices.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_get_voices.restype=POINTER(RHVoice_voice_info)
    lib.RHVoice_get_number_of_voice_profiles.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_get_number_of_voice_profiles.restype=c_uint
    lib.RHVoice_get_voice_profiles.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_get_voice_profiles.restype=POINTER(c_char_p)
    lib.RHVoice_are_languages_compatible.argtypes=(RHVoice_tts_engine,c_char_p,c_char_p)
    lib.RHVoice_are_languages_compatible.restype=c_int
    lib.RHVoice_new_message.argtypes=(RHVoice_tts_engine,c_char_p,c_uint,c_int,POINTER(RHVoice_synth_params),c_void_p)
    lib.RHVoice_new_message.restype=RHVoice_message
    lib.RHVoice_delete_message.arg_types=(RHVoice_message,)
    lib.RHVoice_delete_message.restype=None
    lib.RHVoice_speak.argtypes=(RHVoice_message,)
    lib.RHVoice_speak.restype=c_int
    return lib

def main():
    lib = load_tts_library()
    print("RHVoice %s" % lib.RHVoice_get_version())

if __name__ == '__main__':
    main()
