import sys
import os
import os.path

BUILDDIR=os.path.join("build",sys.platform)
Execute(Mkdir(BUILDDIR))
SConsignFile(os.path.join(BUILDDIR,"scons"))
var_cache=os.path.join(BUILDDIR,"user.conf")
args={"DESTDIR":""}
args.update(ARGUMENTS)
vars=Variables(var_cache,args)
if sys.platform!="win32":
    vars.Add("prefix","Installation prefix","/usr/local")
    vars.Add("bindir","Program installation directory","$prefix/bin")
    vars.Add("datadir","Data installation directory","$prefix/share")
    vars.Add("DESTDIR","Support for staged installation","")
vars.Add("FLAGS","Additional compiler/linker flags")
vars.Add("CCFLAGS","C compiler flags")
vars.Add("LINKFLAGS","Linker flags")
if sys.platform=="win32":
    vars.Add("MSVC_FLAGS","MSVC flags")
vars.Add(EnumVariable("debug","Build debug variant","no",["yes","no"],ignorecase=1))
vars.Add("package_version","Package version","0.3")
if sys.platform=="win32":
    toolset=["mingw","newlines"]
    sapi_env=Environment(tools=["msvc","mslink"],TARGET_ARCH="x86",enabled=False)
    sapi_env["CPPPATH"]=[".",os.path.join("#src","include")]
    sapi_env["CPPDEFINES"]=[("UNICODE","1")]
    sapi_env.Prepend(CCFLAGS="/MT")
    sapi_env.Prepend(CCFLAGS="/EHa")
else:
    toolset=["default","installer"]
env=Environment(tools=toolset,CPPPATH=[],LIBPATH=[],CPPDEFINES=[],package_name="RHVoice",variables=vars)
Help("Type 'scons' to build the package.\n")
if sys.platform!="win32":
    Help("Then type 'scons install' to install it.\n")
    Help("Type 'scons --clean install' to uninstall the software.\n")
Help("You may use the following configuration variables:\n")
Help(vars.GenerateHelpText(env))
env.Prepend(CPPPATH=(".",os.path.join("#src","include")))
env.Append(CPPDEFINES=("PACKAGE",env.subst(r'\"$package_name\"')))
if env["PLATFORM"]=="win32":
    env.Append(CPPDEFINES=("WIN32",1))
flags=env.get("FLAGS")
if flags:
    env.MergeFlags(flags)
if env.get("debug")=="yes":
    env.AppendUnique(CCFLAGS="-O0",delete_existing=1)
    env.AppendUnique(CCFLAGS="-g",delete_existing=1)
else:
    env.PrependUnique(CCFLAGS="-O2")
if env["PLATFORM"]=="win32":
    if env.get("debug")=="yes":
        sapi_env.AppendUnique(CCFLAGS="/Od",delete_existing=1)
    else:
        sapi_env.PrependUnique(CCFLAGS="/O2")
    flags=env.get("MSVC_FLAGS")
    if flags:
        sapi_env.MergeFlags(flags)
env.Append(CPPDEFINES=("VERSION",env.subst(r'\"$package_version\"')))
vars.Save(var_cache,env)
if env["PLATFORM"]=="win32":
    env.Append(CPPDEFINES={"PCRE_STATIC":"1"})
if GetOption("clean"):
    enable_config=False
elif GetOption("help"):
    enable_config=False
else:
    enable_config=True
