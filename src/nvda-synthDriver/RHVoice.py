# -*- coding: utf-8 -*-
# Copyright (C) 2010, 2011, 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import os
import sys

from ctypes import POINTER, Structure


# --- bindings ---

class RHVoice_tts_engine_struct(Structure):
    pass
RHVoice_tts_engine=POINTER(RHVoice_tts_engine_struct)

class RHVoice_message_struct(Structure):
    pass
RHVoice_message=POINTER(RHVoice_message_struct)


# --- main code ---

try:
	module_dir=os.path.dirname(__file__.decode("mbcs"))
except AttributeError:
	module_dir=os.path.dirname(__file__)
RHVoice_lib_path=os.path.join(module_dir,"RHVoice.dll")
