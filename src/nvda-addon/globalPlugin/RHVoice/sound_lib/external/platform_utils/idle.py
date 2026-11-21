import platform

system = platform.system()


def get_user_idle_time():
    """

    Args:

    Returns:
      This is normally obtained from a lack of keyboard and/or mouse input.

    """
    if system == "Windows":
        return get_user_idle_time_windows()
    elif system == "Darwin":
        return get_user_idle_time_mac()
    raise NotImplementedError("This function is not yet implemented for %s" % system)


def get_user_idle_time_windows():
    """ """
    from ctypes import Structure, windll, c_uint, sizeof, byref

    class LASTINPUTINFO(Structure):
        """ """
        _fields_ = [("cbSize", c_uint), ("dwTime", c_uint)]

    lastInputInfo = LASTINPUTINFO()
    lastInputInfo.cbSize = sizeof(lastInputInfo)
    windll.user32.GetLastInputInfo(byref(lastInputInfo))
    millis = windll.kernel32.GetTickCount() - lastInputInfo.dwTime
    return millis / 1000.0


def get_user_idle_time_mac():
    """ """
    import subprocess
    import re

    s = subprocess.Popen(("ioreg", "-c", "IOHIDSystem"), stdout=subprocess.PIPE)
    data = s.communicate()[0]
    expression = "HIDIdleTime.*"
    try:
        data = data.decode()
        r = re.compile(expression)
    except UnicodeDecodeError:
        r = re.compile(expression.encode())
    return int(r.findall(data)[0].split(" = ")[1]) / 1000000000
