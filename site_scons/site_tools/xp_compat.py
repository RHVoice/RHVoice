# Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
import os.path

if sys.version_info[0]>=3:
    import winreg
else:
    import _winreg as winreg

__sdk_dir=None
__checked=False

def get_sdk_dir():
    global __sdk_dir,__checked
    if __checked:
        return __sdk_dir
    try:
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,r"SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1A",0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
            dir=winreg.QueryValueEx(key,"InstallationFolder")[0]
    except WindowsError:
        return None
    finally:
        __checked=True
    if not os.path.isdir(dir):
        return None
    __sdk_dir=dir
    return __sdk_dir

def get_env_vars(sdk_dir,arch):
    vars={"PATH":"bin","LIB":"lib","INCLUDE":"include"}
    if arch and arch.endswith("64"):
        for name in ["PATH","LIB"]:
            vars[name]=os.path.join(vars[name],"x64")
    for name in vars.keys():
        vars[name]=os.path.join(sdk_dir,vars[name])
    return vars

def exists(env):
    return (get_sdk_dir() is not None)

def generate(env):
    sdk_dir=get_sdk_dir()
    if sdk_dir is None:
        return
    arch=env["TARGET_ARCH"]
    vars=get_env_vars(sdk_dir,arch)
    for name,path in vars.items():
        env.PrependENVPath(name,path,delete_existing=True)
    env.Append(CPPDEFINES="_USING_V110_SDK71_")
    for name in ["WINVER","_WIN32_WINNT"]:
        env.Append(CPPDEFINES=(name,"0x0501"))
    env.Append(CPPDEFINES=("NTDDI_VERSION","0x05010000"))
    env["MS_LINKER_SUBSYSTEM"]="CONSOLE"
    minor_version=(2 if arch.endswith("64") else 1)
    env.Append(LINKFLAGS="/SUBSYSTEM:${{MS_LINKER_SUBSYSTEM}},5.0{}".format(minor_version))
    env["xp_compat_enabled"]=True
