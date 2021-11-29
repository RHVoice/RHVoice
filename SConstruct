# Copyright (C) 2010, 2011, 2012, 2013, 2014, 2018, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
import os
import os.path
import subprocess
import platform
import datetime
import re
if sys.platform=="win32":
    if sys.version_info[0]>=3:
        import winreg
    else:
        import _winreg as winreg

def get_version(is_release):
    next_version="1.6.0"
    return next_version

def passthru(env, cmd, unique=False):
    return cmd.rstrip()

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

def CheckMSVC(context):
    context.Message("Checking for Visual C++ ... ")
    result=0
    version=context.env.get("MSVC_VERSION",None)
    if version is not None:
        result=1
    context.Result(result)
    return result

def CheckXPCompat(context):
    context.Message("Checking for Windows XP compatibility ... ")
    result=0
    if context.env.get("xp_compat_enabled",False):
        result=1
    context.Result(result)
    return result

def CheckNSIS(context):
    result=0
    context.Message("Checking for NSIS")
    if "NSISDIR" in os.environ:
        context.env["makensis"]=File(os.path.join(os.environ["NSISDIR"],"makensis.exe"))
        result=1
    else:
        key_name=r"SOFTWARE\NSIS"
        try:
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,key_name,0,winreg.KEY_READ|winreg.KEY_WOW64_32KEY) as key:
                context.env["makensis"]=File(os.path.join(winreg.QueryValueEx(key,None)[0],"makensis.exe"))
                result=1
        except WindowsError:
            pass
    context.Result(result)
    return result

def CheckWiX(context):
    result=0
    context.Message("Checking for WiX toolset")
    if "WIXTOOLPATH" in os.environ or "WIX" in os.environ:
        if "WIXTOOLPATH"in os.environ:
            context.env["WIX"]=os.environ["WIXTOOLPATH"]
        else:
            context.env["WIX"]=os.path.join(os.environ["WIX"],"bin")
        result=1
    context.Result(result)
    return result

def get_spd_module_dir():
    env = Environment()
    return env.ParseConfig("pkg-config speech-dispatcher --variable=modulebindir", passthru)

def validate_spd_version(key,val,env):
    m=re.match(r"^\d+\.\d+",val)
    if m is None:
        raise Exception("Invalid value of spd_version: {}".format(val))

def CheckSpdVersion(ctx):
    ctx.Message("Checking Speech Dispatcher version ... ")
    ver=ctx.env.get("spd_version",None)
    if ver is not None:
        ctx.Result(ver)
        return ver
    res, ver=ctx.TryAction("pkg-config --modversion speech-dispatcher > $TARGET")
    ver=ver.strip()
    if not res:
        src='#include <stdio.h>\n#include <speech-dispatcher/libspeechd_version.h>\nint main() {\nint major=LIBSPEECHD_MAJOR_VERSION;\nint minor=LIBSPEECHD_MINOR_VERSION;\nprintf("%d.%d",major,minor);\nreturn 0;}'
        res,ver=ctx.TryRun(src,".c")
    if not res:
        ctx.Result(res)
        return res
    ctx.env["spd_version"]=ver
    ctx.Result(ver)
    return ver

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

def create_languages_user_var():
    langs_dir=Dir("#data").Dir("languages")
    names=[name for name in sorted(os.listdir(langs_dir.path)) if os.path.isdir(langs_dir.Entry(name).path)]
    langs=[name.lower() for name in names]
    name_map=dict(zip(names,langs))
    def_langs=langs
    if sys.platform!="win32":
        def_langs=[lang for lang in langs if lang not in["georgian"]]
        print("Georgian language is skipped because of non-free license")
    help="Which languages to install"
    return ListVariable("languages",help,def_langs,langs,name_map)

def create_audio_libs_user_var():
    libs=["pulse","libao","portaudio"]
    help="Which audio libraries to use if they are available"
    return ListVariable("audio_libs",help,libs,libs)

def create_user_vars():
    args={"DESTDIR":""}
    args.update(ARGUMENTS)
    vars=Variables(var_cache,args)
    vars.Add(BoolVariable("dev","The build will only be used for development: no global installation, run from the source directory, compile helper utilities",False))
    vars.Add(create_languages_user_var())
    vars.Add(BoolVariable("enable_mage","Build with MAGE",True))
    vars.Add(BoolVariable("enable_sonic","Build with Sonic",False))
    vars.Add(create_audio_libs_user_var())
    vars.Add(BoolVariable("release","Whether we are building a release",True))
    if sys.platform=="win32":
        vars.Add(BoolVariable("enable_x64","Additionally build 64-bit versions of all the libraries",True))
        vars.Add(BoolVariable("enable_xp_compat","Target Windows XP",False))
        vars.Add(PathVariable("msi_repo","Where the msi packages are kept for reuse",None,PathVariable.PathIsDir))
    else:
        vars.Add(PathVariable("spd_module_dir", "Speech dispatcher module directory", get_spd_module_dir(),  PathVariable.PathAccept))
        vars.Add("spd_version","Speech dispatcher version",validator=validate_spd_version)
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

