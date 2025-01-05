# Replacement for py2exe distributed module
# Avoids the use of the standard py2exe console.
# Just import this file and it should go away

import sys

if hasattr(sys, "frozen"):  # true only if we are running as a py2exe app

    class Blackhole(object):
        """Mock file object that does nothing."""

        def write(self, text):
            pass

        def flush(self):
            pass

        def isatty(self):
            return False

    sys.stdout = Blackhole()
    sys.stderr = Blackhole()
    del Blackhole
    del sys
