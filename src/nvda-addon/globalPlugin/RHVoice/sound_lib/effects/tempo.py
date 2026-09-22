from __future__ import absolute_import
import ctypes
from ..external import pybass, pybass_fx
from ..stream import BaseStream
from ..channel import Channel
from ..main import bass_call, bass_call_0

class Tempo(BaseStream):
    """ """
    def __init__(
        self,
        channel,
        flags=0,
        loop=False,
        software=False,
        three_d=False,
        sample_fx=False,
        autofree=False,
        decode=False,
        free_source=False,
    ):
        flags = flags | self.flags_for(
            loop=False,
            software=False,
            three_d=False,
            sample_fx=False,
            autofree=False,
            decode=False,
            free_source=False,
        )
        self.channel = channel
        if isinstance(channel, Channel):
            channel = channel.handle
        handle = bass_call(pybass_fx.BASS_FX_TempoCreate, channel, flags)
        super(Tempo, self).__init__(handle)
        self.add_attributes_to_mapping(
            tempo=pybass_fx.BASS_ATTRIB_TEMPO,
            tempo_pitch=pybass_fx.BASS_ATTRIB_TEMPO_PITCH,
            tempo_freq=pybass_fx.BASS_ATTRIB_TEMPO_FREQ,
        )

    @property
    def tempo(self):
        """The tempo of a channel."""
        return self.get_attribute(pybass_fx.BASS_ATTRIB_TEMPO)

    @tempo.setter
    def tempo(self, val):
        """

        Args:
          val: 

        Returns:

        """
        self.set_attribute("tempo", val)

    @property
    def tempo_pitch(self):
        """ """
        return self.get_attribute("tempo_pitch")

    @tempo_pitch.setter
    def tempo_pitch(self, val):
        """

        Args:
          val: 

        Returns:

        """
        self.set_attribute("tempo_pitch", val)

    @property
    def tempo_freq(self):
        """ """
        return self.get_attribute("tempo_freq")

    @tempo_freq.setter
    def tempo_freq(self, val):
        """

        Args:
          val: 

        Returns:

        """
        self.set_attribute("tempo_freq", val)

    def setup_flag_mapping(self):
        """ """
        super(Tempo, self).setup_flag_mapping()
        self.flag_mapping.update({"free_source": pybass_fx.BASS_FX_FREESOURCE})

    def get_source(self):
        """ """
        source = pybass_fx.BASS_FX_TempoGetSource(self.handle)
        if source == self.channel.handle:
            source = self.channel
        return source

    source = property(fget=get_source)

    def get_rate_ratio(self):
        """ """
        return bass_call(pybass_fx.BASS_FX_TempoGetRateRatio, self.handle)

    rate_ratio = property(fget=get_rate_ratio)
