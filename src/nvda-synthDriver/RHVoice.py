# -*- coding: utf-8 -*-
# Copyright (C) 2010 Olga Yakovleva <yakovleva.o.v@gmail.com>

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
import threading
import ctypes
from ctypes import c_void_p,c_char_p,c_short,c_int,c_float,POINTER,Structure,sizeof,string_at,CFUNCTYPE

import config
import nvwave
from logHandler import log
from synthDriverHandler import SynthDriver,VoiceInfo

class cst_features(Structure):
    pass

class cst_utterance(Structure):
    _fields_=[("features",POINTER(cst_features)),
              ("ffunctions",c_void_p),
              ("relations",c_void_p),
              ("ctx",c_void_p)]

class cst_voice(Structure):
    pass

class cst_item(Structure):
    pass

class cst_val(Structure):
    pass

RHVoice_callback=CFUNCTYPE(c_int,POINTER(c_short),c_int,POINTER(cst_item),c_int)

module_dir=os.path.dirname(unicode(__file__,sys.getfilesystemencoding()))
lib_path=os.path.join(module_dir,"RHVoice.dll")
userdict_path=os.path.join(module_dir,"RHVoice-userdict.txt")

native_rate_min,native_rate_max=1.9,0.1
native_pitch_min,native_pitch_max=0.5,1.5
native_volume_min,native_volume_max=0.1,1.9

class Voice(object):
    def __init__(self,lib,callback):
        self.__lib=lib
        self.__callback=RHVoice_callback(callback)
        self._as_parameter_=self.__lib.RHVoice_create_voice(os.path.join(module_dir,"RHVoice-data","voice").encode(sys.getfilesystemencoding()),self.__callback)
        if not self._as_parameter_:
            raise RuntimeError("RHVoice initialization error")

    def __del__(self):
        self.__lib.RHVoice_delete_voice(self._as_parameter_)

class Features(object):
    def __init__(self,lib):
        self.__lib=lib
        self._as_parameter_=self.__lib.new_features()
        if not self._as_parameter_:
            raise RuntimeError("RHVoice initialization error")

    def __del__(self):
        self.__lib.delete_features(self._as_parameter_)

class AudioCallback(object):
    def __init__(self,lib,player,silence_flag):
        self.__lib=lib
        self.__player=player
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

    def __call__(self,samples,nsamples,seg,pos_in_seg):
        try:
            feats=self.__lib.item_utt(seg).contents.features
            self.index=self.__lib.val_userdata(self.__lib.feat_val(feats,"index"))
            if self.__silence_flag.is_set():
                return 0
            try:
                self.__player.feed(string_at(samples,nsamples*sizeof(c_short)))
            except:
                log.debugWarning("Error feeding audio to nvWave",exc_info=True)
            return 1
        except:
            log.error("RHVoice AudioCallback",exc_info=True)
            return 0

class TTSThread(threading.Thread):
    def __init__(self,lib,tts_queue,voice,player,silence_flag):
        self.__lib=lib
        self.__queue=tts_queue
        self.__voice=voice
        self.__player=player
        self.__silence_flag=silence_flag
        self.__features=Features(self.__lib)
        threading.Thread.__init__(self)
        self.daemon=True

    def run(self):
        while True:
            try:
                msg=self.__queue.get()
                if msg is None:
                    break
                elif not msg:
                    self.__silence_flag.clear()
                elif self.__silence_flag.is_set():
                    pass
                else:
                    text,index,rate,pitch,volume,variant=msg
                    self.__lib.feat_set_float(self.__features,"duration_stretch",c_float(rate))
                    self.__lib.feat_set_float(self.__features,"f0_shift",c_float(pitch))
                    self.__lib.feat_set_float(self.__features,"volume",c_float(volume))
                    self.__lib.feat_set_int(self.__features,"pseudo_english",1 if variant=="pseudo-english" else 0)
                    self.__lib.feat_set(self.__features,"index",self.__lib.userdata_val(index))
                    self.__lib.RHVoice_synth_text(text.encode("utf-8"),self.__voice,self.__features)
                    self.__player.idle()
            except:
                log.error("RHVoice: error while processing a message",exc_info=True)

