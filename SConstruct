# Copyright (C) 2010, 2011, 2012, 2013, 2014  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
import platform
import datetime
if sys.platform=="win32":
    import _winreg

def get_version(is_release):
    next_version="0.5.1"
    if is_release:
        return next_version
    else:
        date=datetime.date.today()
        return "{}-pre-{}{:02}{:02}".format(next_version,date.year,date.month,date.day)

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

def CheckVS(context):
    context.Message("Checking for Visual Studio 2013... ")
    result=1
    try:
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,r"SOFTWARE\Microsoft\VisualStudio\12.0\Setup\VC",_winreg.KEY_READ|_winreg.KEY_WOW64_32KEY) as key:
            context.env["VCDir"]=_winreg.QueryValueEx(key,"ProductDir")[0]
    except WindowsError:
        result=0
    context.Result(result)
    return result

def CheckNSIS(context,unicode_nsis=False):
    result=1
    if unicode_nsis:
        context.Message("Checking for Unicode NSIS... ")
    else:
        context.Message("Checking for NSIS... ")
    key_name=r"SOFTWARE\NSIS"+(r"\Unicode" if unicode_nsis else "")
    try:
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,key_name) as key:
            context.env["makensis"]=File(os.path.join(_winreg.QueryValueEx(key,None)[0],"makensis.exe"))
    except WindowsError:
        result=0
    context.Result(result)
    return result

def get_msvc_env_vars(env,arch):
    var_names=["path","lib","libpath","include","tmp"]
    setenv_script=os.path.join(env["VCDir"],"vcvarsall.bat")
    output=subprocess.check_output(["cmd","/e:on","/v:on","/c",setenv_script,arch,"&&","set"])
    vars=dict()
    lines=output.split("\n")
    for line in lines:
        trimmed_line=line.strip()
        if trimmed_line:
            p=trimmed_line.split("=",1)
            if p[0].lower() in var_names:
                vars[p[0]]=p[1]
    return vars

def convert_flags(value):
    return value.split()

def convert_path(value):
    return value.split(";" if sys.platform=="win32" else ":")

def setup():
    if sys.platform=="win32":
        SetOption("warn","no-visual-c-missing")
    global BUILDDIR,var_cache
    system=platform.system().lower()
    BUILDDIR=os.path.join("build",system)
    var_cache=os.path.join(BUILDDIR,"user.conf")
    Execute(Mkdir(BUILDDIR))
    SConsignFile(os.path.join(BUILDDIR,"scons"))

def create_user_vars():
    args={"DESTDIR":""}
    args.update(ARGUMENTS)
    vars=Variables(var_cache,args)
    vars.Add(BoolVariable("release","Whether we are building a release",True))
    if sys.platform=="win32":
        vars.Add(BoolVariable("enable_x64","Additionally build 64-bit versions of all the libraries",True))
    else:
        vars.Add("prefix","Installation prefix","/usr/local")
        vars.Add("bindir","Program installation directory","$prefix/bin")
        vars.Add("libdir","Library installation directory","$prefix/lib")
        vars.Add("includedir","Header installation directory","$prefix/include")
        vars.Add("datadir","Data installation directory","$prefix/share")
        vars.Add("sysconfdir","A directory for configuration files","$prefix/etc")
        vars.Add("servicedir",".service file installation directory","$datadir/dbus-1/services")
        vars.Add("DESTDIR","Support for staged installation","")
        vars.Add(BoolVariable("enable_shared","Build a shared library",True))
    if sys.platform=="win32":
        suffixes=["32","64"]
    else:
        suffixes=[""]
    for suffix in suffixes:
        vars.Add("CPPPATH"+suffix,"List of directories where to search for headers",[],converter=convert_path)
        vars.Add("LIBPATH"+suffix,"List of directories where to search for libraries",[],converter=convert_path)
    vars.Add("CPPFLAGS","C/C++ preprocessor flags",[],converter=convert_flags)
    vars.Add("CCFLAGS","C/C++ compiler flags",["/O2","/GL","/Gw"] if sys.platform=="win32" else ["-O2"],converter=convert_flags)
    vars.Add("CFLAGS","C compiler flags",[],converter=convert_flags)
    vars.Add("CXXFLAGS","C++ compiler flags",[],converter=convert_flags)
    vars.Add("LINKFLAGS","Linker flags",["/LTCG","/OPT:REF","/OPT:ICF"] if sys.platform=="win32" else [],converter=convert_flags)
    return vars

def create_base_env(vars):
    env_args={}
    if sys.platform=="win32":
        env_args["MSVC_USE_SCRIPT"]=None
        env_args["tools"]=["msvc","mslink","mslib","newlines"]
        env_args["MSVC_BATCH"]=True
    else:
        env_args["tools"]=["default","installer"]
    env_args["tools"].extend(["textfile","library"])
    env_args["variables"]=vars
    env_args["LIBS"]=[]
    env_args["package_name"]="RHVoice"
    env_args["CPPDEFINES"]=[("RHVOICE","1")]
    env=Environment(**env_args)
    env["package_version"]=get_version(env["release"])
    env.Append(CPPDEFINES=("PACKAGE",env.subst(r'\"$package_name\"')))
    env.Append(CPPDEFINES=("VERSION",env.subst(r'\"$package_version\"')))
    if env["PLATFORM"]=="win32":
        env.Append(CPPDEFINES=("WIN32",1))
        env.Append(CPPDEFINES=("UNICODE",1))
        env.Append(CPPDEFINES=("NOMINMAX",1))
        env.AppendUnique(CCFLAGS=["/MT"])
        env.AppendUnique(CXXFLAGS=["/EHsc"])
    if "gcc" in env["TOOLS"]:
        env.MergeFlags("-pthread")
        env.AppendUnique(CXXFLAGS=["-std=c++03"])
    if sys.platform.startswith("linux"):
        env.Append(SHLINKFLAGS="-Wl,-soname,${TARGET.file}.${libversion.split('.')[0]}")
    return env

