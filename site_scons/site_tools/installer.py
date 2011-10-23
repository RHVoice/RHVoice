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
    if env["PLATFORM"]=="win32":
        return False
    else:
        return True

def InstallProgram(env,src):
    if not env.has_key("BINDESTDIR"):
        env["BINDESTDIR"]=env.subst("$DESTDIR$bindir")
        env.Alias("install",env["BINDESTDIR"])
    inst=env.Install(env["BINDESTDIR"],src)
    env.AddPostAction(inst,Chmod("$TARGET",0755))
    return inst

def InstallData(env,src,subdir=None):
    if not env.has_key("DATADESTDIR"):
        env["DATADESTDIR"]=env.subst("$DESTDIR$datadir")
        env.Alias("install",env["DATADESTDIR"])
        env.Clean(env["DATADESTDIR"],os.path.join(env["DATADESTDIR"],env["package_name"]))
    if subdir:
        dest=os.path.join(env["DATADESTDIR"],env["package_name"],subdir)
    else:
        dest=os.path.join(env["DATADESTDIR"],env["package_name"])
    inst=env.Install(dest,src)
    env.AddPostAction(inst,Chmod("$TARGET",0644))
    return inst

def InstallStaticLibrary(env,src):
    if not env.has_key("LIBDESTDIR"):
        env["LIBDESTDIR"]=env.subst("$DESTDIR$libdir")
        env.Alias("install",env["LIBDESTDIR"])
    inst=env.Install(env["LIBDESTDIR"],src)
    env.AddPostAction(inst,Chmod("$TARGET",0644))
    return inst

def InstallSharedLibrary(env,src,version):
    if not env.has_key("LIBDESTDIR"):
        env["LIBDESTDIR"]=env.subst("$DESTDIR$libdir")
        env.Alias("install",env["LIBDESTDIR"])
    if hasattr(os,"uname") and os.uname()[0]=="Linux":
        name=os.path.join(env["LIBDESTDIR"],os.path.split(str(src[0]))[1])
        inst=env.InstallAs(name+"."+version,src)
        inst+=env.Command(name+"."+version.split(".")[0],inst[0],"ln -s ${SOURCE.file} ${TARGET.file}",chdir=1)
        inst+=env.Command(name,inst[0],"ln -s ${SOURCE.file} ${TARGET.file}",chdir=1)
    else:
        inst=env.Install(env["LIBDESTDIR"],src)
    env.AddPostAction(inst,Chmod("$TARGET",0755))
    return inst

def InstallHeader(env,src):
    if not env.has_key("INCDESTDIR"):
        env["INCDESTDIR"]=env.subst("$DESTDIR$includedir")
        env.Alias("install",env["INCDESTDIR"])
    inst=env.Install(env["INCDESTDIR"],src)
    env.AddPostAction(inst,Chmod("$TARGET",0644))
    return inst

def generate(env):
    env.AddMethod(InstallProgram)
    env.AddMethod(InstallData)
    env.AddMethod(InstallStaticLibrary)
    env.AddMethod(InstallSharedLibrary)
    env.AddMethod(InstallHeader)
