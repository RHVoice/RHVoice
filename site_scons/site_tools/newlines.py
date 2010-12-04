from SCons.Script import *

def exists(env):
    if env["PLATFORM"]=="win32":
        return True
    else:
        return False

def ConvertNewlines(target,source,env):
    for t,s in zip(target,source):
        f_in=open(str(s),"rb")
        f_out=open(str(t),"wb")
        f_out.write(f_in.read().replace("\n","\r\n"))
        f_out.close()
        f_in.close()
    return None

def generate(env):
    env["BUILDERS"]["ConvertNewlines"]=Builder(action=ConvertNewlines,suffix=".txt")
