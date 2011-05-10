# 
# LSST Data Management System
# Copyright 2008, 2009, 2010 LSST Corporation.
# 
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the LSST License Statement and 
# the GNU General Public License along with this program.  If not, 
# see <http://www.lsstcorp.org/LegalNotices/>.
#

#
# Define a class to configure debugging information
#
class Info(object):
    """An object cognisant of debugging parameters appropriate for module "name"; any request for a value
will return False unless that value has been set, either in the module or as an attribute of this object.

E.g.
    import lsstDebug

    display = lsstDebug.Info(__name__).display
will set display to False, unless display has been set:
    display = True
    print lsstDebug.Info(__name__).display
will print True; this is equivalent to
    lsstDebug.Info(__name__).display = True
    print lsstDebug.Info(__name__).display

Why is this interesting?  Because you can replace lsstDebug.Info with your own version, e.g.

import lsstDebug

def DebugInfo(name):
    di = lsstDebug.getInfo(name)        # N.b. lsstDebug.Info(name) would call us recursively
    if name == "foo":
        di.display = True
        
    return di
    
lsstDebug.Info = DebugInfo
"""
    def __init__(self, modname):
        import sys
        self.__dict__["_dict"] = sys.modules[modname].__dict__
        self._modname = modname

    def __getattr__(self, what):
        """Return the value of the variable "what" in self.__modname if set, else False"""
        return self._dict.get(what, False)

    def __setattr__(self, what, value):
        """Set the value of the variable "what" in self.__modname to value"""
        self._dict[what] = value

getInfo = Info
