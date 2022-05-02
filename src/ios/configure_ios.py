import os
import sys

import subprocess

def detect_darwin_sdk_path(platform, env):
    sdk_name = ""
    if platform == "osx":
        sdk_name = "macosx"
        var_name = "MACOS_SDK_PATH"
    elif platform == "iphone":
        sdk_name = "iphoneos"
        var_name = "IPHONESDK"
    elif platform == "iphonesimulator":
        sdk_name = "iphonesimulator"
        var_name = "IPHONESDK"
    else:
        raise Exception("Invalid platform argument passed to detect_darwin_sdk_path")

    env[var_name] = subprocess.check_output(["xcrun", "--sdk", sdk_name, "--show-sdk-path"]).strip().decode("utf-8")


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

    # Save this in environment for use by other modules
    if "OSXCROSS_IOS" in os.environ:
        env["osxcross"] = True

    env["ENV"]["PATH"] = env["IPHONEPATH"] + "/Developer/usr/bin/:" + env["ENV"]["PATH"]

    compiler_path = "$IPHONEPATH/usr/bin/${ios_triple}"
    s_compiler_path = "$IPHONEPATH/Developer/usr/bin/"

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
    env["AR"] = compiler_path + "ar"
    env["RANLIB"] = compiler_path + "ranlib"

    ## Compile flags
    env.extra_suffix = ""
    if env["simulator"]:
        detect_darwin_sdk_path("iphonesimulator", env)
        env.Append(CCFLAGS=["-mios-simulator-version-min=13.0"])
        env.extra_suffix = ".simulator" + env.extra_suffix
    else:
        detect_darwin_sdk_path("iphone", env)
        env.Append(CCFLAGS=["-miphoneos-version-min=11.0"])

    if env["arch"] == "x86_64":
        env["ENV"]["MACOSX_DEPLOYMENT_TARGET"] = "10.9"
        env.Append(
            CCFLAGS=(
                "-fobjc-arc -arch x86_64"
                " -fobjc-abi-version=2 -fobjc-legacy-dispatch -fmessage-length=0 -fpascal-strings -fblocks -fembed-bitcode"
                " -fasm-blocks -isysroot $IPHONESDK"
            ).split()
        )
    elif env["arch"] == "arm64":
        env.Append(
            CCFLAGS=(
                "-fobjc-arc -arch arm64 -fmessage-length=0 -fno-strict-aliasing"
                " -fdiagnostics-print-source-range-info -fdiagnostics-show-category=id -fdiagnostics-parseable-fixits"
                " -fpascal-strings -fblocks -fvisibility=hidden -MMD -MT dependencies -fembed-bitcode"
                " -isysroot $IPHONESDK".split()
            )
        )
        env.Append(CPPDEFINES=["NEED_LONG_INT"])

    # Temp fix for ABS/MAX/MIN macros in iPhone SDK blocking compilation
    env.Append(CCFLAGS=["-Wno-ambiguous-macro"])
    
    env.Prepend(
        CPPPATH=[
            env["IPHONESDK"]+"/usr/include",
            env["IPHONESDK"]+"/System/Library/Frameworks/AudioUnit.framework/Headers",
        ]
    )
    env.Append(CPPDEFINES=["IPHONE_ENABLED"])
