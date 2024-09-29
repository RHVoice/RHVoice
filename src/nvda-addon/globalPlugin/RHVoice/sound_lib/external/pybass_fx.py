from __future__ import absolute_import
"BASS_FX wrapper by Christopher Toth"""

import ctypes
import os
from . import pybass
from .paths import x86_path, x64_path
from . import libloader

bass_fx_module = libloader.load_library('bass_fx', x86_path=x86_path, x64_path=x64_path)
func_type = libloader.get_functype()


#Error codes returned by BASS_ErrorGetCode
BASS_ERROR_FX_NODECODE = 4000
BASS_ERROR_FX_BPMINUSE = 4001

#Tempo / Reverse / BPM / Beat flag
BASS_FX_FREESOURCE = 0x10000

#BASS_FX Version 
BASS_FX_GetVersion = func_type(ctypes.c_ulong)(('BASS_FX_GetVersion', bass_fx_module))


"""D S P (Digital Signal Processing)"""

"""
Multi-channel order of each channel is as follows:
	 3 channels       left-front, right-front, center.
	 4 channels       left-front, right-front, left-rear/side, right-rear/side.
	 6 channels (5.1) left-front, right-front, center, LFE, left-rear/side, right-rear/side.
	 8 channels (7.1) left-front, right-front, center, LFE, left-rear/side, right-rear/side, left-rear center, right-rear center.
"""

#DSP channels flags
BASS_BFX_CHANALL = -1  #all channels at once (as by default)
BASS_BFX_CHANNONE = 0 #disable an effect for all channels
BASS_BFX_CHAN1 = 1 #left-front channel
BASS_BFX_CHAN2 = 2 #right-front channel
BASS_BFX_CHAN3 = 4 #see above info
BASS_BFX_CHAN4 = 8 #see above info
BASS_BFX_CHAN5 = 16
BASS_BFX_CHAN6 = 32
BASS_BFX_CHAN7 = 64
BASS_BFX_CHAN8 = 128

#DSP effects
(
 BASS_FX_BFX_ROTATE,
 BASS_FX_BFX_ECHO,
 BASS_FX_BFX_FLANGER,
 BASS_FX_BFX_VOLUME,
 BASS_FX_BFX_PEAKEQ,
 BASS_FX_BFX_REVERB,
 BASS_FX_BFX_LPF,
 BASS_FX_BFX_MIX,
 BASS_FX_BFX_DAMP,
 BASS_FX_BFX_AUTOWAH,
 BASS_FX_BFX_ECHO2,
 BASS_FX_BFX_PHASER,
 BASS_FX_BFX_ECHO3,
 BASS_FX_BFX_CHORUS,
 BASS_FX_BFX_APF,
 BASS_FX_BFX_COMPRESSOR,
 BASS_FX_BFX_DISTORTION,
 BASS_FX_BFX_COMPRESSOR2,
 BASS_FX_BFX_VOLUME_ENV,
 BASS_FX_BFX_BQF,
) = range(0x10000, 0x10000+20)

#BiQuad filters
(
 BASS_BFX_BQF_LOWPASS,
 BASS_BFX_BQF_HIGHPASS,
 BASS_BFX_BQF_BANDPASS, #constant 0 dB peak gain
 BASS_BFX_BQF_BANDPASS_Q, #constant skirt gain, peak gain = Q
 BASS_BFX_BQF_NOTCH,
 BASS_BFX_BQF_ALLPASS,
 BASS_BFX_BQF_PEAKINGEQ,
 BASS_BFX_BQF_LOWSHELF,
 BASS_BFX_BQF_HIGHSHELF,
) = range(9)

#Echo
class BASS_BFX_ECHO(ctypes.Structure):
 _fields_ = [
  ('fLevel', ctypes.c_float), #[0....1....n] linear
  ('lDelay', ctypes.c_int), #[1200..30000]
 ]

