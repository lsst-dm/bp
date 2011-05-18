"""
Miscellaneous utilites for customizing Python attributes and descriptors.
"""

import re

def getattr_error(self):
    raise AttributeError("unreadable attribute")    

def setattr_error(self, value):
    raise AttributeError("can't set attribute")

def delattr_error(self):
    raise AttributeError("can't delete attribute")

def setattr_pass(self, value):
    pass

def delattr_pass(self):
    pass

class CachedProperty(property):

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        if not hasattr(obj, self.private):
            setattr(obj, self.private, self.fget(obj))
        return getattr(obj, self.private)

    def __set__(self, obj, value):
        if self.fset is None:
            raise AttributeError("can't set attribute")
        self.fset(obj, value)
        setattr(obj, self.private, value)

    def __delete__(self, obj):
        self.fdel(obj)
        if hasattr(obj, self.private):
            delattr(obj, self.private)

    def __init__(self, private, fget=getattr_error, fset=setattr_error, fdel=delattr_pass, doc=None):
        property.__init__(self, fget=fget, fset=fset, fdel=fdel, doc=doc)
        self.private = private

def apply(source, function, names=None, ignore=None, names_regex=None, ignore_regex=None, 
          only_classes=False):
    """Call function(obj, name) for each object in dir(source).
    """
    names_re = None if names_regex is None else re.compile(names_regex)
    ignore_re = None if ignore_regex is None else re.compile(ignore_regex)
    if ignore is None:
        ignore = ()
    if names is None:
        names = [name for name in dir(source) if name not in ignore and not name.startswith("_")]
    for name in names:
        if names_re and not names_re.match(name): continue
        if ignore_re and ignore_re.match(name): continue
        obj = getattr(source, name)
        if only_classes and not isinstance(obj, type): continue
        function(obj, name)

def member_of(cls, name=None):
    """A parametrized descriptor that adds a method or nested class to a class outside the class
    definition scope.  Example:

    class test_class(object):
        pass

    @member_of(test_class):
    def test_method(self):
        print "test_method!"

    The function or method will still be added to the module scope as well, replacing any
    existing module-scope function with that name; this appears to be an
    unavoidable side-effect.
    """
    if isinstance(cls, type):
        classes = (cls,)
    else:
        classes = tuple(cls)
    kw = {"name": name}
    def nested(member):
        if kw["name"] is None: kw["name"] = member.__name__
        for scope in classes:
            setattr(scope, kw["name"], member)
        return member
    return nested

class Rescope(object):

    def __init__(self, scope):
        self.scope = scope

    def __call__(self, obj, name):
        try: # this is settable for classes, but not free functions
            obj.__module__ = self.scope['__name__']
        except AttributeError:
            pass
        self.scope[name] = obj

def extend(target, rename=True, rescope=True):
    """A parametrized decorator that allows one to add additional members to an
    existing class.  The decorator returns the target class, adding it to the
    scope of the definition.
    """
    def nested(source):
        target.__module__ = source.__module__
        target.__name__ = source.__name__
        if hasattr(target, "__const_proxy__"):
            proxy = target.__const_proxy__
            proxy.__module__ = source.__module__
            proxy.__name__ = "%s.__const_proxy__" % source.__name__
        else:
            proxy = None
        for name in dir(source):
            member = getattr(source, name)
            if hasattr(member, "im_class"):
                member = member.im_func
            elif name == "__doc__" and member is not None:
                pass
            elif name.startswith("__") or name.endswith("__"):
                continue
            setattr(target, name, member)
            if proxy:
                setattr(proxy, name, member)
        return target
    return nested

def rescope(source, scope, **kw):
    """Move objects from module 'source' into a scope defined by globals dict 'scope', updating
    their __module__ attribute.

    This is intended as a replacement for 'from source import ...' when the objects should be
    considered to exist in the target scope and their definition in 'source' is just an
    implemntation detail (i.e. source is usually a wrapped C++ module, and we want the names
    to appear directly in the package).
    """
    apply(source, Rescope(scope), **kw)

def copy_str_to_repr(cls, name):
    setattr(cls, "__repr__", getattr(cls, "__str__"))
