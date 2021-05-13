# Copyright (C) 2013, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
from SCons.Script import *

def IsLibraryShared(env):
    if sys.platform=="win32":
        return (env["liblevel"]>1)
    else:
        return (env["enable_shared"] and (env["liblevel"]>0))

def BuildLibrary(env,target,sources):
    if env.IsLibraryShared():
        if sys.platform=="win32":
            return env.SharedLibrary(target,sources,MS_LINKER_SUBSYSTEM="WINDOWS")
        else:
            return env.SharedLibrary(target,sources,SHLIBVERSION=env["libversion"])
    elif env["liblevel"]==0:
        if env.get("enable_shared",False):
            return env.SharedObject(sources)
        else:
            return env.StaticObject(sources)
    else:
        return env.StaticLibrary(target,sources)

def exists(env):
    return True

def generate(env):
    env.AddMethod(BuildLibrary)
    env.AddMethod(IsLibraryShared)