#Flanger
class BASS_BFX_FLANGER(ctypes.Structure):
 _fields_ = [
  ('fWetDry', ctypes.c_float), #[0....1....n] linear
  ('fSpeed', ctypes.c_float), #[0......0.09]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#volume
class BASS_BFX_VOLUME(ctypes.Structure):
 _fields_ = [
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s or 0 for global volume control
  ('fVolume', ctypes.c_float), #[0....1....n] linear
 ]

#Peaking Equalizer
class BASS_BFX_PEAKEQ(ctypes.Structure):
 _fields_ = [
  ('lBand', ctypes.c_int), #[0...............n] more bands means more memory & cpu usage
  ('fBandwidth', ctypes.c_float), #[0.1...........<10] in octaves - fQ is not in use (Bandwidth has a priority over fQ)
  ('fQ', ctypes.c_float), #[0...............1] the EE kinda definition (linear) (if Bandwidth is not in use)
  ('fCenter', ctypes.c_float), #[1Hz..<info.freq/2] in Hz
  ('fGain', ctypes.c_float), #[-15dB...0...+15dB] in dB
  ('lChannel', ctypes.c_float), #BASS_BFX_CHANxxx flag/s
 ]

#Reverb
class BASS_BFX_REVERB(ctypes.Structure):
 _fields_ = [
  ('fLevel', ctypes.c_float), #[0....1....n] linear
  ('lDelay', ctypes.c_int), #[1200..10000]
 ]

#Low Pass Filter
class BASS_BFX_LPF(ctypes.Structure):
 _fields_ = [
  ('fResonance', ctypes.c_float), #[0.01...........10]
  ('fCutOffFreq', ctypes.c_float), #[1Hz...info.freq/2] cutoff frequency
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Swap, remap and mix
class BASS_BFX_MIX(ctypes.Structure):
 _fields_ = [
  ('lChannel', ctypes.c_int), #an array of channels to mix using BASS_BFX_CHANxxx flag/s (lChannel[0] is left channel...)
 ]

#Dynamic Amplification
class BASS_BFX_DAMP(ctypes.Structure):
 _fields_ = [
  ('fTarget', ctypes.c_float), #target volume level						[0<......1] linear
  ('fQuiet', ctypes.c_float), #quiet  volume level						[0.......1] linear
  ('fRate', ctypes.c_float), #amp adjustment rate						[0.......1] linear
  ('fGain', ctypes.c_float), #amplification level						[0...1...n] linear
  ('fDelay', ctypes.c_float), #delay in seconds before increasing level	[0.......n] linear
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Auto WAH
class BASS_BFX_AUTOWAH(ctypes.Structure):
 _fields_ = [
  ('fDryMix', ctypes.c_float), #dry (unaffected) signal mix				[-2......2]
  ('fWetMix', ctypes.c_float), #wet (affected) signal mix				[-2......2]
  ('fFeedback', ctypes.c_float), #feedback									[-1......1]
  ('fRate', ctypes.c_float), #rate of sweep in cycles per second		[0<....<10]
  ('fRange', ctypes.c_float), #sweep range in octaves					[0<....<10]
  ('fFreq', ctypes.c_float), #base frequency of sweep Hz				[0<...1000]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Echo 2
class BASS_BFX_ECHO2(ctypes.Structure):
 _fields_ = [
  ('fDryMix', ctypes.c_float), #dry (unaffected) signal mix				[-2......2]
  ('fWetMix', ctypes.c_float), #wet (affected) signal mix				[-2......2]
  ('fFeedback', ctypes.c_float), #feedback									[-1......1]
  ('fDelay', ctypes.c_float), #delay sec								[0<......n]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Phaser
class BASS_BFX_PHASER(ctypes.Structure):
 _fields_ = [
  ('fDryMix', ctypes.c_float), #dry (unaffected) signal mix				[-2......2]
  ('fWetMix', ctypes.c_float), #wet (affected) signal mix				[-2......2]
  ('fFeedback', ctypes.c_float), #feedback									[-1......1]
  ('fRate', ctypes.c_float), #rate of sweep in cycles per second		[0<....<10]
  ('fRange', ctypes.c_float), #sweep range in octaves					[0<....<10]
  ('fFreq', ctypes.c_float), #base frequency of sweep					[0<...1000]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Echo 3
class BASS_BFX_ECHO3(ctypes.Structure):
 _fields_ = [
  ('fDryMix', ctypes.c_float), #dry (unaffected) signal mix				[-2......2]
  ('fWetMix', ctypes.c_float), #wet (affected) signal mix				[-2......2]
  ('fDelay', ctypes.c_float), #delay sec								[0<......n]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Chorus
class BASS_BFX_CHORUS(ctypes.Structure):
 _fields_ = [
  ('fDryMix', ctypes.c_float), #dry (unaffected) signal mix				[-2......2]
  ('fWetMix', ctypes.c_float), #wet (affected) signal mix				[-2......2]
  ('fFeedback', ctypes.c_float), #feedback									[-1......1]
  ('fMinSweep', ctypes.c_float), #minimal delay ms							[0<...6000]
  ('fMaxSweep', ctypes.c_float), #maximum delay ms							[0<...6000]
  ('fRate', ctypes.c_float), #rate ms/s								[0<...1000]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#All Pass Filter
class BASS_BFX_APF(ctypes.Structure):
 _fields_ = [
  ('fGain', ctypes.c_float), #reverberation time						[-1=<..<=1]
  ('fDelay', ctypes.c_float), #delay sec								[0<....<=n]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Compressor
class BASS_BFX_COMPRESSOR(ctypes.Structure):
 _fields_ = [
  ('fThreshold', ctypes.c_float), #compressor threshold						[0<=...<=1]
  ('fAttacktime', ctypes.c_float), #attack time ms							[0<.<=1000]
  ('fReleasetime', ctypes.c_float), #release time ms							[0<.<=5000]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Distortion
class BASS_BFX_DISTORTION(ctypes.Structure):
 _fields_ = [
  ('fDrive', ctypes.c_float), #distortion drive							[0<=...<=5]
  ('fDryMix', ctypes.c_float), #dry (unaffected) signal mix				[-5<=..<=5]
  ('fWetMix', ctypes.c_float), #wet (affected) signal mix				[-5<=..<=5]
  ('fFeedback', ctypes.c_float), #feedback									[-1<=..<=1]
  ('fVolume', ctypes.c_float), #distortion volume						[0=<...<=2]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

#Compressor 2
class BASS_BFX_COMPRESSOR2(ctypes.Structure):
 _fields_ = [
  ('fGain', ctypes.c_float), #output gain of signal after compression	[-60....60] in dB
  ('fThreshold', ctypes.c_float), #point at which compression begins		[-60.....0] in dB
  ('fRatio', ctypes.c_float), #compression ratio						[1.......n]
  ('fAttack', ctypes.c_float), #attack time in ms						[0.01.1000]
  ('fRelease', ctypes.c_float), #release time in ms						[0.01.5000]
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

class BASS_BFX_ENV_NODE(ctypes.Structure):
 _fields_ = [
  ('pos', ctypes.c_long), #node position in seconds (1st envelope node must be at position 0)
  ('val', ctypes.c_float), #node value
 ]

#Volume envelope
class BASS_BFX_VOLUME_ENV(ctypes.Structure):
 _fields_ = [
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
  ('lNodeCount', ctypes.c_int), #number of nodes
  ('pNodes', BASS_BFX_ENV_NODE), #the nodes
  ('bFollow', ctypes.c_bool), #follow source position
 ]

#BiQuad Filters

class BASS_BFX_BQF(ctypes.Structure):
 _fields_ = [
  ('lFilter', ctypes.c_int), #BASS_BFX_BQF_xxx filter types
  ('fCenter', ctypes.c_float), #[1Hz..<info.freq/2] Cutoff (central) frequency in Hz
  ('fGain', ctypes.c_float), #[-15dB...0...+15dB] Used only for PEAKINGEQ and Shelving filters in dB
  ('fBandwidth', ctypes.c_float), #[0.1...........<10] Bandwidth in octaves (fQ is not in use (fBandwidth has a priority over fQ))
   #(between -3 dB frequencies for BANDPASS and NOTCH or between midpoint
   #(fGgain/2) gain frequencies for PEAKINGEQ)
  ('fQ', ctypes.c_float), #[0.1.............1] The EE kinda definition (linear) (if fBandwidth is not in use)
  ('fS', ctypes.c_float), #[0.1.............1] A "shelf slope" parameter (linear) (used only with Shelving filters)
   #when fS = 1, the shelf slope is as steep as you can get it and remain monotonically
   #increasing or decreasing gain with frequency.
  ('lChannel', ctypes.c_int), #BASS_BFX_CHANxxx flag/s
 ]

"""TEMPO / PITCH SCALING / SAMPLERATE"""

"""NOTES: 1. Supported only - mono / stereo - channels
2. Enable Tempo supported flags in BASS_FX_TempoCreate and the others to source handle.
tempo attributes (BASS_ChannelSet/GetAttribute)"""

(
 BASS_ATTRIB_TEMPO,
 BASS_ATTRIB_TEMPO_PITCH,
 BASS_ATTRIB_TEMPO_FREQ,
) = range(0x10000, 0x10000+3)

#tempo attributes options
#[option]											[value]

(
 BASS_ATTRIB_TEMPO_OPTION_USE_AA_FILTER, #TRUE (default) / FALSE
 BASS_ATTRIB_TEMPO_OPTION_AA_FILTER_LENGTH, #32 default (8 .. 128 taps)
 BASS_ATTRIB_TEMPO_OPTION_USE_QUICKALGO, #TRUE / FALSE (default)
 BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS, #82 default, 0 = automatic
 BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS, #28 default, 0 = automatic
 BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS, #8  default
 BASS_ATTRIB_TEMPO_OPTION_PREVENT_CLICK, #TRUE / FALSE (default)
) = range(0x10000, 0x10000+7)

#HSTREAM BASS_FXDEF(BASS_FX_TempoCreate)(DWORD chan, DWORD flags);
BASS_FX_TempoCreate = func_type(pybass.HSTREAM, ctypes.c_ulong, ctypes.c_ulong)(('BASS_FX_TempoCreate', bass_fx_module))

#DWORD BASS_FXDEF(BASS_FX_TempoGetSource)(HSTREAM chan);
BASS_FX_TempoGetSource = func_type(ctypes.c_ulong, pybass.HSTREAM)(('BASS_FX_TempoGetSource', bass_fx_module))

#float BASS_FXDEF(BASS_FX_TempoGetRateRatio)(HSTREAM chan);
BASS_FX_TempoGetRateRatio = func_type(ctypes.c_float, pybass.HSTREAM)(('BASS_FX_TempoGetRateRatio', bass_fx_module))

"""R E V E R S E"""
"""NOTES: 1. MODs won't load without BASS_MUSIC_PRESCAN flag.
2. Enable Reverse supported flags in BASS_FX_ReverseCreate and the others to source handle."""

#reverse attribute (BASS_ChannelSet/GetAttribute)
BASS_ATTRIB_REVERSE_DIR = 0x11000

#playback directions
BASS_FX_RVS_REVERSE = -1
BASS_FX_RVS_FORWARD = 1

#HSTREAM BASS_FXDEF(BASS_FX_ReverseCreate)(DWORD chan, float dec_block, DWORD flags);
BASS_FX_ReverseCreate = func_type(pybass.HSTREAM, ctypes.c_ulong, ctypes.c_float, ctypes.c_ulong)(('BASS_FX_ReverseCreate', bass_fx_module))

#DWORD BASS_FXDEF(BASS_FX_ReverseGetSource)(HSTREAM chan);
BASS_FX_ReverseGetSource = func_type(ctypes.c_ulong, pybass.HSTREAM)(('BASS_FX_ReverseGetSource', bass_fx_module))

"""B P M (Beats Per Minute)"""

#bpm flags
BASS_FX_BPM_BKGRND = 1 #if in use, then you can do other processing while detection's in progress. Not available in MacOSX yet. (BPM/Beat)
BASS_FX_BPM_MULT2 = 2 #if in use, then will auto multiply bpm by 2 (if BPM < minBPM*2)

#translation options
(
 BASS_FX_BPM_TRAN_X2, #multiply the original BPM value by 2 (may be called only once & will change the original BPM as well!)
 BASS_FX_BPM_TRAN_2FREQ, #BPM value to Frequency
 BASS_FX_BPM_TRAN_FREQ2, #Frequency to BPM value
 BASS_FX_BPM_TRAN_2PERCENT, #BPM value to Percents
 BASS_FX_BPM_TRAN_PERCENT2	, #Percents to BPM value
) = range(5)

#typedef void (CALLBACK BPMPROCESSPROC)(DWORD chan, float percent);
BPMPROCESSPROC = func_type(None, ctypes.c_float)

#typedef void (CALLBACK BPMPROC)(DWORD chan, float bpm, void *user);
BPMPROC = func_type(None, ctypes.c_long, ctypes.c_float, ctypes.c_void_p)

#float BASS_FXDEF(BASS_FX_BPM_DecodeGet)(DWORD chan, double startSec, double endSec, DWORD minMaxBPM, DWORD flags, BPMPROCESSPROC *proc);
BASS_FX_BPM_DecodeGet = func_type(ctypes.c_float, ctypes.c_ulong, ctypes.c_double, ctypes.c_double, ctypes.c_ulong, ctypes.c_ulong, BPMPROCESSPROC)(('BASS_FX_BPM_DecodeGet', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_CallbackSet)(DWORD handle, BPMPROC *proc, double period, DWORD minMaxBPM, DWORD flags, void *user);
BASS_FX_BPM_CallbackSet = func_type(ctypes.c_bool, ctypes.c_ulong, BPMPROC, ctypes.c_double, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p)(('BASS_FX_BPM_CallbackSet', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_CallbackReset)(DWORD handle);
BASS_FX_BPM_CallbackReset = func_type(ctypes.c_bool, ctypes.c_ulong)(('BASS_FX_BPM_CallbackReset', bass_fx_module))

#float BASS_FXDEF(BASS_FX_BPM_Translate)(DWORD handle, float val2tran, DWORD trans);
BASS_FX_BPM_Translate = func_type(ctypes.c_float, ctypes.c_ulong, ctypes.c_float, ctypes.c_ulong)(('BASS_FX_BPM_Translate', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_Free)(DWORD handle);
BASS_FX_BPM_Free = func_type(ctypes.c_bool, ctypes.c_ulong)(('BASS_FX_BPM_Free', bass_fx_module))

""" Beat """

#typedef void (CALLBACK BPMBEATPROC)(DWORD chan, double beatpos, void *user);
BPMBEATPROC = func_type(None, ctypes.c_ulong, ctypes.c_double, ctypes.c_void_p)

#BOOL BASS_FXDEF(BASS_FX_BPM_BeatCallbackSet)(DWORD handle, BPMBEATPROC *proc, void *user);
BASS_FX_BPM_BeatCallbackSet = func_type(ctypes.c_bool, ctypes.c_ulong, ctypes.c_void_p)(('BASS_FX_BPM_BeatCallbackSet', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_BeatCallbackReset)(DWORD handle);
BASS_FX_BPM_BeatCallbackReset = func_type(ctypes.c_bool, ctypes.c_ulong)(('BASS_FX_BPM_BeatCallbackReset', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_BeatDecodeGet)(DWORD chan, double startSec, double endSec, DWORD flags, BPMBEATPROC *proc, void *user);
BASS_FX_BPM_BeatDecodeGet = func_type(ctypes.c_bool, ctypes.c_ulong, ctypes.c_double, ctypes.c_double, ctypes.c_ulong, BPMBEATPROC, ctypes.c_void_p)(('BASS_FX_BPM_BeatDecodeGet', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_BeatSetParameters)(DWORD handle, float bandwidth, float centerfreq, float beat_rtime);
BASS_FX_BPM_BeatSetParameters = func_type(ctypes.c_bool, ctypes.c_ulong, ctypes.c_float, ctypes.c_float, ctypes.c_float)(('BASS_FX_BPM_BeatSetParameters', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_BeatGetParameters)(DWORD handle, float *bandwidth, float *centerfreq, float *beat_rtime);
BASS_FX_BPM_BeatGetParameters = func_type(ctypes.c_bool, ctypes.c_ulong, ctypes.c_float, ctypes.c_float, ctypes.c_float)(('BASS_FX_BPM_BeatGetParameters', bass_fx_module))

#BOOL BASS_FXDEF(BASS_FX_BPM_BeatFree)(DWORD handle);
BASS_FX_BPM_BeatFree = func_type(ctypes.c_bool, ctypes.c_ulong)(('BASS_FX_BPM_BeatFree', bass_fx_module))
