import os

import subprocess

def sdk_var_name(platform):
    if platform == "osx":
        return "MACOS_SDK_PATH"
    elif platform == "iphone":
        return "IPHONESDK"
    elif platform == "iphonesimulator":
        return "IPHONESDK"
    else:
        raise Exception("Not supported platform")
        
def detect_darwin_sdk_path(platform, env):
    sdk_name = ""
    if platform == "osx":
        sdk_name = "macosx"
    elif platform == "iphone":
        sdk_name = "iphoneos"
    elif platform == "iphonesimulator":
        sdk_name = "iphonesimulator"
    else:
        raise Exception("Invalid platform argument passed to detect_darwin_sdk_path")

    env[sdk_var_name(platform)] = subprocess.check_output(["xcrun", "--sdk", sdk_name, "--show-sdk-path"]).strip().decode("utf-8")

def configure(env):
    ## Build type

    if env["scheme"].startswith("release"):
        env.Append(CPPDEFINES=["NDEBUG", ("NS_BLOCK_ASSERTIONS", 1)])
    elif env["scheme"] == "debug":
        env.Append(CCFLAGS=["-gdwarf-2", "-O0"])
        env.Append(CPPDEFINES=["_DEBUG", ("DEBUG", 1)])

    ## arch
    env["bits"] = "64"
    if env["arch"] != "x86_64":
        env["arch"] = "arm64"

    ## Compiler configuration
    env["ENV"]["PATH"] = env["XCODE_TOOLCHAINT_PATH"] + "/Developer/usr/bin/:" + env["ENV"]["PATH"]

    compiler_path = "$XCODE_TOOLCHAINT_PATH/usr/bin/${ios_triple}"
    s_compiler_path = "$XCODE_TOOLCHAINT_PATH/Developer/usr/bin/"

    ccache_path = os.environ.get("CCACHE")
    if ccache_path is None:
        env["CC"] = compiler_path + "clang"
        env["CXX"] = compiler_path + "clang++"
        env["S_compiler"] = s_compiler_path + "gcc"
    else:
        # there aren't any ccache wrappers available for iOS,
        # to enable caching we need to prepend the path to the ccache binary
        env["CC"] = ccache_path + " " + compiler_path + "clang"
        env["CXX"] = ccache_path + " " + compiler_path + "clang++"
        env["S_compiler"] = ccache_path + " " + s_compiler_path + "gcc"

    ## Compile flags
    sdk_var = ""
    if env["platform"] == "macos": 
        env["ENV"]["MACOSX_DEPLOYMENT_TARGET"] = "11.0"
        sdk_var = sdk_var_name("osx")
        detect_darwin_sdk_path("osx", env)
    elif env["platform"] == "ios":
        if env["simulator"]:
            sdk_var = sdk_var_name("iphonesimulator")
            detect_darwin_sdk_path("iphonesimulator", env)
            env.Append(CCFLAGS=["-mios-simulator-version-min=13.0"])
            env.extra_suffix = ".simulator"
        else:
            sdk_var = sdk_var_name("iphone")
            detect_darwin_sdk_path("iphone", env)
            env.Append(CCFLAGS=["-miphoneos-version-min=13.0"])
    else:
        raise Exception("Not supported platform")

    if env["arch"] == "x86_64":
        env.Append(
            CCFLAGS=(
                "-fobjc-arc -arch x86_64"
                " -fobjc-abi-version=2 -fobjc-legacy-dispatch -fmessage-length=0 -fpascal-strings -fblocks -fembed-bitcode"
                " -fasm-blocks -isysroot $" + sdk_var
            ).split()
        )
    elif env["arch"] == "arm64":
        env.Append(
            CCFLAGS=(
                "-fobjc-arc -arch arm64 -fmessage-length=0 -fno-strict-aliasing"
                " -fdiagnostics-print-source-range-info -fdiagnostics-show-category=id -fdiagnostics-parseable-fixits"
                " -fpascal-strings -fblocks -fvisibility=hidden -MMD -MT dependencies -fembed-bitcode"
                " -isysroot $" + sdk_var
            ).split()
        )
    
    env.Prepend(
        CPPPATH=[
            env[sdk_var]+"/usr/include",
            env[sdk_var]+"/System/Library/Frameworks/AudioUnit.framework/Headers",
        ]
    )
