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

def Install(env,src,instpath,instname=None,sysdir=True,mode=644):
    destpath=env.subst("$DESTDIR"+instpath)
    env.Alias("install",destpath)
    if instname:
        inst=env.InstallAs(os.path.join(destpath,instname),src)
    else:
        inst=env.Install(destpath,src)
    env.AddPostAction(inst,Chmod("$TARGET",mode))
    return inst

def InstallProgram(env,src):
    return Install(env,src,"$bindir",mode=0755)

def InstallData(env,src,dest=None):
    if dest:
        inst=Install(env,src,os.path.join("$datadir","$package_name",dest))
    else:
        inst=Install(env,src,os.path.join("$datadir","$package_name"))
    env.Clean("install",env.subst("$DESTDIR$datadir/$package_name"))
    return inst

def InstallConfig(env,src,dest=None):
    if dest:
        inst=Install(env,src,os.path.join("$sysconfdir","$package_name",dest))
    else:
        inst=Install(env,src,os.path.join("$sysconfdir","$package_name"))
    env.Clean("install",env.subst("$DESTDIR$sysconfdir/$package_name"))
    return inst

def InstallStaticLibrary(env,src):
    return Install(env,src,"$libdir")

def InstallSharedLibrary(env,src,version):
    if hasattr(os,"uname") and os.uname()[0]=="Linux":
        name=os.path.split(str(src[0]))[1]
        inst=Install(env,src,"$libdir",instname=name+"."+version,mode=0755)
        libpath=os.path.split(inst[0].path)[0]
        for s in [name+"."+version.split(".")[0],name]:
            inst+=env.Command(os.path.join(libpath,s),inst[0],"ln -s ${SOURCE.file} ${TARGET.file}",chdir=1)
            env.AddPostAction(inst[-1],Chmod("$TARGET",0755))
    else:
        inst=Install(env,src,"$libdir",mode=0755)
    return inst

def InstallHeader(env,src):
    return Install(env,src,"$includedir")

def generate(env):
    env.AddMethod(InstallProgram)
    env.AddMethod(InstallData)
    env.AddMethod(InstallConfig)
    env.AddMethod(InstallStaticLibrary)
    env.AddMethod(InstallSharedLibrary)
    env.AddMethod(InstallHeader)
