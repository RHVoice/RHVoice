# -*- coding: utf-8 -*-
# Copyright (C) 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
from collections import OrderedDict
import threading
import ctypes
from ctypes import c_char_p,c_wchar_p,c_short,c_int,c_float,POINTER,Structure,Union,sizeof,string_at,CFUNCTYPE

import config
import nvwave
from logHandler import log
from synthDriverHandler import SynthDriver,VoiceInfo
import speech

module_dir=os.path.join(config.getUserDefaultConfigPath(),"synthDrivers")
lib_path=os.path.join(module_dir,"RHVoice.dll")
data_path=os.path.join(module_dir,"RHVoice-data")
cfg_path=os.path.join(module_dir,"RHVoice-config")

class RHVoice_message_s(Structure):
    pass

RHVoice_message=POINTER(RHVoice_message_s)

RHVoice_event_word_start=0
RHVoice_event_word_end=1
RHVoice_event_sentence_start=2
RHVoice_event_sentence_end=3
RHVoice_event_mark=4

class RHVoice_event_id(Union):
    _fields_=[("number",c_int),("name",c_char_p)]

class RHVoice_event(Structure):
    _fields_=[("message",RHVoice_message),
              ("type",c_int),
              ("text_position",c_int),
              ("text_length",c_int),
              ("audio_position",c_int),
              ("id",RHVoice_event_id)]

RHVoice_callback=CFUNCTYPE(c_int,POINTER(c_short),c_int,POINTER(RHVoice_event),c_int,RHVoice_message)

class Message(object):
    def __init__(self,lib,text):
        self.__lib=lib
        self._as_parameter_=self.__lib.RHVoice_new_message_utf16(text,len(text),1)
        if not self._as_parameter_:
            raise RuntimeError("RHVoice: message creation error")

    def __del__(self):
        self.__lib.RHVoice_delete_message(self._as_parameter_)

    def speak(self):
        self.__lib.RHVoice_speak(self._as_parameter_)

class AudioCallback(object):
    def __init__(self,lib,silence_flag):
        self.__lib=lib
        self.__player=None
        self.__silence_flag=silence_flag
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

    def set_player(self,player):
        self.__player=player

    def __call__(self,samples,num_samples,events,num_events,message):
        try:
            if num_events > 0:
                for i in xrange(num_events):
                    if events[i].type==RHVoice_event_mark:
                        self.index=int(events[i].id.name)
                        break
            if self.__silence_flag.is_set():
                return 0
            try:
                if samples:
                    self.__player.feed(string_at(samples,num_samples*sizeof(c_short)))
            except:
                log.debugWarning("Error feeding audio to nvWave",exc_info=True)
            return 1
        except:
            log.error("RHVoice AudioCallback",exc_info=True)
            return 0

class TTSThread(threading.Thread):
    def __init__(self,lib,tts_queue,player,silence_flag):
        self.__lib=lib
        self.__queue=tts_queue
        self.__player=player
        self.__silence_flag=silence_flag
        threading.Thread.__init__(self)
        self.daemon=True

    def run(self):
        while True:
            try:
                text=self.__queue.get()
                if text is None:
                    break
                elif text is ():
                    self.__silence_flag.clear()
                elif self.__silence_flag.is_set():
                    pass
                else:
                    msg=Message(self.__lib,text)
                    msg.speak()
                    self.__player.idle()
            except:
                log.error("RHVoice: error while processing a message",exc_info=True)

def convert_to_native_percent(val,nmin,nmax,ndef):
    if val>=50:
        nval=ndef+(float(val)-50.0)/50.0*(nmax-ndef)
    else:
        nval=nmin+float(val)/50.0*(ndef-nmin)
    return nval/ndef*100.0

