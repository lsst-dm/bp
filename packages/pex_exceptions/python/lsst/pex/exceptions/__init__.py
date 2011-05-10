from . import _exceptions
import lsst.bputils

@lsst.bputils.extend(_exceptions.Exception)
class Exception(object):

    def __str__(self):
        return str(self.args[0])

    def __getattr__(self, name):
        return getattr(self.args[0], name)

lsst.bputils.rescope(_exceptions, globals(), ignore=("Exception",))
