from __future__ import absolute_import
from ..main import bass_call
import ctypes
from ..external import pybass
import string  # for the alphabet!

class SoundEffect(object):
    """ """
    def __init__(self, channel, type=None, priority=0):
        self.original_channel = channel
        if hasattr(channel, "handle"):
            channel = channel.handle
        if type is None:
            type = self.effect_type
        self.effect_type = type
        self.priority = priority
        self.handle = bass_call(pybass.BASS_ChannelSetFX, channel, type, priority)

    def get_parameters(self):
        """Retrieves the parameters of an effect."""
        res = {}
        params = self.struct()
        bass_call(pybass.BASS_FXGetParameters, self.handle, ctypes.pointer(params))
        for f in params._fields_:
            res[f[0]] = getattr(params, f[0])
        return res

    def set_parameters(self, parameters):
        """

        Args:
          parameters: 

        Returns:

        """
        params = self.struct()
        for p, v in parameters.items():
            setattr(params, p, v)
        bass_call(pybass.BASS_FXSetParameters, self.handle, ctypes.pointer(params))

    def __dir__(self):
        res = dir(self.__class__)
        return res + self._get_pythonic_effect_fields()

    def _get_effect_fields(self):
        """ """
        return [i[0] for i in self.struct._fields_]

    def _get_pythonic_effect_fields(self):
        """ """
        return [
            self._bass_to_python(i)
            for i in self._get_effect_fields()
            if not i.startswith("_")
        ]

    def _bass_to_python(self, func):
        """

        Args:
          func: 

        Returns:

        """
        for c in string.ascii_lowercase:
            func = func.replace(c.upper(), "_%s" % c)
        if func.startswith("_"):
            func = func[1:]
        return func[2:]

    def _python_to_bass(self, func):
        """

        Args:
          func: 

        Returns:

        """
        for c in string.ascii_lowercase:
            func = func.replace("_%s" % c, c.upper())
        func = "%s%s" % (func[0].upper(), func[1:])
        for f in self._get_effect_fields():
            if f.endswith(func):
                func = f
        return func

    def __getattr__(self, attr):
        return self.get_parameters()[self._python_to_bass(attr)]

    def __setattr__(self, attr, val):
        if attr not in self._get_pythonic_effect_fields():
            return super(SoundEffect, self).__setattr__(attr, val)
        params = self.get_parameters()
        key = self._python_to_bass(attr)
        if key not in params:
            raise AttributeError(
                "Unable to set attribute, suspect issue with base name-munging code"
            )
        params[key] = val
        self.set_parameters(params)