def create_base_env(user_vars):
    env_args={"variables":user_vars}
    if sys.platform=="win32":
        env_args["tools"]=["newlines"]
    else:
        env_args["tools"]=["default","installer"]
    env_args["tools"].extend(["textfile","library"])
    env_args["LIBS"]=[]
    env_args["package_name"]="RHVoice"
    env_args["CPPDEFINES"]=[("RHVOICE","1")]
    env=Environment(**env_args)
    if env["dev"]:
        env["prefix"]=os.path.abspath("local")
        env["RPATH"]=env.Dir("$libdir").abspath
    env["package_version"]=get_version(env["release"])
    env.Append(CPPDEFINES=("PACKAGE",env.subst(r'\"$package_name\"')))
    if env["PLATFORM"]=="win32":
        env.Append(CPPDEFINES=("WIN32",1))
        env.Append(CPPDEFINES=("UNICODE",1))
        env.Append(CPPDEFINES=("NOMINMAX",1))
    env["libcore"]="RHVoice_core"
    env["libaudio"]="RHVoice_audio"
    return env

def display_help(env,vars):
    Help("Type 'scons' to build the package.\n")
    if sys.platform!="win32":
        Help("Then type 'scons install' to install it.\n")
        Help("Type 'scons --clean install' to uninstall the software.\n")
    Help("You may use the following configuration variables:\n")
    Help(vars.GenerateHelpText(env))

def clone_base_env(base_env,user_vars,arch=None):
    args={}
    if sys.platform=="win32":
        if arch is not None:
            args["TARGET_ARCH"]=arch
        args["tools"]=["msvc","mslink","mslib"]
    env=base_env.Clone(**args)
    user_vars.Update(env)
    if env["PLATFORM"]=="win32":
        env.AppendUnique(CCFLAGS=["/nologo","/MT"])
        env.AppendUnique(LINKFLAGS=["/nologo"])
        env.AppendUnique(CXXFLAGS=["/EHsc"])
        if env["enable_xp_compat"]:
            env.Tool("xp_compat")
    if "gcc" in env["TOOLS"]:
        env.MergeFlags("-pthread")
        env.AppendUnique(CXXFLAGS=["-std=c++11"])
        env.AppendUnique(CFLAGS=["-std=c11"])
        if 'SOURCE_DATE_EPOCH' in os.environ:
            env['ENV']['SOURCE_DATE_EPOCH'] = os.environ['SOURCE_DATE_EPOCH']
    if sys.platform=="win32":
        bits="64" if arch.endswith("64") else "32"
        env["BUILDDIR"]=os.path.join(BUILDDIR,arch)
        env["CPPPATH"]=env["CPPPATH"+bits]
        env["LIBPATH"]=env["LIBPATH"+bits]
    else:
        env["BUILDDIR"]=BUILDDIR
    third_party_dir=os.path.join("src","third-party")
    for path in Glob(os.path.join(third_party_dir,"*"),strings=True):
        if os.path.isdir(path):
            env.Prepend(CPPPATH=("#"+path))
    env.Prepend(CPPPATH=(os.path.join("#"+env["BUILDDIR"],"include"),".",os.path.join("#src","include")))
    return env

def configure(env):
    tests={"CheckPKGConfig":CheckPKGConfig,"CheckPKG":CheckPKG,"CheckSpdVersion":CheckSpdVersion}
    if env["PLATFORM"]=="win32":
        tests["CheckMSVC"]=CheckMSVC
        tests["CheckXPCompat"]=CheckXPCompat
    conf=env.Configure(conf_dir=os.path.join(env["BUILDDIR"],"configure_tests"),
                       log_file=os.path.join(env["BUILDDIR"],"configure.log"),
                       config_h=os.path.join(env["BUILDDIR"],"include","configure.h"),
                       custom_tests=tests)
    if env["PLATFORM"]=="win32":
        if not conf.CheckMSVC():
            print("Error: Visual C++ is not installed")
            exit(1)
        print("Visual C++ version is {}".format(env["MSVC_VERSION"]))
        if env["enable_xp_compat"] and not conf.CheckXPCompat():
            print("Error: Windows XP compatibility cannot be enabled")
            exit(1)
    if not conf.CheckCC():
        print("The C compiler is not working")
        exit(1)
    if not conf.CheckCXX():
        print("The C++ compiler is not working")
        exit(1)
