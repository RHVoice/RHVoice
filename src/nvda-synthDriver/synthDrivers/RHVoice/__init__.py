# -*- coding: utf-8 -*-
# Copyright (C) 2010, 2011, 2012  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import sys
import os.path
import Queue
from collections import OrderedDict,defaultdict
import threading
import ctypes
from ctypes import c_char_p,c_wchar_p,c_void_p,c_short,c_int,c_uint,c_double,POINTER,Structure,sizeof,string_at,CFUNCTYPE,byref,cast

import config
import nvwave
from logHandler import log
from synthDriverHandler import SynthDriver,VoiceInfo
import speech
import languageHandler

module_dir=os.path.dirname(__file__.decode(sys.getfilesystemencoding()))
lib_path=os.path.join(module_dir,"RHVoice.dll")
data_path=os.path.join(module_dir,"data")
config_path=os.path.join(config.getUserDefaultConfigPath(),"RHVoice-config")

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

class RHVoice_init_params(Structure):
    _fields_=[("data_path",c_char_p),
              ("config_path",c_char_p),
              ("callbacks",RHVoice_callbacks),
              ("options",c_uint)]

class RHVoice_message_type:
    text=0
    ssml=1
    characters=2

class RHVoice_voice_gender:
    unknown=0
    male=1
    female=2

class RHVoice_voice_info(Structure):
    _fields_=[("language",c_char_p),
              ("name",c_char_p),
              ("gender",c_int)]

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

class RHVoice_synth_params(Structure):
    _fields_=[("main_voice",c_char_p),
              ("extra_voice",c_char_p),
              ("absolute_rate",c_double),
              ("absolute_pitch",c_double),
              ("absolute_volume",c_double),
              ("relative_rate",c_double),
              ("relative_pitch",c_double),
              ("relative_volume",c_double),
              ("punctuation_mode",c_int),
              ("punctuation_list",c_char_p),
              ("capitals_mode",c_int)]

def load_tts_library():
    lib=ctypes.CDLL(lib_path.encode(sys.getfilesystemencoding()))
    lib.RHVoice_get_version.restype=c_char_p
    lib.RHVoice_new_tts_engine.argtypes=(POINTER(RHVoice_init_params),)
    lib.RHVoice_new_tts_engine.restype=RHVoice_tts_engine
    lib.RHVoice_delete_tts_engine.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_delete_tts_engine.restype=None
    lib.RHVoice_get_number_of_voices.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_get_number_of_voices.restype=c_uint
    lib.RHVoice_get_voices.argtypes=(RHVoice_tts_engine,)
    lib.RHVoice_get_voices.restype=POINTER(RHVoice_voice_info)
    lib.RHVoice_are_languages_compatible.argtypes=(RHVoice_tts_engine,c_char_p,c_char_p)
    lib.RHVoice_are_languages_compatible.restype=c_int
    lib.RHVoice_new_message.argtypes=(RHVoice_tts_engine,c_char_p,c_uint,c_int,POINTER(RHVoice_synth_params),c_void_p)
    lib.RHVoice_new_message.restype=RHVoice_message
    lib.RHVoice_delete_message.arg_types=(RHVoice_message,)
    lib.RHVoice_delete_message.restype=None
    lib.RHVoice_speak.argtypes=(RHVoice_message,)
    lib.RHVoice_speak.restype=c_int
    return lib

def escape_text(text):
    parts=list()
    for c in text:
        if c.isspace():
            part=u"&#{};".format(ord(c))
        elif c=="<":
            part=u"&lt;"
        elif c==">":
            part=u"&gt;"
        elif c=="&":
            part=u"&amp;"
        elif c=="'":
            part=u"&apos;"
        elif c=='"':
            part=u"&quot;"
        else:
            part=c
        parts.append(part)
    return u"".join(parts)

class speech_callback(object):
    def __init__(self,lib,player,cancel_flag):
        self.__lib=lib
        self.__player=player
        self.__cancel_flag=cancel_flag

    def __call__(self,samples,count,user_data):
        try:
            if self.__cancel_flag.is_set():
                return 0
            try:
                self.__player.feed(string_at(samples,count*sizeof(c_short)))
            except:
                log.debugWarning("Error feeding audio to nvWave",exc_info=True)
            return 1
        except:
            log.error("RHVoice speech callback",exc_info=True)
            return 0

