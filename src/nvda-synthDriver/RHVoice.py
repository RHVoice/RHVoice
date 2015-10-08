# -*- coding: utf-8 -*-
# Copyright (C) 2010-2013  Olga Yakovleva <yakovleva.o.v@gmail.com>
# Copyright (C) 2015  anatoly techtonik <techtonik@gmail.com>

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

__author__ = "Olga Yakovleva <yakovleva.o.v@gmail.com>"
__version__ = "0.5.1.5"
# version decoded:
#
#              ^^^^^    -- RHVoice.dll (RHVoice.so) version that was
#                          tested with this module
#                   ^^  -- RHVoice.py module version

import os
import sys
import time
import wave

from ctypes import CDLL, CFUNCTYPE, POINTER, Structure, c_char_p, c_double
from ctypes import c_int, c_uint, c_short, c_void_p, byref, sizeof, string_at

DEBUG=0

# --- workarounds for Python misbehavior ---

# enable passing unicode argument from command line
# https://stackoverflow.com/questions/846850/read-unicode-characters
def win32_utf8_argv():
    """Uses shell32.GetCommandLineArgvW to get sys.argv as a list of Unicode
    strings.

    Versions 2.x of Python don't support Unicode in sys.argv on
    Windows, with the underlying Windows API instead replacing multi-byte
    characters with '?'.
    """

    from ctypes import POINTER, byref, cdll, c_int, windll
    from ctypes.wintypes import LPCWSTR, LPWSTR

    GetCommandLineW = cdll.kernel32.GetCommandLineW
    GetCommandLineW.argtypes = []
    GetCommandLineW.restype = LPCWSTR

    CommandLineToArgvW = windll.shell32.CommandLineToArgvW
    CommandLineToArgvW.argtypes = [LPCWSTR, POINTER(c_int)]
    CommandLineToArgvW.restype = POINTER(LPWSTR)

    cmd = GetCommandLineW()
    argc = c_int(0)
    argv = CommandLineToArgvW(cmd, byref(argc))
    argnum = argc.value
    sysnum = len(sys.argv)
    result = []
    if argnum > 0:
        # Remove Python executable and commands if present
        start = argnum - sysnum
        for i in range(start, argnum):
            result.append(argv[i].encode('utf-8'))
    return result

# --- bindings ---

class RHVoice_tts_engine_struct(Structure):
    pass
RHVoice_tts_engine=POINTER(RHVoice_tts_engine_struct)

class RHVoice_message_struct(Structure):
    pass
RHVoice_message=POINTER(RHVoice_message_struct)


class RHVoice_callback_types:
    play_speech=CFUNCTYPE(c_int,POINTER(c_short),c_uint,c_void_p)
    process_mark=CFUNCTYPE(c_int,c_char_p,c_void_p)
    word_starts=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    word_ends=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    sentence_starts=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    sentence_ends=CFUNCTYPE(c_int,c_uint,c_uint,c_void_p)
    play_audio=CFUNCTYPE(c_int,c_char_p,c_void_p)

class RHVoice_callbacks(Structure):
    _fields_=[("play_speech",RHVoice_callback_types.play_speech),
              ("process_mark",RHVoice_callback_types.process_mark),
              ("word_starts",RHVoice_callback_types.word_starts),
              ("word_ends",RHVoice_callback_types.word_ends),
              ("sentence_starts",RHVoice_callback_types.sentence_starts),
              ("sentence_ends",RHVoice_callback_types.sentence_ends),
              ("play_audio",RHVoice_callback_types.play_audio)]

class RHVoice_init_params(Structure):  # from RHVoice.h
    _fields_=[("data_path",c_char_p),
              ("config_path",c_char_p),
              ("resource_paths",POINTER(c_char_p)),
              ("callbacks",RHVoice_callbacks),
              ("options",c_uint)]


class RHVoice_voice_info(Structure):
    _fields_=[("language",c_char_p),
              ("name",c_char_p),
              ("gender",c_int)]

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
              ("capitals_mode",c_int)]

class RHVoice_message_type:
    text=0
    ssml=1
    characters=2

class RHVoice_voice_gender:
    unknown=0
    male=1
    female=2

class RHVoice_punctuation_mode:
    default=0
    none=1
    all=2
    some=3

class RHVoice_capitals_mode:
    default=0
    off=1
    word=2
    pitch=3
    sound=4

# --- main code ---

def get_library_location():
    root = os.path.dirname(__file__.decode(sys.getfilesystemencoding()))
    if os.name == 'nt':
        libname = 'RHVoice.dll'
    else:
        libname = 'libRHVoice.so'
    lookup_paths = [root, os.path.join(root, "../../build/linux/lib")]
    for p in lookup_paths:
        libpath = os.path.abspath(os.path.join(p, libname))
        if os.path.exists(libpath):
            break
    else:
        libpath = libname
    if DEBUG:
        print("detected dll/so path: %s " % libpath)
    return libpath

