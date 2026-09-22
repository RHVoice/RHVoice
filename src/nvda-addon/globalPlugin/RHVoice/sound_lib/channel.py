from __future__ import absolute_import
from .external.pybass import *
from .main import bass_call, bass_call_0, BassError, update_3d_system, FlagObject
from ctypes import pointer, c_float, c_long, c_ulong, c_buffer


class Channel(FlagObject):
    """A "channel" can be a sample playback channel (HCHANNEL), a sample stream (HSTREAM), a MOD music (HMUSIC), or a recording (HRECORD).
    Chances are, if you're playing audio, you're using a channel on one level or another.
    Each "Channel" function can be used with one or more of these channel types.
    """

    attribute_mapping = {}

    def __init__(self, handle):
        self.handle = handle
        self.attribute_mapping = {
            "eaxmix": BASS_ATTRIB_EAXMIX,
            "frequency": BASS_ATTRIB_FREQ,
            "pan": BASS_ATTRIB_PAN,
            "volume": BASS_ATTRIB_VOL,
            "byte": BASS_POS_BYTE,
            "decode": BASS_POS_DECODE,
        }

    def add_attributes_to_mapping(self, **attrs):
        self.attribute_mapping.update(**attrs)

    def play(self, restart=False):
        """Starts (or resumes) playback of a sample, stream, MOD music, or recording.

        Args:
          restart (bool):  Specifies whether playback position should be thrown to the beginning of the stream. Defaults to False

        Returns:
            bool: True on success, False otherwise.
        """
        return bass_call(BASS_ChannelPlay, self.handle, restart)

    def play_blocking(self, restart=False):
        """Starts (or resumes) playback, waiting to return until reaching the end of the stream

        Args:
          restart (bool):  Specifies whether playback position should be thrown to the beginning of the stream. Defaults to False.
        """
        self.play(restart=restart)
        while self.is_playing:
            pass

    def pause(self):
        """Pauses a sample, stream, MOD music, or recording.

        Returns:
            bool: True on success, False otherwise.

        raises:
            sound_lib.main.BassError: If this channel isn't currently playing, already paused, or is a decoding channel and thus not playable.
        """
        return bass_call(BASS_ChannelPause, self.handle)

    def is_active(self):
        """Checks if a sample, stream, or MOD music is active (playing) or stalled. Can also check if a recording is in progress.
        """
        return bass_call_0(BASS_ChannelIsActive, self.handle)

    @property
    def is_playing(self):
        """Checks whether the stream is currently playing or recording.

        Returns:
            bool: True if playing, False otherwise.
        """
        return self.is_active() == BASS_ACTIVE_PLAYING

    @property
    def is_paused(self):
        """
        Checks whether the stream is currently paused.

        Returns:
            bool: True if paused, False otherwise.
        """
        return self.is_active() == BASS_ACTIVE_PAUSED

    @property
    def is_stopped(self):
        """
        Checks whether the stream is currently stopped.

        Returns:
            bool: True if stopped, False otherwise.
        """
        return self.is_active() == BASS_ACTIVE_STOPPED

    @property
    def is_stalled(self):
        """
        Checks whether playback of a stream has been stalled.
        This is due to a lack of sample data. Playback will automatically resume once there is sufficient data to do so.

        Returns:
            bool: True if stalled, False otherwise.
        """
        return self.is_active() == BASS_ACTIVE_STALLED

    def get_position(self, mode=BASS_POS_BYTE):
        """Retrieves the playback position of a sample, stream, or MOD music. Can also be used with a recording channel.

        Args:
          mode (str):  How to retrieve the position. Defaults to "byte".

        Returns:
            int: The current position.

        raises:
            sound_lib.main.BassError: If the requested position is not available.
        """
        return bass_call_0(BASS_ChannelGetPosition, self.handle, mode)

    def set_position(self, pos, mode=BASS_POS_BYTE):
        """Sets the playback position of a sample, MOD music, or stream.

        Args:
          pos (int): The position, in units determined by the mode.
          mode:  (str): How to set the position. Defaults to "byte".

        Returns:
            bool: True if the position was set, False otherwise.

        raises:
            sound_lib.main.BassError: If the stream is not a sound_lib.stream.FileStream or the requested position/mode is not available.
        """
        return bass_call(BASS_ChannelSetPosition, self.handle, pos, mode)

    position = property(get_position, set_position)

    def stop(self):
        """Stops a sample, stream, MOD music, or recording."""
        return bass_call(BASS_ChannelStop, self.handle)

    def update(self, length=0):
        """Updates the playback buffer of a stream or MOD music.

        Args:
          length (int): The amount of data to render, in milliseconds...
              0 = default (2 x update period). This is capped at the space available in the buffer.

        Returns:
            bool: True on success, False otherwise.

        raises:
            sound_lib.main.BassError: If this channel has ended or doesn't have an output -buffer.
        """
        return bass_call(BASS_ChannelUpdate, self.handle, length)

    def get_length(self, mode=BASS_POS_BYTE):
        """Retrieves the playback length of this channel.

        Args:
          mode:  How to retrieve the length. Can take either a flag attribute (string) or bass constent (int). Defaults to "byte".

        Returns:
            int: The channel length on success, -1 on failure.

        raises:
            sound_lib.main.BassError: If the requested mode is not available.
        """
        return bass_call_0(BASS_ChannelGetLength, self.handle, mode)

    __len__ = get_length

    def __nonzero__(self):
        return True

    def get_device(self):
        """Retrieves the device in use by this channel.

        returns:
            int: The device number, -1 on failure.
        """
        return bass_call_0(BASS_ChannelGetDevice, self.handle)

    def set_device(self, device):
        """Changes the device in use by this channel. Must be a stream, MOD music or sample.

        Args:
          device: The device to use... 0 = no sound, 1 = first real output device, BASS_NODEVICE = no device.

        Returns:
            bool: True on success, False otherwise.

        raises:
            sound_lib.main.BassError: If device is invalid, device hasn't been initialized, this channel is already using the requested device, the sample format is not supported by the device/drivers or there is insufficient memory.
        """
        bass_call(BASS_ChannelSetDevice, self.handle, device)

    device = property(get_device, set_device)

    def set_fx(self, type, priority=0):
        """Sets an effect on a stream, MOD music, or recording channel.

        Args:
          type: The type of effect
          priority: The priority of the new FX, which determines its position in the DSP chain. DSP/FX with higher priority are applied before those with lower. This parameter has no effect with DX8 effects when the "with FX flag" DX8 effect implementation is used. Defaults to 0.

        Returns:
            A handle to the new effect on success, False otherwise.

        raises:
            sound_lib.main.BassError: If type is invalid, the specified DX8 effect is unavailable or this channel's format is not supported by the effect.
        """
        from .effects.bass_fx import SoundEffect
        return SoundEffect(bass_call(BASS_ChannelSetFX, type, priority))

    def bytes_to_seconds(self, position=None):
        """Translates a byte position into time (seconds), based on the format in use by this channel.

        Args:
          position:  The position to translate. Defaults to None

        Returns:
            int: The translated length on success, a negative bass error code on failure.
        """
        position = position or self.position
        return bass_call_0(BASS_ChannelBytes2Seconds, self.handle, position)

    def length_in_seconds(self):
        """Retrieves the length of the stream, in seconds, regardless of position.

        returns:
            int: The length on success, a negative bass error code on failure.
        """
        return self.bytes_to_seconds(self.get_length())

    def seconds_to_bytes(self, position):
        """Translates a time (seconds) position into bytes, based on the format in use by this channel.

        Args:
          position:  The position to translate.

        Returns:
            int: The translated length on success, a negative bass error code on failure.
        """
        return bass_call_0(BASS_ChannelSeconds2Bytes, self.handle, position)

    def get_attribute(self, attribute):
        """Retrieves the value of this channel's attribute.

        Args:
          attribute: The attribute to get the value of. Can either be an of type str or int.

        Returns:
            The value on success, None on failure.

        raises:
            sound_lib.main.BassError: If the attribute is either unavailable or invalid.
                Some attributes have additional possible instances where an exception might be raised.
        """
        value = pointer(c_float())
        if attribute in self.attribute_mapping:
            attribute = self.attribute_mapping[attribute]
        bass_call(BASS_ChannelGetAttribute, self.handle, attribute, value)
        return value.contents.value

    def set_attribute(self, attribute, value):
        """Sets the value of an attribute on this channel.

        Args:
          attribute: The attribute to set the value of. Can either be of type str or int.
          value: 

        Returns:
            bool: True on success, False on failure.

        raises:
            sound_lib.main.BassError: If either attribute or value is invalid.
        """
        if attribute in self.attribute_mapping:
            attribute = self.attribute_mapping[attribute]
        return bass_call(BASS_ChannelSetAttribute, self.handle, attribute, value)

    def slide_attribute(self, attribute, value, time):
        """Slides this channel's attribute from its current value to a new value.

        Args:
          attribute: The attribute to slide the value of.
          value: The new attribute value. Consult specific documentation depending on the one in question.
          time: The length of time (in milliseconds) that it should take for the attribute to reach the value.

        Returns:
            bool: True on success, False on failure.

        raises:
            sound_lib.main.BassError: If attribute is invalid, or the attributes value is set to go from positive to negative or vice versa when the BASS_SLIDE_LOG flag is used.
        """
        if attribute in self.attribute_mapping:
            attribute = self.attribute_mapping[attribute]
        return bass_call(
            BASS_ChannelSlideAttribute, self.handle, attribute, value, time * 1000
        )

    def is_sliding(self, attribute=0):
        """Checks if an attribute (or any attribute) of this channel is sliding. Must be a sample, stream, or MOD music.

        Args:
          attribute: The attribute to check for sliding, or0 for any. Defaults to 0.

        Returns:
            bool: True if sliding, False otherwise.

        """
        return bass_call_0(BASS_ChannelIsSliding, self.handle, attribute)

    def get_info(self):
        """Retrieves information on this channel.

        returns:
            A BASS_CHANNELINFO structure.
        """
        value = pointer(BASS_CHANNELINFO())
        bass_call(BASS_ChannelGetInfo, self.handle, value)
        return value[0]

    def get_level(self):
        """Retrieves the level (peak amplitude) of a stream, MOD music or recording channel.

        returns:
            int: -1 on error. If successful, the level of the left channel is returned in the low word (low 16 bits), and the level of the right channel is returned in the high word (high 16 bits).
                If the channel is mono, then the low word is duplicated in the high word. The level ranges linearly from 0 (silent) to 32768 (max).
                0 will be returned when a channel is stalled.        

        raises:
            sound_lib.main.BassError: If this channel is not playing, or this is a decoding channel which has reached the end
        """
        return bass_call_0(BASS_ChannelGetLevel, self.handle)

    def lock(self):
        """Locks a stream, MOD music or recording channel to the current thread.

        returns:
            bool: True on success, False on failure.
        """
        return bass_call(BASS_ChannelLock, self.handle, True)

    def unlock(self):
        """Unlocks a stream, MOD music or recording channel from the current thread.

        returns:
            bool: True on success, False on failure.
        """
        return bass_call(BASS_ChannelLock, self.handle, False)

    def get_3d_attributes(self):
        """Retrieves the 3D attributes of a sample, stream, or MOD music channel with 3D functionality.

        returns:
            dict: A dict containing the stream's 3d attributes

        raises:
            sound_lib.main.BassError: If this channel does not have 3d functionality.
        """
        answer = dict(
            mode=c_ulong(),
            min=c_float(),
            max=c_float(),
            iangle=c_ulong(),
            oangle=c_ulong(),
            outvol=c_float(),
        )
        bass_call(
            BASS_ChannelGet3DAttributes,
            self.handle,
            pointer(answer["mode"]),
            pointer(answer["min"]),
            pointer(answer["max"]),
            pointer(answer["iangle"]),
            pointer(answer["oangle"]),
            pointer(answer["outvol"]),
        )
        res = {}
        for k in answer:
            res[k] = answer[k].value
        return res

    @update_3d_system
    def set_3d_attributes(
        self, mode=-1, min=0.0, max=0.0, iangle=-1, oangle=-1, outvol=-1
    ):
        """Sets the 3D attributes of a sample, stream, or MOD music channel with 3D functionality.

        Args:
          mode:  The 3D processing mode. Defaults to -1.
          min (float):  The minimum distance. The channel's volume is at maximum when the listener is within this distance... 0 or less = leave current. Defaults to 0.0.
          max (float):  The maximum distance. The channel's volume stops decreasing when the listener is beyond this distance... 0 or less = leave current. Defaults to 0.0.
          iangle (int):  The angle of the inside projection cone in degrees... 0 (no cone) to 360 (sphere), -1 = leave current. Defaults to -1.
          oangle (int):  The angle of the inside projection cone in degrees... 0 (no cone) to 360 (sphere), -1 = leave current. Defaults to -1.
          outvol (float):  The delta-volume outside the outer projection cone... 0 (silent) to 1 (same as inside the cone), less than 0 = leave current. Defaults to -1.0.

        Returns:
            bool: True on success, False otherwise.

        raises:
            sound_lib.main.BassError: If this channel does not have 3d functionality, or one or more attribute values are invalid.
        """
        return bass_call(
            BASS_ChannelSet3DAttributes,
            self.handle,
            mode,
            min,
            max,
            iangle,
            oangle,
            outvol,
        )

    def get_3d_position(self):
        """Retrieves the 3D position of a sample, stream, or MOD music channel with 3D functionality.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        answer = dict(
            position=BASS_3DVECTOR(),
            orientation=BASS_3DVECTOR(),
            velocity=BASS_3DVECTOR(),
        )
        bass_call(
            BASS_ChannelGet3DPosition,
            self.handle,
            pointer(answer["position"]),
            pointer(answer["orientation"]),
            pointer(answer["velocity"]),
        )
        return answer

    @update_3d_system
    def set_3d_position(self, position=None, orientation=None, velocity=None):
        """Sets the 3D position of a sample, stream, or MOD music channel with 3D functionality.

        Args:
          position: Defaults to None.
          orientation: Defaults to None.
          velocity: Defaults to None

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        if position:
            position = pointer(position)
        if orientation:
            orientation = pointer(orientation)
        if velocity:
            velocity = pointer(velocity)
        return bass_call(
            BASS_ChannelSet3DPosition, self.handle, position, orientation, velocity
        )

    def set_link(self, handle):
        """Links two MOD music or stream channels together.

        Args:
          handle: The bass handle to link with this one. Can take both a sound_lib.channel or bass handle. Must be HMUSIC or HSTREAM.

        Returns:
            bool: True on success, False on failure.

        raises:
            sound_lib.main.BassError: If handle points to an invalid channel, either one is a decoding channel, or this channel is already linked to handle.
        """
        if isinstance(handle, Channel):
            handle=handle.handle
        bass_call(BASS_ChannelSetLink, self.handle, handle)

    def remove_link(self, handle):
        """Removes a link between two MOD music or stream channels.

        Args:
          handle: The bass handle to unlink with this one. Can take both a sound_lib.channel or bass handle. Must be a HMUSIC or HSTREAM. Must currently be linked.

        Returns:
            bool: True on success, False on failure.
        raises:
            sound_lib.main.BassError: If chan is either not a valid channel, or is not already linked to handle.

        """
        if isinstance(handle, Channel):
            handle=handle.handle
        return bass_call(BASS_ChannelRemoveLink, self.handle, handle)

    def __iadd__(self, other):
        """Convenience method to link this channel to another.  Calls set_link on the passed in item's handle"""
        self.set_link(other.handle)
        return self

    def __isub__(self, other):
        """Convenience method to unlink this channel from another.  Calls remove_link on the passed in item's handle"""
        self.remove_link(other.handle)
        return self

    def get_frequency(self):
        """Retrieves sample frequency (sample rate).

        returns:
            bool: True on success, False on failure.
        """
        return self.get_attribute(BASS_ATTRIB_FREQ)

    def set_frequency(self, frequency):
        """Sets the frequency (sample rate) of this channel.

        Args:
          frequency (float): The sample rate... 0 = original rate (when the channel was created).

        Returns:
            bool: True on success, False on failure.
        """
        self.set_attribute(BASS_ATTRIB_FREQ, frequency)

    frequency = property(fget=get_frequency, fset=set_frequency)

    def get_pan(self):
        """Gets the panning/balance position of this channel."""
        return self.get_attribute(BASS_ATTRIB_PAN)

    def set_pan(self, pan):
        """Sets the panning/balance position of this channel.

        Args:
          pan (float): The pan position... -1 (full left) to +1 (full right), 0 = centre.

        Returns:
            bool: True on success, False on Failure.

        """
        return self.set_attribute(BASS_ATTRIB_PAN, pan)

    pan = property(fget=get_pan, fset=set_pan)

    def get_volume(self):
        """Gets the volume level of a channel.

        returns:
            float: The volume level... 0 = silent, 1.0 = normal, above 1.0 = amplification.
        """
        return self.get_attribute(BASS_ATTRIB_VOL)

    def set_volume(self, volume):
        """sets the volume level of a channel.

        Args:
          volume (float): The volume level... 0 = silent, 1.0 = normal, above 1.0 = amplification.

        Returns:
            True on success, False on failure.
        """
        self.set_attribute(BASS_ATTRIB_VOL, volume)

    volume = property(fget=get_volume, fset=set_volume)

    def get_data(self, length=16384):
        """Retrieves the immediate sample data (or an FFT representation of it) of this channel. Must be a sample channel, stream, MOD music, or recording channel.

        Args:
          length: Number of bytes wanted (up to 268435455 or 0xFFFFFFF). Defaults to 16384.

        Returns:
            The requested bytes.

        raises:
            sound_lib.main.BassError: If this channel has reached the end, or the BASS_DATA_AVAILABLE flag was used and this is a decoding channel.
        """
        buf = c_buffer(length)
        bass_call_0(BASS_ChannelGetData, self.handle, pointer(buf), length)
        return buf

    def get_looping(self):
        """Returns whether this channel is currently setup to loop."""
        return bass_call_0(BASS_ChannelFlags, self.handle, BASS_SAMPLE_LOOP, 0) == 20

    def set_looping(self, looping):
        """Determines whether this channel is setup to loop.

        Args:
          looping: (bool): Specifies whether this channel should loop.
        """
        if looping:
            return bass_call_0(
                BASS_ChannelFlags, self.handle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP
            )
        return bass_call_0(BASS_ChannelFlags, self.handle, 0, BASS_SAMPLE_LOOP)

    looping = property(fget=get_looping, fset=set_looping)

    def free(self):
        """Frees a channel.

        Returns:
            bool: True on success, False on failure.
        """
        return bass_call(BASS_ChannelFree, self.handle)

    def __del__(self):
        try:
            self.free()
        except:
            pass

    def get_x(self):
        """Retrieves this channel's position on the X-axis, if 3d functionality is available.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        return self.get_3d_position()["position"].x

    def set_x(self, val):
        """Sets positioning of this channel on the X-axis, if 3d functionality is available.

        Args:
            val: The coordinate position.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        pos = self.get_3d_position()
        pos["position"].x = val
        self.set_3d_position(**pos)

    x = property(fget=get_x, fset=set_x)

    def get_y(self):
        """Retrieves this channel's position on the Y-axis, if 3d functionality is available.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        return self.get_3d_position()["position"].y

    def set_y(self, val):
        """Sets positioning of this channel on the Y-axis, if 3d functionality is available.

        Args:
          val: The coordinate position.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        pos = self.get_3d_position()
        pos["position"].y = val
        self.set_3d_position(**pos)

    y = property(fget=get_y, fset=set_y)

    def get_z(self):
        """Retrieves this channel's position on the Z-axis, if 3d functionality is available.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        return self.get_3d_position()["position"].z

    def set_z(self, val):
        """Sets positioning of this channel on the Z-axis, if 3d functionality is available.

        Args:
          val: The coordinate position.

        raises:
            sound_lib.main.BassError: If this channel was not initialized with support for 3d functionality.
        """
        pos = self.get_3d_position()
        pos["position"].z = val
        self.set_3d_position(**pos)

    z = property(fget=get_z, fset=set_z)

    def get_attributes(self):
        """Retrieves all values of all attributes from this object and displays them in a dictionary whose keys are determined by this object's attribute_mapping"""
        res = {}
        for k in self.attribute_mapping:
            try:
                res[k] = self.get_attribute(k)
            except BassError:
                pass
        return res