if enable_config:
    conf=env.Configure(conf_dir=os.path.join(BUILDDIR,"configure_tests"),log_file=os.path.join(BUILDDIR,"configure.log"))
    if not conf.CheckCC():
        print "The C compiler is not working"
        exit(1)
    conf.CheckLib("m",autoadd=1)
    has_flite_h=conf.CheckCHeader("flite.h")
    if (not has_flite_h) and (env["PLATFORM"]!="win32"):
        for prefix in ["/usr/local","/usr"]:
            flite_cpppath=os.path.join(prefix,"include","flite")
            if os.path.isdir(flite_cpppath):
                if not GetOption("silent"):
                    print "trying to search in %s" % flite_cpppath
                env.Prepend(CPPPATH=flite_cpppath)
                has_flite_h=conf.CheckCHeader("flite.h")
                break
    if not has_flite_h:
        print "Error: cannot find flite.h"
        exit(1)
    has_libflite=conf.CheckLibWithHeader("flite","flite.h","C",call="flite_init();",autoadd=0)
    if not has_libflite:
        if hasattr(os,"uname"):
            if os.uname()[0]=="Linux":
                if not GetOption("silent"):
                    print "Perhaps this version of flite depends on alsa"
                if conf.CheckLib("asound",autoadd=1):
                    has_libflite=conf.CheckLibWithHeader("flite","flite.h","C",call="flite_init();",autoadd=0)
    if not has_libflite:
        print "error: cannot link with the flite library"
        exit(1)
    env.PrependUnique(LIBS="flite")
    if not conf.CheckLib("flite","cst_utf8_explode",autoadd=0):
        print "This version of flite is too old, please install flite 1.4 or later"
        exit(1)
    if not conf.CheckLib("flite_cmulex",autoadd=0):
        print "error: cannot link with the flite_cmulex library"
        exit(1)
    env.PrependUnique(LIBS="flite_cmulex")
    has_libunistring=conf.CheckLibWithHeader("unistring","uniconv.h","C",call="u8_strconv_to_locale((const uint8_t*)\"a\");",autoadd=0)
    if not has_libunistring:
        if not GetOption("silent"):
            print "Perhaps libunistring requires libiconv on this platform"
        if conf.CheckLib("iconv",autoadd=0):
            env.PrependUnique(LIBS="iconv")
            has_libunistring=conf.CheckLibWithHeader("unistring","unistr.h","C",call="u8_strchr((const uint8_t*)\"a\",'a');",autoadd=0)
    if not has_libunistring:
        print "Error: cannot link with libunistring"
        exit(1)
    env.PrependUnique(LIBS="unistring")
    has_expat=conf.CheckLibWithHeader("expat","expat.h","C",call='XML_ParserCreate("UTF-8");',autoadd=0)
    if not has_expat:
        print "Error: cannot link with expat"
        exit(1)
    env.PrependUnique(LIBS="expat")
    has_pcre=conf.CheckLibWithHeader("pcre","pcre.h","C",call='pcre_compile("^$",0,NULL,NULL,NULL);',autoadd=0)
    if not has_pcre:
        print "Error: cannot link with pcre"
        exit(1)
    env.PrependUnique(LIBS="pcre")
    has_sox=conf.CheckLibWithHeader("sox","sox.h","C",call='sox_init();',autoadd=0)
    if not has_sox:
        print "Error: cannot link with libsox"
        exit(1)
    env.PrependUnique(LIBS="sox")
    if env["PLATFORM"]=="win32":
        env.AppendUnique(LIBS="kernel32")
    env=conf.Finish()
    if env["PLATFORM"]=="win32":
        sapi_conf=sapi_env.Configure(conf_dir=os.path.join(BUILDDIR,"sapi_configure_tests"),
                                     log_file=os.path.join(BUILDDIR,"sapi_configure.log"))
        if sapi_conf.CheckCXX():
            found=False
            headers=["windows.h","shlobj.h","sapi.h","sapiddk.h",
                     "string","stdexcept","new","map","algorithm",
                     "sstream","locale","eh.h","comdef.h"]
            for header in headers:
                found=sapi_conf.CheckCXXHeader(header)
                if not found:
                    break
            if found:
                for lib in ["kernel32","advapi32","shell32","ole32","sapi","comsuppw"]:
                    found=sapi_conf.CheckLib(lib,language="C++",autoadd=0)
                    if not found:
                        break
                    sapi_env.PrependUnique(LIBS=lib)    
                if found:
                    sapi_env["enabled"]=True
                if not GetOption("silent") and not sapi_env["enabled"]:
                        print "Sapi 5 support cannot be compiled"
        sapi_env=sapi_conf.Finish()

Export(["env","BUILDDIR"])
if env["PLATFORM"]=="win32":
    Export("sapi_env")
lib_objects=[]
Export("lib_objects")
src_subdirs=["hts_engine_api","lib","bin"]
if env["PLATFORM"]=="win32":
    if sapi_env["enabled"]:
        src_subdirs.append("sapi")
for subdir in src_subdirs:
    SConscript(os.path.join("src",subdir,"SConscript"),
               variant_dir=os.path.join(BUILDDIR,subdir),
               duplicate=0)
if env["PLATFORM"]!="win32":
    SConscript(os.path.join("data","SConscript"))
if env["PLATFORM"]=="win32":
    for f in ["README","COPYING"]:
        env.ConvertNewlines(os.path.join(BUILDDIR,f),f)
