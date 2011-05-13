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
from . import _pex_policy
from . import paf

import lsst.bputils

_pex_policy.Policy._set = _pex_policy.Policy.set
_pex_policy.Policy._add = _pex_policy.Policy.add

lsst.bputils.rescope(_pex_policy, globals(), ignore=("Policy", "ValidationError", "PAFWriter"))

@lsst.bputils.extend(_pex_policy.Policy)
class Policy:
    
    typeName = dict((v, _pex_policy.Policy._typeName(v)) 
                    for v in _pex_policy.Policy.ValueType.names.itervalues())

    def get(p, name):
        type = p.getValueType(name);
        if (type == p.UNDEF):
            return p.getInt(name) # will raise an exception
            # raise NameNotFound("Policy parameter name not found: " + name)

        if (type == p.INT):
            return p.getInt(name)
        elif (type == p.DOUBLE):
            return p.getDouble(name)
        elif (type == p.BOOL):
            return p.getBool(name)
        elif (type == p.STRING):
            return p.getString(name)
        elif (type == p.POLICY):
            return p.getPolicy(name)
        elif (type == p.FILE):
            return p.getFile(name)

    def getArray(p, name):
        type = p.getValueType(name);
        if (type == p.UNDEF):
            return p.getIntArray(name) # will raise an exception
            # raise NameNotFound("Policy parameter name not found: " + name)

        if (type == p.INT):
            return p.getIntArray(name)
        elif (type == p.DOUBLE):
            return p.getDoubleArray(name)
        elif (type == p.BOOL):
            return p.getBoolArray(name)
        elif (type == p.STRING):
            return p.getStringArray(name)
        elif (type == p.POLICY):
            return p.getPolicyArray(name)
        elif (type == p.FILE):
            return p.getFileArray(name)

    def set(p, name, value):
        if isinstance(value, bool):
            p._setBool(name, value)
        elif isinstance(value, int) or isinstance(value, long):
            p._setInt(name, value)
        elif (value == None):
            raise RuntimeError("Attempt to set value of \"" + name 
                               + "\" to None.  Values must be non-None.  Use remove() instead.")
    #        raise lsst.pex.exceptions.InvalidParameterException("Value of " + name + " cannot be None.")
        else:
            if name == "int_range_count":
                print "\n", value, type(value)
            p._set(name, value)

    def add(p, name, value):
        if isinstance(value, bool):
            p._addBool(name, value)
        elif isinstance(value, int) or isinstance(value, long):
            p._addInt(name, value)
        else:
            p._add(name, value)

@lsst.bputils.extend(_pex_policy.ValidationError)
class ValidationError:

    def __init__(self, *args, **kwds):
        return lsst.pex.exceptions.LogicErrorException.__init__(
            self, _pex_policy.LsstCppValidationError(*args, **kwds)
            )

    ErrorType = _pex_policy.LsstCppValidationError.ErrorType
    EMPTY = _pex_policy.LsstCppValidationError.EMPTY
    getErrorMessageFor = _pex_policy.LsstCppValidationError.getErrorMessageFor

for name, value in ValidationError.ErrorType.names.iteritems():
    setattr(ValidationError, name, value)
