import platform
import ctypes
import os
import signal


def kill_windows_process(pid):
    PROCESS_TERMINATE = 1
    SYNCHRONIZE = 1048576
    handle = ctypes.windll.kernel32.OpenProcess(
        PROCESS_TERMINATE | SYNCHRONIZE, False, pid
    )
    ctypes.windll.kernel32.TerminateProcess(handle, -1)
    ctypes.windll.kernel32.WaitForSingleObject(handle, 1000)
    ctypes.windll.kernel32.CloseHandle(handle)


def kill_unix_process(pid):
    try:
        os.kill(pid, signal.SIGKILL)
    except OSError:
        pass


def kill_process(pid):
    """Forcefully kills a process."""
    if pid < 0:
        return
    if platform.system() == "Windows":
        kill_windows_process(pid)
    else:
        kill_unix_process(pid)
