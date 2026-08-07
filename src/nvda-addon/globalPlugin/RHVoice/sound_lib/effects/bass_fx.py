from __future__ import absolute_import
from ..external import pybass_fx
from .effect import SoundEffect

""" Effects from bass_fx """

class Volume(SoundEffect):
    """ """
    effect_type = pybass_fx.BASS_FX_BFX_VOLUME
    struct = pybass_fx.BASS_BFX_VOLUME


class PeakEq(SoundEffect):
    """ """
    effect_type = pybass_fx.BASS_FX_BFX_PEAKEQ
    struct = pybass_fx.BASS_BFX_PEAKEQ


class DAmp(SoundEffect):
    """ """
    effect_type = pybass_fx.BASS_FX_BFX_DAMP
    struct = pybass_fx.BASS_BFX_DAMP