def load_tts_library():
    lib=CDLL(get_library_location().encode(sys.getfilesystemencoding()))
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


class SpeechCallback(object):
    """
    This is a callable object for using in RHVoice speech_callback. The
    only obligatory part is __call__() method with its parameters.
    """
    sample_size = sizeof(c_short)

    def __call__(self, samples, count, user_data):
        """Should return False to stop synthesis"""
        return True

class DebugCallback(SpeechCallback):
    """ Callback that prints info about generated samples. """
    def __init__(self):
        self.counter = 0
        self.datasize = 0
        self.starttime = time.clock()

    def __call__(self, samples, count, user_data):
        self.counter += 1
        size = count*self.sample_size
        self.datasize += size
        kbps = self.datasize / (time.clock() - self.starttime) / 1024
        self.debug(count, size, kbps)
        return True

    def debug(self, count, size, kbps):
        print("speech callback %s time(s) samples: %s, size: %s, %.2f kBps" % (self.counter, count, size, kbps))

class WaveWriteCallback(SpeechCallback):
    """ Callback that writes sound to wave file. """
    def __init__(self, filename, ):
        super(WaveWriteCallback, self).__init__()
        self.wavefile = wave.open(filename, 'wb')
        self.wavefile.setnchannels(1)
        self.wavefile.setsampwidth(self.sample_size)
        self.wavefile.setframerate(16000)

    def __call__(self, samples, count, user_data):
        """Should return False to stop synthesis"""
        self.wavefile.writeframes(string_at(samples, count*self.sample_size))
        return True


# --- Global state. High level API ---

# Reference to loaded library
LIB = None
# This dir is used for distributing voices in Python Wheel
DATADIR = "RHVoice.langdata"

def get_rhvoice_version():
    global LIB
    if not LIB:
        LIB = load_tts_library()
    return LIB.RHVoice_get_version()

def init_rhvoice(datadir=DATADIR, callback=DebugCallback()):
    """
    Load DLL and initialize speech engine - load language data
    and set callbacks.
    """
       
    global LIB
    global init_params  # need to preserve reference for ctypes
    if not LIB:
        LIB = load_tts_library()
    LIB.RHVoice_set_logging(True)

    # creating obligatory callback with .play_speech()
    # RHVoice_new_tts_engine fails without callback
    c_speech_callback = RHVoice_callback_types.play_speech(callback)
    callbacks = RHVoice_callbacks()
    callbacks.play_speech = c_speech_callback
    # possible callbacks
    """
    RHVoice_callbacks(self.__c_speech_callback,
        self.__c_mark_callback,
        cast(None,RHVoice_callback_types.word_starts),
        cast(None,RHVoice_callback_types.word_ends),
        cast(None,RHVoice_callback_types.sentence_starts),
        cast(None,RHVoice_callback_types.sentence_ends),
        cast(None,RHVoice_callback_types.play_audio)),
    """

    init_params = RHVoice_init_params()
    init_params.data_path = datadir
    init_params.callbacks = callbacks

    return LIB.RHVoice_new_tts_engine(byref(init_params))

def get_voices(engine):
    """
    Returns nested dictionary with voice information. First
    level key is voice name in lowercase, second level keys
    are voice properties.
    """
    global LIB
    genders = {1: 'male', 2:'female'}
    voices = dict()
    voices_total = LIB.RHVoice_get_number_of_voices(engine)
    first_voice = LIB.RHVoice_get_voices(engine)
    for voiceno in range(voices_total):
        vi = first_voice[voiceno]
        key = vi.name.lower()
        voices[key] = dict(
            no = voiceno,
            name = vi.name,
            lang = vi.language,
            gender = genders[vi.gender]
        )
    return voices


