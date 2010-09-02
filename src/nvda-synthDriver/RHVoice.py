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
from ctypes import c_void_p,c_char_p,c_char,c_short,c_int,c_float,POINTER,Structure,sizeof,string_at,CFUNCTYPE,cast

import config
import nvwave
from logHandler import log
from synthDriverHandler import SynthDriver,VoiceInfo

class cst_utterance(Structure):
    _fields_=[("features",c_void_p),
              ("ffunctions",c_void_p),
              ("relations",c_void_p),
              ("ctx",c_void_p)]

class cst_voice(Structure):
    pass

cst_voice._fields_=[("name",c_char_p),
                    ("features",c_void_p),
                    ("ffunctions",c_void_p),
                    ("utt_init",CFUNCTYPE(POINTER(cst_utterance),POINTER(cst_utterance),POINTER(cst_voice)))]

class cst_wave(Structure):
    _fields_=[("type",c_char_p),
              ("sample_rate",c_int),
              ("num_samples",c_int),
              ("num_channels",c_int),
              ("samples",POINTER(c_short))]

CST_AUDIO_STREAM_CONT=0
CST_AUDIO_STREAM_STOP=-1

cst_audio_stream_callback=CFUNCTYPE(c_int,POINTER(cst_wave),c_int,c_int,c_int,c_void_p)
cst_uttfunc=CFUNCTYPE(c_void_p,POINTER(cst_utterance))

class cst_audio_streaming_info(Structure):
    _fields_=[("min_buffsize",c_int),
              ("asc",cst_audio_stream_callback),
              ("userdata",c_void_p)]

module_dir=os.path.dirname(unicode(__file__,sys.getfilesystemencoding()))
lib_path=os.path.join(module_dir,"RHVoice.dll")
userdict_path=os.path.join(module_dir,"RHVoice-userdict.txt")

native_rate_min,native_rate_max=1.9,0.1
native_pitch_min,native_pitch_max=0.5,1.5
native_volume_min,native_volume_max=0.1,1.9

class Voice(object):
    def __init__(self,lib):
        self.__lib=lib
        self._as_parameter_=self.__lib.RHVoice_create_voice(os.path.join(module_dir,"RHVoice-data","voice").encode(sys.getfilesystemencoding()))
        if not self._as_parameter_:
            raise RuntimeError("RHVoice initialization error")

    def __del__(self):
        self.__lib.RHVoice_delete_voice(self._as_parameter_)

    @property
    def features(self):
        return self._as_parameter_.contents.features

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

    def __call__(self,wp,start,size,is_last,user_data):
        try:
            self.index=user_data
            if self.__silence_flag.is_set():
                return CST_AUDIO_STREAM_STOP
            try:
                self.__player.feed(string_at(cast(wp.contents.samples,c_void_p).value+start*sizeof(c_short),size*sizeof(c_short)))
            except:
                log.debugWarning("Error feeding audio to nvWave",exc_info=True)
            if is_last:
                self.__player.idle()
            return CST_AUDIO_STREAM_CONT
        except:
            log.error("RHVoice AudioCallback",exc_info=True)
            return CST_AUDIO_STREAM_STOP

class UttCallback(object):
    def __init__(self,lib,cfunc_audio_callback,silence_flag):
        self.__lib=lib
        self.__cfunc_audio_callback=cfunc_audio_callback
        self.__silence_flag=silence_flag

    def __call__(self,utt):
        try:
            if self.__silence_flag.is_set():
                return None
            asi=self.__lib.new_audio_streaming_info()
            asi.contents.min_buffsize=2400
            asi.contents.asc=self.__cfunc_audio_callback
            asi.contents.userdata=self.index
            self.__lib.feat_set(utt.contents.features,"streaming_info",self.__lib.audio_streaming_info_val(asi))
            self.__lib.feat_set_float(utt.contents.features,"duration_stretch",c_float(self.rate))
            self.__lib.feat_set_float(utt.contents.features,"f0_shift",c_float(self.pitch))
            self.__lib.feat_set_float(utt.contents.features,"volume",c_float(self.volume))
            self.__lib.feat_set_int(utt.contents.features,"pseudo_english",1 if self.variant=="pseudo-english" else 0)
            return cast(utt,c_void_p).value
        except:
            log.error("RHVoice UttCallback",exc_info=True)

class TTSThread(threading.Thread):
    def __init__(self,lib,tts_queue,voice,audio_callback,silence_flag):
        self.__lib=lib
        self.__queue=tts_queue
        self.__voice=voice
        self.__cfunc_audio_callback=cst_audio_stream_callback(audio_callback)
        self.__silence_flag=silence_flag
        self.__utt_callback=UttCallback(self.__lib,self.__cfunc_audio_callback,self.__silence_flag)
        self.__cfunc_utt_callback=cst_uttfunc(self.__utt_callback)
        self.__lib.feat_set(self.__voice.features,"utt_user_callback",self.__lib.uttfunc_val(self.__cfunc_utt_callback))
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
                    self.__utt_callback.index=index
                    self.__utt_callback.rate=rate
                    self.__utt_callback.pitch=pitch
                    self.__utt_callback.volume=volume
                    self.__utt_callback.variant=variant
                    self.__lib.RHVoice_synth_text(text.encode("utf-8"),self.__voice)
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
        self.__lib.RHVoice_create_voice.argtypes=(c_char_p,)
        self.__lib.RHVoice_create_voice.restype=POINTER(cst_voice)
        self.__lib.RHVoice_delete_voice.argtypes=(POINTER(cst_voice),)
        self.__lib.RHVoice_synth_text.argtypes=(c_char_p,POINTER(cst_voice))
        self.__lib.RHVoice_load_user_dict.argtypes=(POINTER(cst_voice),c_char_p)
        self.__lib.new_audio_streaming_info.restype=POINTER(cst_audio_streaming_info)
        self.__lib.audio_streaming_info_val.argtypes=(POINTER(cst_audio_streaming_info),)
        self.__lib.audio_streaming_info_val.restype=c_void_p
        self.__lib.uttfunc_val.argtypes=(cst_uttfunc,)
        self.__lib.uttfunc_val.restype=c_void_p
        self.__lib.feat_set_int.argtypes=(c_void_p,c_char_p,c_int)
        self.__lib.feat_set_float.argtypes=(c_void_p,c_char_p,c_float)
        self.__lib.feat_set_string.argtypes=(c_void_p,c_char_p,c_char_p)
        self.__lib.feat_set.argtypes=(c_void_p,c_char_p,c_void_p)
        self.__voice=Voice(self.__lib)
        if os.path.isfile(userdict_path):
            self.__lib.RHVoice_load_user_dict(self.__voice,userdict_path.encode(sys.getfilesystemencoding()))
        self.__player=nvwave.WavePlayer(channels=1,samplesPerSec=16000,bitsPerSample=16,outputDevice=config.conf["speech"]["outputDevice"])
        self.__silence_flag=threading.Event()
        self.__audio_callback=AudioCallback(self.__lib,self.__player,self.__silence_flag)
        self.__tts_queue=Queue.Queue()
        self.__tts_thread=TTSThread(self.__lib,self.__tts_queue,self.__voice,self.__audio_callback,self.__silence_flag)
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
