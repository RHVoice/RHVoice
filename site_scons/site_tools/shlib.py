# Copyright (C) 2010, 2011  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import os.path
import os
from SCons.Script import *

def exists(env):
    return True

def SharedLibraryEx(env,name,source,version):
    if hasattr(os,"uname") and os.uname()[0]=="Linux":
        so_name=env.subst("$SHLIBPREFIX")+name+env.subst("$SHLIBSUFFIX")+"."+version.split(".")[0]
        flags="-Wl,-soname,"+so_name
        return env.SharedLibrary(name,source,LINKFLAGS=flags)
    else:
        return env.SharedLibrary(name,source)

def generate(env):
    env.AddMethod(SharedLibraryEx)
