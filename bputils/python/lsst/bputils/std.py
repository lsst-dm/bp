"""
Common C++ standard library objects wrapped with Boost.Python
"""

from . import _bputils
from . import attributes

attributes.rescope(_bputils, globals(), ("ofstream", "ostringstream"))

@attributes.extend(_bputils.ostream)
class ostream:

    def writelines(self, iterable):
        for line in iterable:
            self.write(line)

cout = _bputils._get_cout()
cerr = _bputils._get_cerr()

del attributes
