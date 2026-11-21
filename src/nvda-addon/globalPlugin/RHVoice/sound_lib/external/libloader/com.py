from pywintypes import com_error
from win32com.client import gencache


def prepare_gencache():
    gencache.is_readonly = False
    gencache.GetGeneratePath()


def load_com(*names):
    if gencache.is_readonly:
        prepare_gencache()
    result = None
    for name in names:
        try:
            result = gencache.EnsureDispatch(name)
            break
        except com_error:
            continue
    if result is None:
        raise com_error("Unable to load any of the provided com objects.")
    return result