class SynthDriver(SynthDriver):
    name="RHVoice"
    description="RHVoice"

    supportedSettings=(SynthDriver.RateSetting(),SynthDriver.PitchSetting(),SynthDriver.VolumeSetting(),SynthDriver.VoiceSetting(),SynthDriver.VariantSetting())

    @classmethod
    def check(cls):
        return os.path.isfile(lib_path)

    def __init__(self):
        self.__lib=ctypes.CDLL(lib_path.encode(sys.getfilesystemencoding()))
        self.__lib.RHVoice_initialize.argtypes=(c_char_p,RHVoice_callback,c_char_p)
        self.__lib.RHVoice_initialize.restype=c_int
        self.__lib.RHVoice_new_message_utf16.argtypes=(c_wchar_p,c_int,c_int)
        self.__lib.RHVoice_new_message_utf16.restype=RHVoice_message
        self.__lib.RHVoice_delete_message.argtypes=(RHVoice_message,)
        self.__lib.RHVoice_speak.argtypes=(RHVoice_message,)
        self.__lib.RHVoice_get_min_rate.restype=c_float
        self.__lib.RHVoice_get_rate.restype=c_float
        self.__lib.RHVoice_get_max_rate.restype=c_float
        self.__lib.RHVoice_get_min_pitch.restype=c_float
        self.__lib.RHVoice_get_pitch.restype=c_float
        self.__lib.RHVoice_get_max_pitch.restype=c_float
        self.__lib.RHVoice_get_volume.restype=c_float
        self.__lib.RHVoice_get_max_volume.restype=c_float
        self.__lib.RHVoice_get_voice_count.restype=c_int
        self.__lib.RHVoice_get_variant_count.restype=c_int
        self.__lib.RHVoice_get_voice_name.argtypes=(c_int,)
        self.__lib.RHVoice_get_voice_name.restype=c_char_p
        self.__lib.RHVoice_get_variant_name.argtypes=(c_int,)
        self.__lib.RHVoice_get_variant_name.restype=c_char_p
        self.__lib.RHVoice_find_voice.argtypes=(c_char_p,)
        self.__lib.RHVoice_find_voice.restype=c_int
        self.__lib.RHVoice_find_variant.argtypes=(c_char_p,)
        self.__lib.RHVoice_find_variant.restype=c_int
        self.__lib.RHVoice_get_voice.restype=c_int
        self.__lib.RHVoice_get_variant.restype=c_int
        self.__lib.RHVoice_set_voice.argtypes=(c_int,)
        self.__lib.RHVoice_set_variant.argtypes=(c_int,)
        self.__lib.RHVoice_get_version.restype=c_char_p
        self.__silence_flag=threading.Event()
        self.__audio_callback=AudioCallback(self.__lib,self.__silence_flag)
        self.__audio_callback_wrapper=RHVoice_callback(self.__audio_callback)
        sample_rate=self.__lib.RHVoice_initialize(data_path.encode("UTF-8"),self.__audio_callback_wrapper,cfg_path.encode("UTF-8"))
        if sample_rate==0:
            raise RuntimeError("RHVoice: initialization error")
        voice_count=self.__lib.RHVoice_get_voice_count()
        if voice_count==0:
            raise RuntimeError("RHVoice: initialization error")
        self.__player=nvwave.WavePlayer(channels=1,samplesPerSec=sample_rate,bitsPerSample=16,outputDevice=config.conf["speech"]["outputDevice"])
        self.__audio_callback.set_player(self.__player)
        self.__tts_queue=Queue.Queue()
        self.__tts_thread=TTSThread(self.__lib,self.__tts_queue,self.__player,self.__silence_flag)
        self._availableVoices=OrderedDict()
        for id in range(1,voice_count+1):
            name=self.__lib.RHVoice_get_voice_name(id)
            self._availableVoices[name]=VoiceInfo(name,name,"ru")
        self.__lib.RHVoice_set_voice(1)
        self.__voice=self.__lib.RHVoice_get_voice_name(1)
        variant_count=self.__lib.RHVoice_get_variant_count()
        self._availableVariants=OrderedDict()
        for id in range(1,variant_count+1):
            name=self.__lib.RHVoice_get_variant_name(id)
            self._availableVariants[name]=VoiceInfo(name,name,"ru")
        self.__lib.RHVoice_set_variant(1)
        self.__variant=self.__lib.RHVoice_get_variant_name(1)
        self.__rate=50
        self.__pitch=50
        self.__volume=50
        self.__native_rate_range=(self.__lib.RHVoice_get_min_rate(),self.__lib.RHVoice_get_max_rate(),self.__lib.RHVoice_get_rate())
        self.__native_pitch_range=(self.__lib.RHVoice_get_min_pitch(),self.__lib.RHVoice_get_max_pitch(),self.__lib.RHVoice_get_pitch())
        self.__native_volume_range=(0,self.__lib.RHVoice_get_max_volume(),self.__lib.RHVoice_get_volume())
        self.__char_mapping={}
        for c in range(9):
            self.__char_mapping[c]=32
            self.__char_mapping[11]=32
            self.__char_mapping[12]=32
            for c in range(14,32):
                self.__char_mapping[c]=32
                self.__char_mapping[ord("<")]=u"&lt;"
                self.__char_mapping[ord("&")]=u"&amp;"
        self.__tts_thread.start()
        log.info("Using RHVoice version %s" % self.__lib.RHVoice_get_version())

    def terminate(self):
        self.cancel()
        self.__tts_queue.put(None)
        self.__tts_thread.join()
        self.__player.close()
        self.__lib.RHVoice_terminate()

    def speak(self,speech_sequence):
        spell_mode=False
        text_list=[]
        for item in speech_sequence:
            if isinstance(item,basestring):
                s=unicode(item).translate(self.__char_mapping)
                text_list.append((u'<say-as interpret-as="characters">%s</say-as>' % s) if spell_mode else s)
            elif isinstance(item,speech.IndexCommand):
                text_list.append('<mark name="%d"/>' % item.index)
            elif isinstance(item,speech.CharacterModeCommand):
                if item.state:
                    spell_mode=True
                else:
                    spell_mode=False
            elif isinstance(item,speech.SpeechCommand):
                log.debugWarning("Unsupported speech command: %s"%item)
            else:
                log.error("Unknown speech: %s"%item)
        text=u"".join(text_list)
        fmt_str=u'<speak><voice name="%s" variant="%d"><prosody rate="%f%%" pitch="%f%%" volume="%f%%">%s</prosody></voice></speak>'
        variant=self.__lib.RHVoice_find_variant(self.__variant)
        if variant==0:
            variant=1
        rate=convert_to_native_percent(self.__rate,*self.__native_rate_range)
        pitch=convert_to_native_percent(self.__pitch,*self.__native_pitch_range)
        volume=convert_to_native_percent(self.__volume,*self.__native_volume_range)
        ssml=fmt_str % (self.__voice,variant,rate,pitch,volume,text)
        self.__tts_queue.put(ssml)

    def pause(self,switch):
        self.__player.pause(switch)

    def cancel(self):
        try:
            while True:
                self.__tts_queue.get_nowait()
        except Queue.Empty:
            self.__silence_flag.set()
            self.__tts_queue.put(())
            self.__player.stop()

    def _get_lastIndex(self):
        return self.__audio_callback.index

    def _get_language(self):
        return "ru"

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
            self.__voice=self._availableVoices[voice].ID
        except:
            pass

    def _get_variant(self):
        return self.__variant

    def _set_variant(self,variant):
        try:
            self.__variant=self._availableVariants[variant].ID
        except:
            pass
