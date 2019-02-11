# Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com>
# Copyright (C) 2019  Beqa Gozalishvili <beqaprogger@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys
import os.path
import shutil

import config

def onInstall():
    try:
        module_dir=os.path.dirname(__file__.decode("mbcs"))
    except AttributeError:
        module_dir=os.path.dirname(__file__)
    config_dir=os.path.join(module_dir,"synthDrivers","RHVoice","config")
    user_config_dir=os.path.join(config.getUserDefaultConfigPath(),"RHVoice-config")
    if not os.path.isdir(user_config_dir):
        shutil.move(config_dir,user_config_dir)
    else:
        shutil.rmtree(config_dir, ignore_errors=False, onerror=None)
