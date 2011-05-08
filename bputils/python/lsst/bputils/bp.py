"""
Common C++ standard library objects wrapped with Boost.Python
"""

from . import _bputils

try:
    _bputils._get_cerr(None)
except Exception, err:
    ArgumentError = type(err)
