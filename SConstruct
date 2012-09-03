# Copyright (C) 2010, 2011, 2012  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

import sys
import os
import os.path
import subprocess

def CheckPKGConfig(context):
    context.Message("Checking for pkg-config... ")
    result=context.TryAction("pkg-config --version")[0]
    context.Result(result)
    return result

def CheckPKG(context,name):
    context.Message("Checking for {}... ".format(name))
    result=context.TryAction("pkg-config --exists '{}'".format(name))[0]
    context.Result(result)
    return result

def get_msvc_env_vars():
    var_names=["path","lib","include","tmp"]
    setenv_script=r"C:\Program^ Files\Microsoft^ SDKs\Windows\v7.1\Bin\SetEnv.cmd"
    command='{} /x86 /release && set'.format(setenv_script)
    output=subprocess.check_output(["cmd","/c",command])
    vars=dict()
    lines=output.split("\n")
    for line in lines:
        trimmed_line=line.strip()
        if trimmed_line:
            p=trimmed_line.split("=",1)
            if p[0].lower() in var_names:
                vars[p[0]]=p[1]
    return vars

BUILDDIR=os.path.join("build",sys.platform)
Execute(Mkdir(BUILDDIR))
SConsignFile(os.path.join(BUILDDIR,"scons"))
var_cache=os.path.join(BUILDDIR,"user.conf")
env_args={}
args={"DESTDIR":""}
args.update(ARGUMENTS)
vars=Variables(var_cache,args)
if sys.platform!="win32":
    vars.Add("prefix","Installation prefix","/usr/local")
    vars.Add("bindir","Program installation directory","$prefix/bin")
    vars.Add("libdir","Library installation directory","$prefix/lib")
    vars.Add("includedir","Header installation directory","$prefix/include")
    vars.Add("datadir","Data installation directory","$prefix/share")
    vars.Add("sysconfdir","A directory for configuration files","$prefix/etc")
    vars.Add("servicedir",".service file installation directory","$datadir/dbus-1/services")
    vars.Add("DESTDIR","Support for staged installation","")
    vars.Add(EnumVariable("enable_shared","Build a shared library","yes",["yes","no"],ignorecase=1))
vars.Add("FLAGS","Additional compiler/linker flags")
vars.Add("CCFLAGS","C compiler flags")
vars.Add("LINKFLAGS","Linker flags")
vars.Add(EnumVariable("debug","Build debug variant","no",["yes","no"],ignorecase=1))
vars.Add("package_version","Package version","0.4-a1")
if sys.platform=="win32":
    env_args["tools"]=["msvc","mslink","mslib","newlines"]
    env_args["ENV"]=get_msvc_env_vars()
else:
    env_args["tools"]=["default","textfile","installer"]
env_args["variables"]=vars
env_args["CPPPATH"]=[]
env_args["LIBS"]=[]
env_args["LIBPATH"]=[]
env_args["package_name"]="RHVoice"
env_args["CPPDEFINES"]=[]
env=Environment(**env_args)
Help("Type 'scons' to build the package.\n")
if sys.platform!="win32":
    Help("Then type 'scons install' to install it.\n")
    Help("Type 'scons --clean install' to uninstall the software.\n")
Help("You may use the following configuration variables:\n")
Help(vars.GenerateHelpText(env))
env.Prepend(CPPPATH=(".",os.path.join("#src","include")))
env.Append(CPPDEFINES=("PACKAGE",env.subst(r'\"$package_name\"')))
env.Append(CPPDEFINES=("VERSION",env.subst(r'\"$package_version\"')))
if env["PLATFORM"]=="win32":
    env.Append(CPPDEFINES=("WIN32",1))
    env.Append(CPPDEFINES=("UNICODE",1))
    env.Append(CPPDEFINES=("NOMINMAX",1))
    env.Append(CCFLAGS=["/MT"])
    env.Append(CXXFLAGS=["/EHsc"])
flags=env.get("FLAGS")
if flags:
    env.MergeFlags(flags)
if env.get("debug")=="yes":
    if env["PLATFORM"]=="win32":
        env.AppendUnique(CCFLAGS="/Od",delete_existing=1)
    else:
        env.AppendUnique(CCFLAGS="-O0",delete_existing=1)
        env.AppendUnique(CCFLAGS="-g",delete_existing=1)
else:
    if env["PLATFORM"]=="win32":
        env.PrependUnique(CCFLAGS="/O2")
    else:
        env.PrependUnique(CCFLAGS="-O2")
vars.Save(var_cache,env)
conf=env.Configure(conf_dir=os.path.join(BUILDDIR,"configure_tests"),
                   log_file=os.path.join(BUILDDIR,"configure.log"),
                   custom_tests={"CheckPKGConfig":CheckPKGConfig,"CheckPKG":CheckPKG})
if not conf.CheckCC():
    print "The C compiler is not working"
    exit(1)
if not conf.CheckCXX():
    print "The C++ compiler is not working"
    exit(1)
# has_sox=conf.CheckLibWithHeader("sox","sox.h","C",call='sox_init();',autoadd=0)
# if not has_sox:
#     print "Error: cannot link with libsox"
#     exit(1)
# env.PrependUnique(LIBS="sox")
has_ao=conf.CheckLibWithHeader("ao","ao/ao.h","C",call='ao_initialize();',autoadd=0)
has_giomm=False
if conf.CheckPKGConfig():
    has_giomm=conf.CheckPKG("giomm-2.4")
if env["PLATFORM"]=="win32":
    env.AppendUnique(LIBS="kernel32")
env=conf.Finish()
Export(["env","BUILDDIR"])
src_subdirs=["core","lib","utils"]
if has_ao:
    src_subdirs.append("test")
    if has_giomm:
        src_subdirs.append("service")
if env["PLATFORM"]=="win32":
    src_subdirs.append("sapi")
else:
    src_subdirs.append("include")
for subdir in src_subdirs:
    SConscript(os.path.join("src",subdir,"SConscript"),
               variant_dir=os.path.join(BUILDDIR,subdir),
               duplicate=0)
if env["PLATFORM"]!="win32":
    SConscript(os.path.join("data","SConscript"))
    SConscript(os.path.join("config","SConscript"))
if env["PLATFORM"]=="win32":
    for f in ["README","COPYING","COPYING.LESSER"]:
        env.ConvertNewlines(os.path.join(BUILDDIR,f),f)
    env.ConvertNewlinesB(os.path.join(BUILDDIR,"RHVoice.ini"),os.path.join("config","RHVoice.conf"))
    # env.ConvertNewlinesB(os.path.join(BUILDDIR,"dict.txt"),os.path.join("config","dicts","example.txt"))
