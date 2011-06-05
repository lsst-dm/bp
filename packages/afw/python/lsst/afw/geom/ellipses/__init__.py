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

"""lsst.afw.geom.ellipses
"""
from . import _afw_geom_ellipses
import lsst.bputils
import weakref
import numpy

lsst.bputils.rescope(_afw_geom_ellipses, globals(), ignore=("Ellipse",))

@lsst.bputils.extend(_afw_geom_ellipses.BaseCore)
class BaseCore:

    def __str__(self):
        args = [self.getName()]
        for i, v in enumerate(self.getParameterVector()):
            k = self.ParameterEnum.values[i].name.lower()
            args.append(k)
            args.append(v)
        return "%s(%s=%s, %s=%s, %s=%s)" % tuple(args)

    __repr__ = __str__

@lsst.bputils.extend(_afw_geom_ellipses.Ellipse)
class Ellipse:

    def __str__(self):
        return "Ellipse(%s, Point%s)" % (self.core, self.center)

    __repr__ = __str__

    class EllipseMatplotlibInterface(object):
        """An interface for drawing the ellipse using matplotlib.

        This is typically initiated by calling Ellipse.plot(), which
        adds the interface as the matplotlib attribute of the ellipse
        object (this can be deleted later if desired).
        """

        def __init__(self, ellipse, scale=1.0, **kwds):
            import matplotlib.patches
            self.__ellipse = weakref.proxy(ellipse)
            self.scale = float(scale)
            core = Axes(self.__ellipse.getCore())
            core.scale(2.0 * scale)
            self.patch = matplotlib.patches.Ellipse(
                (self.__ellipse.getCenter().getX(), self.__ellipse.getCenter().getY()),
                core.getA(), core.getB(), core.getTheta() * 180.0 / numpy.pi,
                **kwds
                )

        def __getattr__(self, name):
            return getattr(self.patch, name)

        def update(self, show=True, rescale=True):
            """Update the matplotlib representation to the current ellipse parameters.
            """
            import matplotlib.patches
            core = _agl.Axes(self.__ellipse.getCore())
            core.scale(2.0 * scale)
            new_patch = matplotlib.patches.Ellipse(
                (self.__ellipse.getCenter().getX(), self.__ellipse.getCenter().getY()),
                core.a, core.b, core.theta * 180.0 / numpy.pi
                )
            new_patch.update_from(self.patch)
            axes = self.patch.get_axes()
            if axes is not None:
                self.patch.remove()
                axes.add_patch(new_patch)
            self.patch = new_patch
            if axes is not None:
                if rescale: axes.autoscale_view()
                if show: axes.figure.canvas.draw()

    def plot(self, axes=None, scale=1.0, show=True, rescale=True, **kwds):
        """Plot the ellipse in matplotlib, adding a MatplotlibInterface
        object as the 'matplotlib' attribute of the ellipse.

        Aside from those below, keyword arguments for the
        matplotlib.patches.Patch constructor are also accepted
        ('facecolor', 'linestyle', etc.)

        Arguments:
        axes -------- A matplotlib.axes.Axes object, or None to use
        matplotlib.pyplot.gca().
        scale ------- Scale the displayed ellipse by this factor.
        show -------- If True, update the figure automatically.  Set
        to False for batch processing.
        rescale ----- If True, rescale the axes.
        """
        import matplotlib.pyplot
        self.matplotlib = self.MatplotlibInterface(self, scale, **kwds)
        if axes is None:
            axes = matplotlib.pyplot.gca()
        axes.add_patch(self.matplotlib.patch)
        if rescale: axes.autoscale_view()
        if show: axes.figure.canvas.draw()
        return self.matplotlib.patch

Ellipse.center = property(Ellipse.getCenter, Ellipse.setCenter)
Ellipse.__const_proxy__.center = property(Ellipse.__const_proxy__.getCenter)
Ellipse.core = property(Ellipse.getCore, Ellipse.setCore)
Ellipse.__const_proxy__.core = property(Ellipse.__const_proxy__.getCore)

Quadrupole.matrix = property(Quadrupole.getMatrix)
Quadrupole.__const_proxy__.matrix = property(Quadrupole.__const_proxy__.getMatrix)

@lsst.bputils.extend(_afw_geom_ellipses.EllipticityBase)
class EllipticityBase:

    def __str__(self):
        return str(self.complex)

    def __repr__(self):
        return "%s(%r)" % (self.getName(), self.complex)

EllipticityBase.complex = property(EllipticityBase.getComplex, EllipticityBase.setComplex)
EllipticityBase.__const_proxy__.complex = property(EllipticityBase.__const_proxy__.getComplex)

Separable = {}

def injectProperties():
    _add_props_to = [Axes, Quadrupole, EllipticityBase]
    for r in (DeterminantRadius, TraceRadius, LogDeterminantRadius, LogTraceRadius):
        for e in (Distortion, ConformalShear, ReducedShear):
            cls = getattr(_afw_geom_ellipses, "Separable" + e.__name__ + r.__name__)
            Separable[e,r] = cls
            _add_props_to.append(cls)
            cls.ellipticity = property(cls.getEllipticity, cls.setEllipticity)
            cls.__const_proxy__.ellipticity = property(cls.__const_proxy__.getEllipticity)
        r.__str__ = float
        r.__repr__ = lambda self: "%s(%r)" % (self.getName(), self.value)

    for cls in _add_props_to:
        for name in cls.ParameterEnum.names:
            lower = name.lower()
            title = name.title()
            setattr(cls, lower, property(getattr(cls, "get" + title), getattr(cls, "set" + title)))
            setattr(cls.__const_proxy__, lower, property(getattr(cls.__const_proxy__, "get" + title)))

injectProperties()

del injectProperties
del lsst