# has_sox=conf.CheckLibWithHeader("sox","sox.h","C",call='sox_init();',autoadd=0)
# if not has_sox:
#     print("Error: cannot link with libsox")
#     exit(1)
# env.PrependUnique(LIBS="sox")
    has_giomm=False
    has_pkg_config=conf.CheckPKGConfig()
    if has_pkg_config:
        if "pulse" in env["audio_libs"] and not conf.CheckPKG("libpulse-simple"):
            env["audio_libs"].remove("pulse")
        if "libao" in env["audio_libs"] and not conf.CheckPKG("ao"):
            env["audio_libs"].remove("libao")
        if "portaudio" in env["audio_libs"] and not conf.CheckPKG("portaudio-2.0"):
            env["audio_libs"].remove("portaudio")
        if env["audio_libs"]:
            conf.CheckSpdVersion()
    else:
        env["audio_libs"]=[]
#        has_giomm=conf.CheckPKG("giomm-2.4")
    if env["PLATFORM"]=="win32":
        env.AppendUnique(LIBS="kernel32")
    conf.Finish()
    env.Prepend(LIBPATH=os.path.join("#"+env["BUILDDIR"],"core"))
    src_subdirs=["third-party","core","lib"]
    if env["dev"]:
        src_subdirs.append("utils")
    src_subdirs.append("audio")
    src_subdirs.append("test")
    if env["audio_libs"]:
        src_subdirs.append("sd_module")
    env.Prepend(LIBPATH=os.path.join("#"+env["BUILDDIR"],"audio"))
    if has_giomm:
        src_subdirs.append("service")
    if env["PLATFORM"]=="win32":
        src_subdirs.append("sapi")
    else:
        src_subdirs.append("include")
    return src_subdirs

def build_binaries(base_env,user_vars,arch=None):
    env=clone_base_env(base_env,user_vars,arch)
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

def build_for_linux(base_env,user_vars):
    build_binaries(base_env,user_vars)
    for subdir in ["data","config"]:
        SConscript(os.path.join(subdir,"SConscript"),exports={"env":base_env},
                   variant_dir=os.path.join(BUILDDIR,subdir),
                   duplicate=0)

def preconfigure_for_windows(env):
    conf=env.Configure(conf_dir=os.path.join(BUILDDIR,"configure_tests"),
                       log_file=os.path.join(BUILDDIR,"configure.log"),
                       custom_tests={"CheckNSIS":CheckNSIS,"CheckWiX":CheckWiX})
    conf.CheckWiX()
    conf.CheckNSIS()
    conf.Finish()

def build_for_windows(base_env,user_vars):
    preconfigure_for_windows(base_env)
    build_binaries(base_env,user_vars,"x86")
    if base_env["enable_x64"]:
        build_binaries(base_env,user_vars,"x86_64")
    if "WIX" in base_env:
        SConscript(os.path.join("src","wininst","SConscript"),
                   variant_dir=os.path.join(BUILDDIR,"wininst"),
                   exports={"env":base_env},
                   duplicate=0)
    SConscript(os.path.join("data","SConscript"),
               variant_dir=os.path.join(BUILDDIR,"data"),
               exports={"env":base_env},
               duplicate=0)
    docs=["README.md"]+[os.path.join("licenses",name) for name in os.listdir("licenses") if name!="voices"]
    for f in docs:
        base_env.ConvertNewlines(os.path.join(BUILDDIR,f),f)
    base_env.ConvertNewlinesB(os.path.join(BUILDDIR,"RHVoice.ini"),os.path.join("config","RHVoice.conf"))
    # env.ConvertNewlinesB(os.path.join(BUILDDIR,"dict.txt"),os.path.join("config","dicts","example.txt"))
    SConscript(os.path.join("src","nvda-synthDriver","SConscript"),
               variant_dir=os.path.join(BUILDDIR,"nvda-synthDriver"),
               exports={"env":base_env},
               duplicate=0)

setup()
vars=create_user_vars()
base_env=create_base_env(vars)
display_help(base_env,vars)
vars.Save(var_cache,base_env)
if sys.platform=="win32":
    build_for_windows(base_env,vars)
else:
    build_for_linux(base_env,vars)
