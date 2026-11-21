from __future__ import absolute_import
import platform
import sys
from .channel import Channel
from .main import bass_call, bass_call_0
from .external.pybass import *

try:
    convert_to_unicode = unicode
except NameError:
    convert_to_unicode = str


class BaseStream(Channel):
    """ """
    def _callback(*args):
        """

        Args:
          *args: 

        Returns:

        """
        # Stub it out as otherwise it'll crash, hard.  Used for stubbing download procs
        return 0

    def free(self):
        """Frees a sample stream's resources, including any sync/DSP/FX it has. """
        return bass_call(BASS_StreamFree, self.handle)

    def get_file_position(self, mode):
        """
        Retrieves the file position/status of a stream.
        
        Args:
          mode: 

        Returns:
            int: The requested file position on success, -1 otherwise.

        raises:
            sound_lib.main.BassError: If the handle is invalid, the stream is not a FileStream, or the requested position is not available.
        """
        return bass_call_0(BASS_StreamGetFilePosition, self.handle, mode)

    def setup_flag_mapping(self):
        """ """
        super(BaseStream, self).setup_flag_mapping()
        self.flag_mapping.update({"unicode": BASS_UNICODE})


class Stream(BaseStream):
    """A sample stream.
    Higher-level streams are used in 90% of cases."""

    def __init__(
        self,
        freq=44100,
        chans=2,
        flags=0,
        proc=None,
        user=None,
        three_d=False,
        autofree=False,
        decode=False,
    ):
        self.proc = STREAMPROC(proc)
        self.setup_flag_mapping()
        flags = flags | self.flags_for(
            three_d=three_d, autofree=autofree, decode=decode
        )
        handle = bass_call(BASS_StreamCreate, freq, chans, flags, self.proc, user)
        super(Stream, self).__init__(handle)


class FileStream(BaseStream):
    """A sample stream that loads from a supported filetype.
    Can load from both disk and memory."""

    def __init__(
        self,
        mem=False,
        file=None,
        offset=0,
        length=0,
        flags=0,
        three_d=False,
        mono=False,
        autofree=False,
        decode=False,
        unicode=True,
    ):
        if platform.system() == "Darwin" and file:
            unicode = False
            file = file.encode(sys.getfilesystemencoding())
        self.setup_flag_mapping()
        flags = flags | self.flags_for(
            three_d=three_d,
            autofree=autofree,
            mono=mono,
            decode=decode,
            unicode=unicode,
        )
        if unicode and isinstance(file, str):
            file = convert_to_unicode(file)
        self.file = file

        handle = bass_call(BASS_StreamCreateFile, mem, file, offset, length, flags)
        super(FileStream, self).__init__(handle)


class URLStream(BaseStream):
    """Creates a sample stream from a file found on the internet.
    Downloaded data can optionally be received through a callback function for further manipulation."""

    def __init__(
        self,
        url="",
        offset=0,
        flags=0,
        downloadproc=None,
        user=None,
        three_d=False,
        autofree=False,
        decode=False,
        unicode=True,
    ):
        if platform.system() in ('Darwin', 'Linux'):
            unicode = False
            url = url.encode(sys.getfilesystemencoding())
        self._downloadproc = downloadproc or self._callback  # we *must hold on to this
        self.downloadproc = DOWNLOADPROC(self._downloadproc)
        self.url = url
        self.setup_flag_mapping()
        flags = flags | self.flags_for(
            three_d=three_d, autofree=autofree, decode=decode, unicode=unicode
        )
        offset = int(offset)
        handle = bass_call(
            BASS_StreamCreateURL, url, offset, flags, self.downloadproc, user
        )
        super(URLStream, self).__init__(handle)


class PushStream(BaseStream):
    """ A stream that receives and plays raw audio data in realtime."""

    def __init__(
        self,
        freq=44100,
        chans=2,
        flags=0,
        user=None,
        three_d=False,
        autofree=False,
        decode=False,
    ):
        self.proc = STREAMPROC_PUSH
        self.setup_flag_mapping()
        flags = flags | self.flags_for(
            three_d=three_d, autofree=autofree, decode=decode
        )
        handle = bass_call(BASS_StreamCreate, freq, chans, flags, self.proc, user)
        super(PushStream, self).__init__(handle)

    def push(self, data):
        """
        Adds sample data to the stream.

        Args:
          data (bytes): Data to be sent.

        Returns:
            int: The amount of queued data on success, -1 otherwise.

        raises:
            sound_lib.main.BassError: If the stream has ended or there is insufficient memory.
        """
        return bass_call_0(BASS_StreamPutData, self.handle, data, len(data))