class SynthDriver(SynthDriver):
    name="RHVoice"
    description="RHVoice"

    supportedSettings=(SynthDriver.RateSetting(),SynthDriver.PitchSetting(),SynthDriver.VolumeSetting(),SynthDriver.VariantSetting())

    @classmethod
    def check(cls):
        return os.path.isfile(lib_path)

    def __init__(self):
        self.__lib=ctypes.CDLL(lib_path.encode(sys.getfilesystemencoding()))
        self.__lib.RHVoice_create_voice.argtypes=(c_char_p,RHVoice_callback)
        self.__lib.RHVoice_create_voice.restype=POINTER(cst_voice)
        self.__lib.RHVoice_delete_voice.argtypes=(POINTER(cst_voice),)
        self.__lib.RHVoice_synth_text.argtypes=(c_char_p,POINTER(cst_voice),POINTER(cst_features))
        self.__lib.RHVoice_load_user_dict.argtypes=(POINTER(cst_voice),c_char_p)
        self.__lib.feat_set_int.argtypes=(POINTER(cst_features),c_char_p,c_int)
        self.__lib.feat_set_float.argtypes=(POINTER(cst_features),c_char_p,c_float)
        self.__lib.feat_set_string.argtypes=(POINTER(cst_features),c_char_p,c_char_p)
        self.__lib.feat_set.argtypes=(POINTER(cst_features),c_char_p,POINTER(cst_val))
        self.__lib.feat_val.argtypes=(POINTER(cst_features),c_char_p)
        self.__lib.feat_val.restype=POINTER(cst_val)
        self.__lib.new_features.restype=POINTER(cst_features)
        self.__lib.delete_features.argtypes=(POINTER(cst_features),)
        self.__lib.userdata_val.argtypes=(c_void_p,)
        self.__lib.userdata_val.restype=POINTER(cst_val)
        self.__lib.val_userdata.argtypes=(POINTER(cst_val),)
        self.__lib.val_userdata.restype=c_void_p
        self.__lib.item_utt.argtypes=(POINTER(cst_item),)
        self.__lib.item_utt.restype=POINTER(cst_utterance)
        self.__player=nvwave.WavePlayer(channels=1,samplesPerSec=16000,bitsPerSample=16,outputDevice=config.conf["speech"]["outputDevice"])
        self.__silence_flag=threading.Event()
        self.__audio_callback=AudioCallback(self.__lib,self.__player,self.__silence_flag)
        self.__voice=Voice(self.__lib,self.__audio_callback)
        if os.path.isfile(userdict_path):
            self.__lib.RHVoice_load_user_dict(self.__voice,userdict_path.encode(sys.getfilesystemencoding()))
        self.__tts_queue=Queue.Queue()
        self.__tts_thread=TTSThread(self.__lib,self.__tts_queue,self.__voice,self.__player,self.__silence_flag)
        self.__native_rate=1.0
        self.__native_pitch=1.0
        self.__native_volume=1.0
        self._availableVariants=[VoiceInfo("pseudo-english",u"псевдо-английский"),VoiceInfo("russian",u"русский")]
        self.__variant="pseudo-english"
        self.__tts_thread.start()

    def terminate(self):
        self.cancel()
        self.__tts_queue.put(None)
        self.__tts_thread.join()
        self.__player.close()

    def speakText(self,text,index=None):
        self.__tts_queue.put((text,index,self.__native_rate,self.__native_pitch,self.__native_volume,self.__variant))

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

    def _get_rate(self):
        return int(round((self.__native_rate -native_rate_min)/(native_rate_max -native_rate_min)*100.0))

    def _set_rate(self,rate):
        self.__native_rate=native_rate_min+float(rate)/100.0*(native_rate_max -native_rate_min)

    def _get_pitch(self):
        return int(round((self.__native_pitch -native_pitch_min)/(native_pitch_max -native_pitch_min)*100.0))

    def _set_pitch(self,pitch):
        self.__native_pitch=native_pitch_min+float(pitch)/100.0*(native_pitch_max -native_pitch_min)

    def _get_volume(self):
        return int(round((self.__native_volume -native_volume_min)/(native_volume_max -native_volume_min)*100.0))

    def _set_volume(self,volume):
        self.__native_volume=native_volume_min+float(volume)/100.0*(native_volume_max -native_volume_min)

    def _get_variant(self):
        return self.__variant

    def _set_variant(self,variant):
        self.__variant=variant

    def loadSettings(self):
        c=config.conf["speech"][self.name]
        c.configspec=self.getConfigSpec()
        config.conf.validate(config.val,copy=True,section=c)
        super(SynthDriver,self).loadSettings()
