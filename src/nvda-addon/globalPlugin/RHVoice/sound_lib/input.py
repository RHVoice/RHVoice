from ctypes import string_at
import platform
import wave

from .external.pybass import *

if platform.system() == "Windows":
    from .external.pybasswasapi import *
from . import config
from .main import bass_call, bass_call_0


class Input(object):
    """Provides initialization and management for recording on a global level.
    Initialization is required if wanting to grab audio from an input device.
    """

    def __init__(self, device=-1):
        bass_call(BASS_RecordInit, device)
        self._device = device
        self.config = config.BassConfig()

    def free(self):
        """Frees all resources used by the recording device."""
        return bass_call(BASS_RecordFree)

    def get_device(self):
        """
        Retrieves the device used for recording.

        returns:
            int: The device index on success, -1 on failure.
        """
        return bass_call_0(BASS_RecordGetDevice)

    def set_device(self, device):
        """
        Sets the device to use for recording.

        Args:
          device (int): Device to use... 0 = first.
        """
        if device == self._device:
            return
        self.free()
        self.__init__(device=device)

    device = property(fget=get_device, fset=set_device)

    @staticmethod
    def get_device_names():
        """Convenience method that returns a list of device names that are considered
        	valid by bass.

        Returns:
            list: A list containing names of input devices on the system.
        """
        result = ["Default"]
        info = BASS_DEVICEINFO()
        count = 0
        while BASS_RecordGetDeviceInfo(count, ctypes.byref(info)):
            if info.flags & BASS_DEVICE_ENABLED:
                retrieved = info.name
                if platform.system() == "Windows":
                    retrieved = retrieved.decode("mbcs")
                elif platform.system() == "Darwin":
                    retrieved = retrieved.decode("utf-8")
                retrieved = retrieved.replace("(", "").replace(")", "").strip()
                result.append(retrieved)
            count += 1
        return result

    def find_device_by_name(self, name):
        """
        Attempts to locate an input device given it's name.

        Args:
          name (str): Name of the device to search for.

        Returns:
            int: Index of the device if found.

        raises:
            ValueError: If the provided name was not found among input devices.
        """
        return self.get_device_names().index(name) - 1

    def find_default_device(self):
        """Returns the index of the default device."""
        return -1

    def find_user_provided_device(self, device_name):
        """Locate an input device given it's name, falling back to the default if not found.

        Args:
          device_name (str): Name of the device to search for.

        Returns:
            int: Index of the requested device on success, default device on failure.

        """
        try:
            return self.find_device_by_name(device_name)
        except ValueError:
            return self.find_default_device()


class WASAPIInput(object):
    """Provides the ability to use WASAPI (windows audio session API) input.
    Supported on windows versions above vista."""

    def __init__(
        self,
        device=-2,
        frequency=0,
        channels=0,
        flags=0,
        buffer=0.0,
        period=0.0,
        callback=None,
    ):
        if callback is None:
            callback = lambda buffer, length, user: True
        self.proc = WASAPIPROC(callback)
        bass_call(
            BASS_WASAPI_Init,
            device,
            frequency,
            channels,
            flags,
            buffer,
            period,
            self.proc,
            None,
        )

    def free(self):
        """Frees all resources used by the recording device."""
        bass_call(BASS_WASAPI_Free)

    def set_device(self, device):
        """
        Sets the device to use for recording.

        Args:
          device (int): Device to use... 0 = first.
        """
        bass_call(BASS_WASAPI_SetDevice, device)

    def get_device(self):
        """
        Retrieves the device used for recording.

        returns:
            int: The device index on success, -1 on failure.
        """
        return bass_call_0(BASS_WASAPI_GetDevice)

    device = property(fget=get_device, fset=set_device)

    def start(self):
        """Starts the device.

        returns:
            bool: True on success, False otherwise.
        """
        return bass_call(BASS_WASAPI_Start)

    def stop(self, reset=False):
        """
        Stops the device.

        Args:
          reset:  (Default value = False): Flush the device buffer?

        Returns:
            bool: True on success, False otherwise.
        """
        return bass_call(BASS_WASAPI_Stop, reset)
