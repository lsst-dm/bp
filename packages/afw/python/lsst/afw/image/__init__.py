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

"""Application Framework image-related classes including Image, Mask and MaskedImage
"""
from lsst.bputils import rescope, extend
from . import _afw_image
import numpy

rescope(_afw_image, globals(), ignore=())

suffixes = {str(numpy.uint16): "U", str(numpy.int32): "I", str(numpy.float32): "F", str(numpy.float64): "D"}

def makeImageFromArray(array):
    """Construct an Image from a NumPy array, inferring the Image type from the NumPy type.
    Return None if input is None.
    """
    if array is None: return None
    cls = globals()["Image%s" % suffixes[str(array.dtype.type)]]
    return cls(array)

def makeMaskFromArray(array):
    """Construct an Mask from a NumPy array, inferring the Mask type from the NumPy type.
    """
    if array is None: return None
    cls = globals()["Mask%s" % suffixes[str(array.dtype.type)]]
    return cls(array)

def makeMaskedImageFromArrays(image, mask=None, variance=None):
    """Construct a MaskedImage from three NumPy arrays, inferring the MaskedImage types from the NumPy types.
    """
    cls = globals()["MaskedImage%s" % suffixes[str(image.dtype.type)]]
    return cls(makeImageFromArray(image), makeMaskFromArray(mask), makeImageFromArray(variance))

def _injectMethods():

    def _MaskedImage_set(self, x, y=None, values=None):
        """Set the point (x, y) to a triple (value, mask, variance)"""

        if values is None:
            assert (y is None)
            values = x
            try:
                self.getImage().set(values[0])
                self.getMask().set(values[1])
                self.getVariance().set(values[2])
            except TypeError:
                self.getImage().set(values)
                self.getMask().set(0)
                self.getVariance().set(0)
        else:
            try:
                self.getImage().set(x, y, values[0])
                if len(values) > 1:
                    self.getMask().set(x, y, values[1])
                if len(values) > 2:
                   self.getVariance().set(x, y, values[2])
            except TypeError:
                self.getImage().set(x)
                self.getMask().set(y)
                self.getVariance().set(values)

    def _MaskedImage_get(self, x, y):
        """Return a triple (value, mask, variance) at the point (x, y)"""
        return (self.getImage().get(x, y),
                self.getMask().get(x, y),
                self.getVariance().get(x, y))

    def _MaskedImage_getArrays(self):
        """Return a tuple (value, mask, variance) numpy arrays."""
        return (self.getImage().getArray() if self.getImage() else None,
                self.getMask().getArray() if self.getMask() else None,
                self.getVariance().getArray() if self.getVariance() else None)

    for cls in (MaskedImageU, MaskedImageI, MaskedImageF, MaskedImageD):
        cls.set = _MaskedImage_set
        cls.get = _MaskedImage_get
        cls.getArrays = _MaskedImage_getArrays

    for basename in ("Image", "DecoratedImage", "MaskedImage", "Exposure"):
        for suffix in ("U", "I", "F", "D"):
            cls = getattr(_afw_image, basename + suffix)
            cls.Factory = cls

_injectMethods()
del _injectMethods
