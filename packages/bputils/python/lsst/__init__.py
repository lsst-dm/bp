import pkgutil
__path__ = pkgutil.extend_path(__path__, __name__)

try:
    import lsstimport
except ImportError:
    pass
