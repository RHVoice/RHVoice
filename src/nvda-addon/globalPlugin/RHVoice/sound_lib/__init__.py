__author__ = "Christopher Toth"
__version__ = "0.8.5"


def find_datafiles():
    """ """
    from glob import glob
    import os
    import platform
    import sound_lib

    path = os.path.join(sound_lib.__path__[0], "lib")
    system = platform.system()
    if system == "Windows":
        file_ext = "*.dll"
    elif system == "Darwin":
        file_ext = "*.dylib"
    else:
        file_ext = "*.so"
    if platform.architecture()[0] == "32bit" or platform.system() == "Darwin":
        arch = "x86"
    else:
        arch = "x64"
    dest_dir = os.path.join("sound_lib", "lib", arch)
    source = glob(os.path.join(path, arch, file_ext))
    return [(dest_dir, source)]
