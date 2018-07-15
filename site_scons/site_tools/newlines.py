from SCons.Script import *

def exists(env):
    return (env["PLATFORM"]=="win32")

def ConvertNewlines(target,source,env):
    for t,s in zip(target,source):
        f_in=open(str(s),"rb")
        f_out=open(str(t),"wb")
        f_out.write(f_in.read().replace(b"\n",b"\r\n"))
        f_out.close()
        f_in.close()
    return None

def ConvertNewlinesB(target,source,env):
    for t,s in zip(target,source):
        f_in=open(str(s),"rb")
        f_out=open(str(t),"wb")
        f_out.write(b"\xef\xbb\xbf")
        f_out.write(f_in.read().replace(b"\n",b"\r\n"))
        f_out.close()
        f_in.close()
    return None

def generate(env):
    env["BUILDERS"]["ConvertNewlines"]=Builder(action=ConvertNewlines,suffix=".txt")
    env["BUILDERS"]["ConvertNewlinesB"]=Builder(action=ConvertNewlinesB,suffix=".txt")
