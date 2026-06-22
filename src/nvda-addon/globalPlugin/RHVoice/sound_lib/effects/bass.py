from __future__ import absolute_import
from ..external import pybass
from .effect import SoundEffect

"""Effects built-in to BASS"""

class Chorus(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_CHORUS
    struct = pybass.BASS_DX8_CHORUS


class Echo(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_ECHO
    struct = pybass.BASS_DX8_ECHO


class Compressor(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_COMPRESSOR
    struct = pybass.BASS_DX8_COMPRESSOR


class Reverb(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_REVERB
    struct = pybass.BASS_DX8_REVERB


class Distortion(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_DISTORTION
    struct = pybass.BASS_DX8_DISTORTION


class Flanger(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_FLANGER
    struct = pybass.BASS_DX8_FLANGER


class Gargle(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_GARGLE
    struct = pybass.BASS_DX8_GARGLE


class I3DL2Reverb(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_I3DL2REVERB
    struct = pybass.BASS_DX8_I3DL2REVERB


class ParamEq(SoundEffect):
    """ """
    effect_type = pybass.BASS_FX_DX8_PARAMEQ
    struct = pybass.BASS_DX8_PARAMEQ