class mark_callback(object):
    def __init__(self,lib):
        self.__lib=lib
        self.__lock=threading.Lock()
        self.__index=None

    @property
    def index(self):
        with self.__lock:
            return self.__index

    @index.setter
    def index(self,value):
        with self.__lock:
            self.__index=value

    def __call__(self,name,user_data):
        try:
            self.index=int(name)
            return 1
        except:
            log.error("RHVoice mark callback",exc_info=True)
            return 0

class speak_text(object):
    def __init__(self,lib,tts_engine,text,cancel_flag):
        self.__lib=lib
        self.__tts_engine=tts_engine
        self.__text=text.encode("utf-8")
        self.__cancel_flag=cancel_flag
        self.__synth_params=RHVoice_synth_params(main_voice=None,
                                                 extra_voice=None,
                                                 absolute_rate=0,
                                                 relative_rate=1,
                                                 absolute_pitch=0,
                                                 relative_pitch=1,
                                                 absolute_volume=0,
                                                 relative_volume=1,
                                                 punctuation_mode=RHVoice_punctuation_mode.default,
                                                 punctuation_list=None,
                                                 capitals_mode=RHVoice_capitals_mode.default)

    def set_rate(self,rate):
        self.__synth_params.absolute_rate=rate/50.0-1

    def set_pitch(self,pitch):
        self.__synth_params.absolute_pitch=pitch/50.0-1

    def set_volume(self,volume):
        self.__synth_params.absolute_volume=volume/50.0-1

    def set_main_voice(self,name):
        self.__synth_params.main_voice=name

    def set_extra_voice(self,name):
        self.__synth_params.extra_voice=name

    def __call__(self):
        if self.__cancel_flag.is_set():
            return
        msg=self.__lib.RHVoice_new_message(self.__tts_engine,
                                           self.__text,
                                           len(self.__text),
                                           RHVoice_message_type.ssml,
                                           byref(self.__synth_params),
                                           None)
        if msg:
            self.__lib.RHVoice_speak(msg)
            self.__lib.RHVoice_delete_message(msg)

class TTSThread(threading.Thread):
    def __init__(self,tts_queue):
        self.__queue=tts_queue
        threading.Thread.__init__(self)
        self.daemon=True

    def run(self):
        while True:
            try:
                task=self.__queue.get()
                if task is None:
                    break
                else:
                    task()
            except:
                log.error("RHVoice: error while executing a tts task",exc_info=True)