def main():
    global DATADIR, DEBUG

    usage = """
Usage:
  1. RHVoice.py <command>
  2. RHVoice.py [--debug] [-o output.wav] \"text\"
  3. RHVoice.py [--debug] [-o output.wav] -i input.txt

Commands:
  list          - list voices loaded from datadir
  version       - show version of C module and Python API

Options:
  -i --input FILE       file with text encoded in UTF-8
  -o --output FILE      output filename (default: output.wav)
  --datadir DATADIR     path to language data
                        (default: %(datadir)s)
  --voice NAME[,NAME]   choose voices
  --debug               show debug info
""" % dict(datadir=DATADIR)


    # --- patch Python to read unicode from command line ---
    if sys.platform == "win32":
        sys.argv = win32_utf8_argv()

    # --- process commands ---
    possible_command = sys.argv[1:2]
    if possible_command == ['version']:
        print("RHVoice %s" % get_rhvoice_version())
        print("Python API %s" % __version__)
        sys.exit(0)

    # --- process options ---
    import optparse
    parser = optparse.OptionParser()
    parser.add_option("-i", "--input",
                      help="file with text encoded in UTF-8")
    parser.add_option("-o", "--output", default="output.wav",
                      help="output filename (default: output.wav)")
    parser.add_option("--datadir",
                      help="path to language data (default: %s/)" % DATADIR)
    parser.add_option("--voice",
                      help="choose voices")

    parser.add_option("--debug", help="show debug info", action="store_true")
    opts, args = parser.parse_args()
    if not args and not opts.input:
        #parser.print_help()
        print(usage)
        print("\nError: No input text")
        sys.exit(-1)

    if opts.debug:
        DEBUG = 1

    root_path = os.path.dirname(__file__.decode(sys.getfilesystemencoding()))
    data_path = os.path.join(root_path, DATADIR)
    if opts.datadir:
        data_path = opts.datadir

    # --- setup synthesizer and main ---

    if DEBUG:
        print("RHVoice Python %s" % __version__)
        print("Data Path: %s" % data_path)
        print("")
    if not os.path.exists(data_path):
        print("WARNING: Language resource dir does not exist - ")
        print("    %s" % data_path)
        print("")

    engine = init_rhvoice(datadir=data_path, callback=WaveWriteCallback(opts.output))
    if not engine:
        if DEBUG:
            raise RuntimeError("RHVoice: engine initialization error")
        else:
            sys.exit("RuntimeError: RHVoice: engine initialization error")
    print("Writing to: %s" % opts.output)

    # --- voice ---
    # [ ] multiple voice selection (profile) is not there yet
    voices = get_voices(engine)
    voice_selected = []
    voice_selected_name = []
    if opts.voice:
        for v in opts.voice.split(','):
            if v.lower() not in voices:
                print("\nError: Voice %s is unknown." % v)
                sys.exit(-1)
            else:
                if DEBUG:
                    print("Selecting voice: %s" % v)
                voice_selected.append(voices[v.lower()]["no"])
                voice_selected_name.append(voices[v.lower()]["name"])
    if not voice_selected:
        # choose the first voice (doesn't depend on language)
        voice_selected = [0]
        voice_selected_name = [voices[v]["name"] for v in voices if voices[v]["no"] == 0]
    elif len(voice_selected) > 1:
        print("\nNote: Sorry, multiple language selection doesn't work yet")
    print("Selected voice: %s" % voice_selected_name[0])

    # --- list ---
    if possible_command == ['list']:
        if DEBUG:
            print("    Voice     Language  Gender")
        #from pprint import pprint
        #pprint(voices)
        #  {'alan': {'gender': 'male', 'lang': 'en', 'name': 'Alan', 'no': 0},
        #   'aleksandr': {'gender': 'male', 'lang': 'ru', 'name': 'Aleksandr', 'no': 1},
        #  ...
        # sort voices by their no, which is id to select them
        voice_order = sorted(voices.items(), key=lambda x: x[1]["no"])
        voice_order = [v[0] for v in voice_order]
        for i in range(len(voices)):
            voice = voices[voice_order[i]]
            print("  %(name)-16s  %(lang)2s    %(gender)2s " % voice)
        if DEBUG:
            print("Number of voices: %s" % len(voices))
        sys.exit(0)



    lib = LIB

    profiles_total = lib.RHVoice_get_number_of_voice_profiles(engine)
    profiles = []
    first_profile = lib.RHVoice_get_voice_profiles(engine)
    for profno in range(profiles_total):
            profiles.append(first_profile[profno])
    if DEBUG:
        print("Voice Profiles")
        for p in profiles:
            print(" %s" % p)

    # transform text to RHVoice_message for RHVoice_speak 
    # RHVoice_new_message is a function to do so. Its parameters:
    # (RHVoice_tts_engine, c_char_p, c_uint, c_int, POINTER(RHVoice_synth_params), c_void_p)
    # (tts_engine, const char* text, length, RHVoice_message_type,   synth_params, void* user_data)

    text = ""
    if args:
        text += args[0]
    else:
        text += open(opts.input, "rb").read()

    # message also specifies voice parameters, which are obligatory
    synth_params = RHVoice_synth_params()
    # choosing voice. profile is a set of voices for multi-language text.
    # there are default profiles with single voice selected, so for now
    # we just use them.
    # [ ] multiple voice selection
    synth_params.voice_profile = profiles[voice_selected[0]]
    synth_params.relative_pitch = 1.0
    synth_params.relative_rate = 1.0
    # setting volume is tricky, relative_volume value 1.0 produces
    # -4 dB with voice Alan, and -16 dB if setting is not set
    # (measured with in Audacity)
    synth_params.relative_volume = 1.0

    message = lib.RHVoice_new_message(engine,
                                      text,
                                      len(text),
                                      RHVoice_message_type.text,
                                      byref(synth_params),
                                      None)

    if not message:
        raise RuntimeError("RHVoice: message building error")
    lib.RHVoice_speak(message)
    lib.RHVoice_delete_message(message)  # free the memory (check when message is stored)

if __name__ == '__main__':
    main()
