def CheckSpdVersion(ctx):
    ctx.Message("Checking Speech Dispatcher version ... ")
    ver=ctx.env.get("spd_version",None)
    if ver is not None:
        ctx.Result(ver)
        return ver
    src='#include <stdio.h>\n#include <speech-dispatcher/libspeechd_version.h>\nint main() {\nint major=LIBSPEECHD_MAJOR_VERSION;\nint minor=LIBSPEECHD_MINOR_VERSION;\nprintf("%d.%d",major,minor);\nreturn 0;}'
    res,ver=ctx.TryRun(src,".c")
    if not res:
        ctx.Result(res)
        return res
    ctx.env["spd_version"]=ver
    ctx.Result(ver)
    return ver
