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

from . import _logging
import lsst.bputils

lsst.bputils.rescope(_logging, globals(), ignore=("LogRecord", "Log", "LogRec", "Debug"))

class Prop:
    """package a property to send it to a LogRecord"""
    def __init__(self, name, value):
        self.name = name
        self.value = value

    def addToRec(self, rec):
        rec.addProperty(self.name, self.value)

@lsst.bputils.extend(_logging.LogRecord)
class LogRecord:
    
    def addProperty(self, name, val):
        """add a property with a default type.

        Users can explicitly set the C++ type of a property that gets added by 
        using the proper type-specific addProperty method (addPropertyBool(), 
        addPropertyLongLong(), etc.); the methods that are supported this way
        are int, long, long long, float, double, bool, string, and PropertySet.  
        This method will choose a type for the property based on the value.  

        If the value is a Python integer, then the value will be set based on 
        its value:  if the value is out of range of a 32-bit integer, 
        [-2147483648, 2147483648), it will be stored as a C++ int; out of that 
        range, the type will be long long.  All floating point numbers are stored
        as C++ doubles.  Booleans and strings are stored as C++ bools and 
        std::strings, respectively.  Lists are stored as arrays with the same
        mappings for the elements (but note that all values in the list must be
        of the same type).  Dictionaries are stored as PropertySets with similar
        mappings for their underlying types.

        @param name    the name of the property
        @param val     that value to set for the property
        @exception LsstException   if the value is of an unsupported type.
        """
        if isinstance(val, int):
            if val > 2147483648 or val <= -2147483648:
                return self.addPropertyLongLong(name, val)
            else:
                return self.addPropertyInt(name, val)
        elif isinstance(val, float):
            return self.addPropertyDouble(name, val)
        elif isinstance(val, bool):
            return self.addPropertyBool(name, val)
        elif isinstance(val, str):
            return self.addPropertyString(name, val)
        elif isinstance(val, lsst.daf.base.PropertySet):
    #        return self.addPropertyPropertySetPtr(name, val)
            raise lsst.pex.exceptions.LogicErrorException("PropertySet type temporarily unsupported")
        elif isinstance(val, list):
            for v in val:
                self.addProperty(name, v)
        elif isinstance(val, dict):
            for k in val.keys():
                self.addProperty("%s.%s" % (name, k), val[k])
        else:
            raise lsst.pex.exceptions.InvalidParameterException("unsupported property type for logging: %s(%s)" % (name, type(val)))

    def setProperty(self, name, val):
        """add a property with a default type.

        Users can explicitly set the C++ type of a property that gets added by 
        using the proper type-specific addProperty method (addPropertyBool(), 
        addPropertyLongLong(), etc.); the methods that are supported this way
        are int, long, long long, float, double, bool, string, and PropertySet.  
        This method will choose a type for the property based on the value.
        See addProperty() for an explanation of the default mappings. 

        @param name    the name of the property
        @param val     that value to set for the property
        @exception LsstException   if the value is of an unsupported type.
        """
        if isinstance(val, int):
            if val > 2147483648 or val <= -2147483648:
                return self.setPropertyLongLong(name, val)
            else:
                return self.setPropertyInt(name, val)
        elif isinstance(val, float):
            return self.setPropertyDouble(name, val)
        elif isinstance(val, bool):
            return self.setPropertyBool(name, val)
        elif isinstance(val, string):
            return self.setPropertyBool(name, val)
        elif isinstance(val, lsst.daf.base.PropertySet):
    #        return self.setPropertyPropertySet(name, val)
            raise lsst.pex.exceptions.LogicErrorException("PropertySet type temporarily unsupported")
        elif isinstance(val, list):
            v = val.pop(0)
            self.setProperty(name, v)
            for v in val:
                self.addProperty(name, v)
        elif isinstance(val, dict):
            self.data().remove(name)
            for k in val.keys():
                self.addProperty("%s.%s" % (name, k), val[k])
        else:
            raise lsst.pex.exceptions.InvalidParameterException("unsupported property type for logging: %s(%s)" % (name, type(val)))

@lsst.bputils.extend(_logging.Log)
class Log:

    def _log(self, verb, *args):
        """send any number of strings, PropertySets, or other properties 
        in a message to the Log.  
        """
        rec = LogRec(self, verb)
        for prop in args:
            if isinstance(prop,str):
                rec << prop
            if isinstance(prop, lsst.daf.base.PropertySet):
                rec.addProperties(prop)

        rec << endr    # sends result
        return self

    def _registerLogRec(self, logrec):
        if not hasattr(self.__dict__, '_logrecs'):
            self.__dict__['_logrecs'] = {}
        self.__dict__['_logrecs'][repr(logrec)] = logrec

    def _unregisterLogRec(self, logrec):
        if not hasattr(self.__dict__, '_logrecs'):
            self.__dict__['_logrecs'] = {}
        rep = repr(logrec)
        if self.__dict__['_logrecs'].has_key(rep):
            del (self.__dict__['_logrecs'])[rep]

getDefaultLog = Log.getDefaultLog

class LogRec(_logging.LogRec):

    def __init__(self, *args, **kwds):
        _logging.LogRec.__init__(self, *args, **kwds)
        if isinstance(args[0], LogRec):
            self._theLog = args[0]._theLog
        else:
            self._theLog = args[0]

    def __lshift__(self, *args):
        """an extension to the << operator to support C++-like interface.

        This implementation does two things:  (1) supports adding Prop objects,
        PropertySets, and dictionaries for adding properties, and (2) unregisters
        this LogRec from its Log instance when an endr object is added.
        """
        out = self
        if isinstance(args[0], Prop):
            self.addProperty(args[0].name, args[0].value)
        elif isinstance(args[0], lsst.daf.base.PropertySet):
            self.addProperties(args[0])
        elif isinstance(args[0], dict):
            for k in args[0].keys():
                self.addProperty(k, args[0][k])
        else:
            # all other types, handle with the default behavior
            out = self._lshift_cpp(*args)

        # if we just shifted on the endr manipulator, assume that it is safe
        # to garbage collect this object:  unregister it from it's Log.
        if (hasattr(self,'_theLog') and len(args) > 0 and
            isinstance(args[0], int) and args[0] == LogRec.endr):
            self._theLog._unregisterLogRec(self)

        return out

endr = LogRec.endr
Rec = LogRec

class Debug(_logging.Debug):

    default_max_debug = None

    def __init__(self, name, verbosity=None, parent=None):
        if verbosity is None:
            verbosity = Debug.default_max_debug
        if verbosity is None:
            verbosity = -1 * Log.INHERIT_THRESHOLD
        if parent is not None:
            _logging.Debug.__init__(self, parent, name=name, verbosity=verbosity)
        else:
            _logging.Debug.__init__(self, name=name, verbosity=verbosity)

def version():
    """
    Return the version of this product (extracted from an internal HeadURL string).
    If a different version is setup according to eups, include it in the return string.
    """
    HeadURL = r"$HeadURL: svn+ssh://svn.lsstcorp.org/DMS/pex/logging/branches/boost.python/python/lsst/pex/logging/loggingLib.i $"
    return lsst.utils.version(HeadURL)