def display_help(env,vars):
    Help("Type 'scons' to build the package.\n")
    if sys.platform!="win32":
        Help("Then type 'scons install' to install it.\n")
        Help("Type 'scons --clean install' to uninstall the software.\n")
    Help("You may use the following configuration variables:\n")
    Help(vars.GenerateHelpText(env))

def clone_base_env(base_env,arch=None):
    env=base_env.Clone()
    if sys.platform=="win32":
        bits="64" if arch.endswith("64") else "32"
        env["BUILDDIR"]=os.path.join(BUILDDIR,arch)
        env["CPPPATH"]=env["CPPPATH"+bits]
        env["LIBPATH"]=env["LIBPATH"+bits]
        env["ENV"]=get_msvc_env_vars(env,arch)
    else:
        env["BUILDDIR"]=BUILDDIR
    third_party_dir=os.path.join("src","third-party")
    for path in Glob(os.path.join(third_party_dir,"*"),strings=True):
        if os.path.isdir(path):
            env.Prepend(CPPPATH=("#"+path))
    env.Prepend(CPPPATH=(".",os.path.join("#src","include")))
    return env

def configure(env):
    conf=env.Configure(conf_dir=os.path.join(env["BUILDDIR"],"configure_tests"),
                       log_file=os.path.join(env["BUILDDIR"],"configure.log"),
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
    env["audio_libs"]=set()
    has_giomm=False
    has_pkg_config=conf.CheckPKGConfig()
    if has_pkg_config:
        if conf.CheckPKG("libpulse-simple"):
            env["audio_libs"].add("pulse")
        if conf.CheckPKG("ao"):
            env["audio_libs"].add("libao")
        if conf.CheckPKG("portaudio-2.0"):
            env["audio_libs"].add("portaudio")
        has_giomm=conf.CheckPKG("giomm-2.4")
    if env["PLATFORM"]=="win32":
        env.AppendUnique(LIBS="kernel32")
    conf.Finish()
    src_subdirs=["third-party","core","lib","utils"]
    if env["audio_libs"]:
        src_subdirs.append("audio")
        src_subdirs.append("test")
        src_subdirs.append("sd_module")
    if has_giomm:
        src_subdirs.append("service")
    if env["PLATFORM"]=="win32":
        src_subdirs.append("sapi")
    else:
        src_subdirs.append("include")
    return src_subdirs

def build_binaries(base_env,arch=None):
    env=clone_base_env(base_env,arch)
    if env["BUILDDIR"]!=BUILDDIR:
        Execute(Mkdir(env["BUILDDIR"]))
    if arch:
        print("Configuring the build system for {}".format(arch))
    src_subdirs=configure(env)
    for subdir in src_subdirs:
        SConscript(os.path.join("src",subdir,"SConscript"),
                   variant_dir=os.path.join(env["BUILDDIR"],subdir),
                   exports={"env":env},
                   duplicate=0)

def build_for_linux(base_env):
    build_binaries(base_env)
    for subdir in ["data","config"]:
        SConscript(os.path.join(subdir,"SConscript"),exports={"env":base_env},
                   variant_dir=os.path.join(BUILDDIR,subdir),
                   duplicate=0)

def preconfigure_for_windows(env):
    conf=env.Configure(conf_dir=os.path.join(BUILDDIR,"configure_tests"),
                       log_file=os.path.join(BUILDDIR,"configure.log"),
                       custom_tests={"CheckVS":CheckVS,"CheckNSIS":CheckNSIS})
    if not conf.CheckVS():
        print("Error: Visual Studio 2013 is not installed")
        exit(1)
    if not conf.CheckNSIS(True):
        conf.CheckNSIS()
    conf.Finish()

def build_for_windows(base_env):
    preconfigure_for_windows(base_env)
    build_binaries(base_env,"x86")
    if base_env["enable_x64"]:
        build_binaries(base_env,"x64")
    SConscript(os.path.join("data","SConscript"),
               variant_dir=os.path.join(BUILDDIR,"data"),
               exports={"env":base_env},
               duplicate=0)
    for f in ["README","COPYING","COPYING.LESSER"]:
        base_env.ConvertNewlines(os.path.join(BUILDDIR,f),f)
    base_env.ConvertNewlinesB(os.path.join(BUILDDIR,"RHVoice.ini"),os.path.join("config","RHVoice.conf"))
    # env.ConvertNewlinesB(os.path.join(BUILDDIR,"dict.txt"),os.path.join("config","dicts","example.txt"))
    SConscript(os.path.join("src","nvda-synthDriver","SConscript"),
               variant_dir=os.path.join(BUILDDIR,"nvda-synthDriver"),
               exports={"env":base_env},
               duplicate=0)
    if "makensis" in base_env:
        SConscript(os.path.join("src","wininst","SConscript"),
                   variant_dir=os.path.join(BUILDDIR,"wininst"),
                   exports={"env":base_env},
                   duplicate=0)

setup()
vars=create_user_vars()
base_env=create_base_env(vars)
display_help(base_env,vars)
vars.Save(var_cache,base_env)
if sys.platform=="win32":
    build_for_windows(base_env)
else:
    build_for_linux(base_env)
