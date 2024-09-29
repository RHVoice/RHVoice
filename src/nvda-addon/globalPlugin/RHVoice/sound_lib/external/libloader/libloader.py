import ctypes
import collections
import platform
import os
import sys

TYPES = {
    "Linux": {
        "loader": ctypes.CDLL,
        "functype": ctypes.CFUNCTYPE,
        "prefix": "lib",
        "extension": ".so",
    },
    "Darwin": {
        "loader": ctypes.CDLL,
        "functype": ctypes.CFUNCTYPE,
        "prefix": "lib",
        "extension": ".dylib",
    },
}
if platform.system() == "Windows":
    TYPES["Windows"] = {
        "loader": ctypes.WinDLL,
        "functype": ctypes.WINFUNCTYPE,
        "prefix": "",
        "extension": ".dll",
    }


class LibraryLoadError(OSError):
    pass


def load_library(library, x86_path=".", x64_path=".", *args, **kwargs):
    lib = find_library_path(library, x86_path=x86_path, x64_path=x64_path)
    loaded = _do_load(lib, *args, **kwargs)
    if loaded is not None:
        return loaded
    raise LibraryLoadError(
        "unable to load %r. Provided library path: %r" % (library, path)
    )


def _do_load(file, *args, **kwargs):
    loader = TYPES[platform.system()]["loader"]
    return loader(file, *args, **kwargs)


def find_library_path(libname, x86_path=".", x64_path="."):
    libname = "%s%s" % (TYPES[platform.system()]["prefix"], libname)
    if platform.architecture()[0] == "64bit":
        path = os.path.join(x64_path, libname)
    else:
        path = os.path.join(x86_path, libname)
    ext = get_library_extension()
    path = "%s%s" % (path, ext)
    return os.path.abspath(path)


def get_functype():
    return TYPES[platform.system()]["functype"]


def get_library_extension():
    return TYPES[platform.system()]["extension"]