class SynthDriver(SynthDriver):
    name="RHVoice"
    description="RHVoice"

    supportedSettings=(SynthDriver.VoiceSetting(),SynthDriver.RateSetting(),SynthDriver.PitchSetting(),SynthDriver.VolumeSetting())

    @classmethod
    def check(cls):
        return os.path.isfile(lib_path)

    def __init__(self):
        self.__lib=load_tts_library()
        self.__cancel_flag=threading.Event()
        self.__player=nvwave.WavePlayer(channels=1,samplesPerSec=16000,bitsPerSample=16,outputDevice=config.conf["speech"]["outputDevice"])
        self.__speech_callback=speech_callback(self.__lib,self.__player,self.__cancel_flag)
        self.__c_speech_callback=RHVoice_callback_types.play_speech(self.__speech_callback)
        self.__mark_callback=mark_callback(self.__lib)
        self.__c_mark_callback=RHVoice_callback_types.process_mark(self.__mark_callback)
        init_params=RHVoice_init_params(data_path.encode("utf-8"),
                                        config_path.encode("utf-8"),
                                        RHVoice_callbacks(self.__c_speech_callback,
                                                          self.__c_mark_callback,
                                                          cast(None,RHVoice_callback_types.word_starts),
                                                          cast(None,RHVoice_callback_types.word_ends),
                                                          cast(None,RHVoice_callback_types.sentence_starts),
                                                          cast(None,RHVoice_callback_types.sentence_ends),
                                                          cast(None,RHVoice_callback_types.play_audio)),
                                        0)
        self.__tts_engine=self.__lib.RHVoice_new_tts_engine(byref(init_params))
        if not self.__tts_engine:
            raise RuntimeError("RHVoice: initialization error")
        number_of_voices=self.__lib.RHVoice_get_number_of_voices(self.__tts_engine)
        voices=self.__lib.RHVoice_get_voices(self.__tts_engine)
        self.__voices_by_language=defaultdict(list)
        for i in xrange(number_of_voices):
            voice=voices[i]
            self.__voices_by_language[voice.language].append(voice.name)
        nvda_language=languageHandler.getLanguage().split("_")[0]
        if nvda_language in self.__voices_by_language:
            self.__language=nvda_language
        else:
            self.__lib.RHVoice_delete_tts_engine(self.__tts_engine)
            raise RuntimeError("RHVoice: unsupported language {}".format(nvda_language))
        self.__voice=self.__voices_by_language[self.__language][0]
        self.__rate=50
        self.__pitch=50
        self.__volume=50
        self.__tts_queue=Queue.Queue()
        self.__tts_thread=TTSThread(self.__tts_queue)
        self.__tts_thread.start()
        log.info("Using RHVoice version {}".format(self.__lib.RHVoice_get_version()))

    def terminate(self):
        self.cancel()
        self.__tts_queue.put(None)
        self.__tts_thread.join()
        self.__player.close()
        self.__lib.RHVoice_delete_tts_engine(self.__tts_engine)
        self.__tts_engine=None

    def speak(self,speech_sequence):
        spell_mode=False
        language_changed=False
        text_list=[u"<speak>"]
        for item in speech_sequence:
            if isinstance(item,basestring):
                s=escape_text(unicode(item))
                text_list.append((u'<say-as interpret-as="characters">{}</say-as>'.format(s)) if spell_mode else s)
            elif isinstance(item,speech.IndexCommand):
                text_list.append('<mark name="%d"/>' % item.index)
            elif isinstance(item,speech.CharacterModeCommand):
                if item.state:
                    spell_mode=True
                else:
                    spell_mode=False
            elif isinstance(item,speech.LangChangeCommand):
                if language_changed:
                    text_list.append(u"</voice>")
                    language_changed=False
                if not item.lang:
                    continue
                new_language=item.lang.split("_")[0]
                if new_language not in self.__voices_by_language:
                    continue
                elif new_language==self.__language:
                    continue
                text_list.append(u'<voice xml:lang="{}">'.format(new_language))
                language_changed=True
            elif isinstance(item,speech.SpeechCommand):
                log.debugWarning("Unsupported speech command: %s"%item)
            else:
                log.error("Unknown speech: %s"%item)
        if language_changed:
            text_list.append(u"</voice>")
        text_list.append(u"</speak>")
        text=u"".join(text_list)
        task=speak_text(self.__lib,self.__tts_engine,text,self.__cancel_flag)
        if "+" in self.__voice:
            pair=self.__voice.split("+")
            task.set_main_voice(pair[0])
            task.set_extra_voice(pair[1])
        else:
            task.set_main_voice(self.__voice)
        task.set_rate(self.__rate)
        task.set_pitch(self.__pitch)
        task.set_volume(self.__volume)
        self.__tts_queue.put(task)

    def pause(self,switch):
        self.__player.pause(switch)

    def cancel(self):
        try:
            while True:
                self.__tts_queue.get_nowait()
        except Queue.Empty:
            self.__cancel_flag.set()
            self.__tts_queue.put(self.__cancel_flag.clear)
            self.__player.stop()

    def _get_lastIndex(self):
        return self.__mark_callback.index

    def _get_availableVoices(self):
        result=OrderedDict()
        primary_voices=self.__voices_by_language[self.__language]
        for voice in primary_voices:
            result[voice]=VoiceInfo(voice,voice,self.__language)
        for language,voices in self.__voices_by_language.iteritems():
            if language!=self.__language:
                if self.__lib.RHVoice_are_languages_compatible(self.__tts_engine,self.__language,language):
                    for main_voice in primary_voices:
                        for extra_voice in voices:
                            voice="{}+{}".format(main_voice,extra_voice)
                            result[voice]=VoiceInfo(voice,voice,self.__language)
        return result

    def _get_language(self):
        return self.__language

    def _get_rate(self):
        return self.__rate

    def _set_rate(self,rate):
        self.__rate=max(0,min(100,rate))

    def _get_pitch(self):
        return self.__pitch

    def _set_pitch(self,pitch):
        self.__pitch=max(0,min(100,pitch))

    def _get_volume(self):
        return self.__volume

    def _set_volume(self,volume):
        self.__volume=max(0,min(100,volume))

    def _get_voice(self):
        return self.__voice

    def _set_voice(self,voice):
        try:
            self.__voice=self.availableVoices[voice].ID
        except:
            pass
