import os.path
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

def generate(env):
    env.AddMethod(InstallProgram)
    env.AddMethod(InstallData)
