from __future__ import absolute_import
from .external import pybass
from .channel import Channel


class Music(Channel):
    """ """
    def __init__(self, mem=False, file=None, offset=0, length=0, flags=0, freq=0):
        handle = BASS_MusicLoad(mem, file, offset, length, flags, freq)
        super(Music, self).__init__(handle)
        self.add_attributes_to_mapping(
            music_amplify=pybass.BASS_ATTRIB_MUSIC_AMPLIFY,
            music_bpm=BASS_ATTRIB_MUSIC_BPM,
            music_pansep=BASS_ATTRIB_MUSIC_PANSEP,
            music_speed=BASS_ATTRIB_MUSIC_SPEED,
            music_vol_chan=BASS_ATTRIB_MUSIC_VOL_CHAN,
            music_vol_global=BASS_ATTRIB_MUSIC_VOL_GLOBAL,
            music_vol_inst=BASS_ATTRIB_MUSIC_VOL_INST,
        )
