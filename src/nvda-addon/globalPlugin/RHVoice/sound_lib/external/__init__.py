from __future__ import absolute_import
import platform
from . import pybassopus
if platform.system() == 'Windows':
 from . import pybasswma
if platform.system() != 'Darwin':
 from . import pybass_aac
 from . import pybass_alac
 from . import pybassflac
 from . import pybassmidi
