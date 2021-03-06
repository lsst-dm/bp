#!/usr/bin/env python

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

import itertools
import math
import unittest

import numpy

import lsst.afw.math as afwMath
import lsst.utils.tests as utilsTests
import lsst.pex.logging as pexLog

VERBOSITY = 0 # increase to see trace

pexLog.Debug("lsst.afwMath", VERBOSITY)

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def sincpi(x):
    if abs(x) < 1.0e-15:
        return 1.0
    return math.sin(math.pi * x) / (math.pi * x)

def referenceChebyshev1(x, n):
    """Reference implementation of Chebyshev polynomials of the first kind
    
    f(x) = T_n(x)
    """
    # from Wikipedia
    if n == 0:
        return 1.0
    if n == 1:
        return x
    return (2.0 * x * referenceChebyshev1(x, n-1)) - referenceChebyshev1(x, n-2)

def referenceChebyshev1Polynomial1(x, params):
    """Reference implementation of a 1-D polynomial of Chebyshev polynomials of the first kind
    
    f(x) = params[0] T_0(x) + params[1] T_1(x) + params[2] T_2(x)
    """
    retVal = 0.0
    for ii in range(len(params)-1, -1, -1):
        retVal += params[ii] * referenceChebyshev1(x, ii)
    return retVal

def referenceChebyshev1Polynomial2(x, y, params):
    """Reference implementation of a 2-D polynomial of Chebyshev polynomials of the first kind
    
    f(x) =   params[0] T_0(x) T_0(y)                                                        # order 0
           + params[1] T_1(x) T_0(y) + params[2] T_0(x) T_1(y)                              # order 1
           + params[3] T_2(x) T_0(y) + params[4] T_1(x) T_1(y) + params[5] T_0(x) T_2(y)    # order 2
           + ...

    Raise RuntimeError if the number of parameters does not match an integer order.
    """
    retVal = 0.0
    order = 0
    y_order = 0
    for ii in range(0, len(params)):
        x_order = order - y_order
        retVal += params[ii] * referenceChebyshev1(x, x_order) * referenceChebyshev1(y, y_order)
        if x_order > 0:
            y_order += 1
        else:
            order += 1
            y_order = 0
    if y_order != 0:
        raise RuntimeError("invalid # of parameters=%d" % (len(params),))
    return retVal

class FunctionTestCase(unittest.TestCase):
    def testChebyshev1Function1D(self):
        """A test for Chebyshev1Function1D"""
        maxOrder = 6
        deltaCoeff = 0.3
        allCoeffs = numpy.arange(deltaCoeff, deltaCoeff * (maxOrder + 1) + (deltaCoeff / 2.0), deltaCoeff)
        ranges = ((-1, 1), (-1, 0), (0, 1), (-17, -2), (-65.3, 2.132))
        rangeIter = itertools.cycle(ranges)
        nPoints = 9
        
        for order in range(maxOrder + 1):
            xMin, xMax = rangeIter.next()
            xMean = (xMin + xMax) / 2.0
            xDelta = (xMax - xMin) / float(nPoints - 1)

            f = afwMath.Chebyshev1Function1D(order, xMin, xMax)
            nCoeffs = f.getNParameters()
            coeffs = numpy.arange(deltaCoeff, deltaCoeff * nCoeffs + (deltaCoeff / 2.0), deltaCoeff)
            f.setParameters(coeffs)
            g = afwMath.Chebyshev1Function1D(coeffs, xMin, xMax)
            
            self.assertEqual(f.getNParameters(), g.getNParameters())
            
            self.assertEqual(f.getMinX(), xMin)
            self.assertEqual(f.getMaxX(), xMax)

            self.assertEqual(g.getMinX(), xMin)
            self.assertEqual(g.getMaxX(), xMax)

            minXNorm = None
            maxXNorm = None
            for x in numpy.arange(xMin, xMax + xDelta/2.0, xDelta):
                xNorm = 2.0 * (x - xMean) / float(xMax - xMin)
                if minXNorm == None or xNorm < minXNorm:
                    minXNorm = xNorm
                if maxXNorm == None or xNorm > maxXNorm:
                    maxXNorm = xNorm

                predVal = referenceChebyshev1Polynomial1(xNorm, coeffs)
                if not numpy.allclose(predVal, f(x)):
                    self.fail(
                        "%s = %s != %s for x=%s, xMin=%s, xMax=%s, xNorm=%s, coeffs=%s; order constructor" % \
                        (type(f).__name__, f(x), predVal, x, xMin, xMax, xNorm, coeffs))
                if not numpy.allclose(predVal, g(x)):
                    self.fail(
                        "%s = %s != %s for x=%s, xMin=%s, xMax=%s, xNorm=%s, coeffs=%s; coeffs constructor" %\
                        (type(f).__name__, g(x), predVal, x, xMin, xMax, xNorm, coeffs))

            if not numpy.allclose((minXNorm, maxXNorm), (-1.0, 1.0)):
                raise RuntimeError(
                    "Invalid x normalization: xMin=%s, xMax=%s, min/max xNorm=(%s, %s) != (-1, 1)" %
                    (xMin, xMax, minXNorm, maxXNorm))

    def testChebyshev1Function2D(self):
        """A test for Chebyshev1Function2D"""
        maxOrder = 6
        deltaCoeff = 0.3
        ranges = ((-1, 1), (-1, 0), (0, 1), (-17, -2), (-65.3, 2.132))
        xRangeIter = itertools.cycle(ranges)
        yRangeIter = itertools.cycle(ranges)
        yRangeIter.next() # make x and y ranges off from each other
        nPoints = 7 # number of points in x and y at which to test the functions
        
        for order in range(maxOrder + 1):
            xMin, xMax = xRangeIter.next()
            xMean = (xMin + xMax) / 2.0
            xDelta = (xMax - xMin) / float(nPoints - 1)

            yMin, yMax = yRangeIter.next()
            yMean = (yMin + yMax) / 2.0
            yDelta = (yMax - yMin) / float(nPoints - 1)

            f = afwMath.Chebyshev1Function2D(order, xMin, yMin, xMax, yMax)
            nCoeffs = f.getNParameters()
            coeffs = numpy.arange(deltaCoeff, deltaCoeff * nCoeffs + (deltaCoeff / 2.0), deltaCoeff)
            f.setParameters(coeffs)
            g = afwMath.Chebyshev1Function2D(coeffs, xMin, yMin, xMax, yMax)
            
            self.assertEqual(f.getNParameters(), g.getNParameters())
            
            self.assertEqual(f.getOrder(), order)
            self.assertEqual(f.getMinX(), xMin)
            self.assertEqual(f.getMinY(), yMin)
            self.assertEqual(f.getMaxX(), xMax)
            self.assertEqual(f.getMaxY(), yMax)

            self.assertEqual(g.getOrder(), order)
            self.assertEqual(g.getMinX(), xMin)
            self.assertEqual(g.getMinY(), yMin)
            self.assertEqual(g.getMaxX(), xMax)
            self.assertEqual(g.getMaxY(), yMax)

            minXNorm = None
            maxXNorm = None
            for x in numpy.arange(xMin, xMax + xDelta/2.0, xDelta):
                xNorm = 2.0 * (x - xMean) / float(xMax - xMin)
                if minXNorm == None or xNorm < minXNorm:
                    minXNorm = xNorm
                if maxXNorm == None or xNorm > maxXNorm:
                    maxXNorm = xNorm

                minYNorm = None
                maxYNorm = None
                for y in numpy.arange(yMin, yMax + yDelta/2.0, yDelta):
                    yNorm = 2.0 * (y - yMean) / float(yMax - yMin)
                    if minYNorm == None or yNorm < minYNorm:
                        minYNorm = yNorm
                    if maxYNorm == None or yNorm > maxYNorm:
                        maxYNorm = yNorm

                        predVal = referenceChebyshev1Polynomial2(xNorm, yNorm, coeffs)
                        if not numpy.allclose(predVal, f(x, y)):
                            self.fail(
"%s = %s != %s for x=%s, xMin=%s, xMax=%s, xNorm=%s, yMin=%s, yMax=%s, yNorm=%s, coeffs=%s; order constructor" % \
(type(f).__name__, f(x, y), predVal, x, xMin, xMax, xNorm, yMin, yMax, yNorm, coeffs))
                        if not numpy.allclose(predVal, g(x, y)):
                            self.fail(
"%s = %s != %s for x=%s, xMin=%s, xMax=%s, xNorm=%s, yMin=%s, yMax=%s, yNorm=%s, coeffs=%s; coeffs constructor" % \
(type(f).__name__, g(x, y), predVal, x, xMin, xMax, xNorm, yMin, yMax, yNorm, coeffs))

                if not numpy.allclose((minYNorm, maxYNorm), (-1.0, 1.0)):
                    raise RuntimeError(
                        "Invalid y normalization: yMin=%s, yMax=%s, min/max yNorm=(%s, %s) != (-1, 1)" %
                        (yMin, yMax, minYNorm, maxYNorm))

            if not numpy.allclose((minXNorm, maxXNorm), (-1.0, 1.0)):
                raise RuntimeError(
                    "Invalid x normalization: xMin=%s, xMax=%s, min/max xNorm=(%s, %s) != (-1, 1)" %
                    (xMin, xMax, minXNorm, maxXNorm))

        # test that the number of parameters is correct for the given order
        def numParamsFromOrder(order):
            return (order + 1) * (order + 2) / 2
        MaxOrder = 13
        for order in range(MaxOrder+1):
            f = afwMath.Chebyshev1Function2D(order)
            predNParams = numParamsFromOrder(order)
            self.assertEqual(f.getNParameters(), predNParams)
            afwMath.Chebyshev1Function2D(numpy.zeros(predNParams, dtype=float))
        
        # test that the wrong number of parameters raises an exception
        validNumParams = set()
        for order in range(MaxOrder+1):
            validNumParams.add(numParamsFromOrder(order))
        for numParams in range(numParamsFromOrder(MaxOrder)):
            if numParams in validNumParams:
                continue
            self.assertRaises(Exception, afwMath.Chebyshev1Function2D, numpy.zeros(numParams, dtype=float))

    def testChebyshev1Function2DTruncate(self):
        """A test for Chebyshev1Function2D.truncate"""
        maxOrder = 6
        deltaCoeff = 0.3
        ranges = ((-1, 1), (-17, -2), (-65.3, 2.132))
        xRangeIter = itertools.cycle(ranges)
        yRangeIter = itertools.cycle(ranges)
        yRangeIter.next() # make x and y ranges off from each other
        nPoints = 7 # number of points in x and y at which to test the functions
        
        for order in range(maxOrder + 1):
            xMin, xMax = xRangeIter.next()
            xMean = (xMin + xMax) / 2.0
            xDelta = (xMax - xMin) / float(nPoints - 1)

            yMin, yMax = yRangeIter.next()
            yMean = (yMin + yMax) / 2.0
            yDelta = (yMax - yMin) / float(nPoints - 1)

            fullNParams = afwMath.Chebyshev1Function2D.nParametersFromOrder(order)
            fullCoeffs = numpy.arange(deltaCoeff, deltaCoeff * fullNParams + (deltaCoeff / 2.0), deltaCoeff)
            fullPoly = afwMath.Chebyshev1Function2D(fullCoeffs, xMin, yMin, xMax, yMax)
            
            for tooBigTruncOrder in range(order + 1, order + 3):
                self.assertRaises(Exception, fullPoly.truncate, tooBigTruncOrder)

            for truncOrder in range(order + 1):
                truncNParams = fullPoly.nParametersFromOrder(truncOrder)

                f = fullPoly.truncate(truncOrder)
                self.assertEqual(f.getNParameters(), truncNParams)

                g = afwMath.Chebyshev1Function2D(fullCoeffs[0:truncNParams], xMin, yMin, xMax, yMax)
                
                self.assertEqual(f.getNParameters(), g.getNParameters())
                
                self.assertEqual(f.getOrder(), truncOrder)
                self.assertEqual(f.getMinX(), xMin)
                self.assertEqual(f.getMinY(), yMin)
                self.assertEqual(f.getMaxX(), xMax)
                self.assertEqual(f.getMaxY(), yMax)
    
                self.assertEqual(g.getOrder(), truncOrder)
                self.assertEqual(g.getMinX(), xMin)
                self.assertEqual(g.getMinY(), yMin)
                self.assertEqual(g.getMaxX(), xMax)
                self.assertEqual(g.getMaxY(), yMax)
    
                minXNorm = None
                maxXNorm = None
                for x in numpy.arange(xMin, xMax + xDelta/2.0, xDelta):
                    xNorm = 2.0 * (x - xMean) / float(xMax - xMin)
                    if minXNorm == None or xNorm < minXNorm:
                        minXNorm = xNorm
                    if maxXNorm == None or xNorm > maxXNorm:
                        maxXNorm = xNorm
    
                    minYNorm = None
                    maxYNorm = None
                    for y in numpy.arange(yMin, yMax + yDelta/2.0, yDelta):
                        yNorm = 2.0 * (y - yMean) / float(yMax - yMin)
                        if minYNorm == None or yNorm < minYNorm:
                            minYNorm = yNorm
                        if maxYNorm == None or yNorm > maxYNorm:
                            maxYNorm = yNorm
    
                            if not numpy.allclose(f(x, y), g(x, y)):
                                self.fail(
    "%s = %s != %s = %s for x=%s, xMin=%s, xMax=%s, xNorm=%s, yMin=%s, yMax=%s, yNorm=%s, coeffs=%s; order constructor" % \
    (type(f).__name__, f(x, y), g(x, y), type(g).__name__, x, xMin, xMax, xNorm, yMin, yMax, yNorm, coeffs))
    
                    if not numpy.allclose((minYNorm, maxYNorm), (-1.0, 1.0)):
                        raise RuntimeError(
                            "Invalid y normalization: yMin=%s, yMax=%s, min/max yNorm=(%s, %s) != (-1, 1)" %
                            (yMin, yMax, minYNorm, maxYNorm))
    
                if not numpy.allclose((minXNorm, maxXNorm), (-1.0, 1.0)):
                    raise RuntimeError(
                        "Invalid x normalization: xMin=%s, xMax=%s, min/max xNorm=(%s, %s) != (-1, 1)" %
                        (xMin, xMax, minXNorm, maxXNorm))
        
    def testGaussianFunction1D(self):
        """A test for GaussianFunction1D"""
        def basicGaussian(x, sigma):
            return (1.0 / (sigma * math.sqrt(2 * math.pi))) * math.exp(-x**2 / (2.0 * sigma**2))
        
        f = afwMath.GaussianFunction1D(1.0)
        for xsigma in (0.1, 1.0, 3.0):
            f.setParameters((xsigma,))
            xdelta = xsigma / 10.0
            fSum = 0.0
            for x in numpy.arange(-xsigma * 20, xsigma * 20.01, xdelta):
                predVal = basicGaussian(x, xsigma)
                fSum += predVal
                if not numpy.allclose(predVal, f(x)):
                    self.fail("%s = %s != %s for x=%s, xsigma=%s" % \
                        (type(f).__name__, f(x), predVal, x, xsigma))
            approxArea = fSum * xdelta
            if not numpy.allclose(approxArea, 1.0):
                self.fail("%s area = %s != 1.0 for xsigma=%s" % \
                    (type(f).__name__, approxArea, xsigma))

    def testGaussianFunction2D(self):
        """A test for GaussianFunction2D
        Assumes GaussianFunction1D is correct (tested elsewhere)
        """
        f = afwMath.GaussianFunction2D(1.0, 1.0, 0.0)
        f1 = afwMath.GaussianFunction1D(1.0)
        f2 = afwMath.GaussianFunction1D(1.0)
        for sigma1 in (0.1, 1.0, 3.0):
            for sigma2 in (0.1, 1.0, 3.0):
                for angle in (0.0, 0.4, 1.1):
                    sinNegAngle = math.sin(-angle)
                    cosNegAngle = math.cos(-angle)
                    f.setParameters((sigma1, sigma2, angle))
                    f1.setParameters((sigma1,))
                    f2.setParameters((sigma2,))
                    fSum = 0.0
                    delta1 = sigma1 / 5.0
                    delta2 = sigma2 / 5.0
                    for pos1 in numpy.arange(-sigma1 * 5, sigma1 * 5.01, delta1):
                        for pos2 in numpy.arange(-sigma2 * 5.0, sigma2 * 5.01, delta2):
                            x = ( cosNegAngle * pos1) + (sinNegAngle * pos2)
                            y = (-sinNegAngle * pos1) + (cosNegAngle * pos2)
                            predVal = f1(pos1) * f2(pos2)
                            fSum += predVal
                            if not numpy.allclose(predVal, f(x, y)):
                                self.fail(
"%s = %s != %s for pos1=%s, pos2=%s, x=%s, y=%s, sigma1=%s, sigma2=%s, angle=%s" % \
(type(f).__name__, f(x, y), predVal, pos1, pos2, x, y, sigma1, sigma2, angle))
                    approxArea = fSum * delta1 * delta2
                    if not numpy.allclose(approxArea, 1.0):
                        self.fail("%s area = %s != 1.0 for sigma1=%s, sigma2=%s" % \
                            (type(f).__name__, approxArea, sigma1, sigma2))
    
    def testDoubleGaussianFunction2D(self):
        """A test for DoubleGaussianFunction2D
        Assumes GaussianFunction2D is correct (tested elsewhere)
        """
        f = afwMath.DoubleGaussianFunction2D(1.0, 1.0)
        f1 = afwMath.GaussianFunction2D(1.0, 1.0, 0.0)
        f2 = afwMath.GaussianFunction2D(1.0, 1.0, 0.0)
        for sigma1 in (1.0,):
            for sigma2 in (0.5, 2.0):
                for b in (0.0, 0.2, 2.0):
                    f.setParameters((sigma1, sigma2, b))
                    f1.setParameters((sigma1, sigma1, 0.0))
                    f2.setParameters((sigma2, sigma2, 0.0))
                    sigma1Sq = sigma1**2
                    sigma2Sq = sigma2**2
                    f1Mult = b * sigma2Sq / sigma1Sq
                    allMult = sigma1Sq / (sigma1Sq + (b * sigma2Sq))
                    fSum = 0.0
                    maxsigma = max(sigma1, sigma2)
                    minsigma = min(sigma1, sigma2)
                    delta = minsigma / 5.0
                    for y in numpy.arange(-maxsigma * 5, maxsigma * 5.01, delta):
                        for x in numpy.arange(-maxsigma * 5.0, maxsigma * 5.01, delta):
                            predVal = (f1(x, y) + (f1Mult * f2(x, y))) * allMult
                            fSum += predVal
                            if not numpy.allclose(predVal, f(x, y)):
                                self.fail("%s = %s != %s for x=%s, y=%s, sigma1=%s, sigma2=%s, b=%s" % \
                                    (type(f).__name__, f(x, y), predVal, x, y, sigma1, sigma2, b))
                    approxArea = fSum * delta**2
                    if not numpy.allclose(approxArea, 1.0):
                        self.fail("%s area = %s != 1.0 for sigma1=%s, sigma2=%s" % \
                            (type(f).__name__, approxArea, sigma1, sigma2))
    
    def testIntegerDeltaFunction2D(self):
        """A test for IntegerDeltaFunction2D"""
        def basicDelta(x, xo):
            return (x == xo)
        
        for xo in numpy.arange(-5.0, 5.0, 1.0):
            for yo in numpy.arange(-5.0, 5.0, 1.0):
                f = afwMath.IntegerDeltaFunction2D(xo, yo)
                for x in numpy.arange(-5.0, 5.0, 1.0):
                    for y in numpy.arange(-5.0, 5.0, 1.0):
                        predVal = basicDelta(x, xo) * basicDelta(y, yo)
                        if predVal != f(x, y):
                            self.fail("%s = %s != %s for x=%s, y=%s, xo=%s, yo=%s" % \
                                (type(f).__name__, f(x, y), predVal, x, y, xo, yo))
    
    def testLanczosFunction1D(self):
        """A test for LanczosFunction1D"""
        def basicLanczos1(x, n):
            return sincpi(x) * sincpi(x / float(n))

        for n in range(1, 5):
            f = afwMath.LanczosFunction1D(n)
            for xOffset in (-10.0, 0.0, 0.05):
                f.setParameters((xOffset,))
                for x in numpy.arange(-10.0, 10.1, 0.50):
                    xAdj = x - xOffset
                    predVal = basicLanczos1(xAdj, n)
                    if not numpy.allclose(predVal, f(x)):
                        self.fail("%s = %s != %s for n=%s, x=%s, xOffset=%s, xAdj=%s" % \
                            (type(f).__name__, f(x), predVal, n, x, xOffset, xAdj))

    def testLanczosFunction2D(self):
        """A test for LanczosFunction2D"""
        def basicLanczos1(x, n):
            return sincpi(x) * sincpi(x / float(n))

        for n in range(1, 5):
            f = afwMath.LanczosFunction2D(n)
            for xOffset in (-10.0, 0.0, 0.05):
                for yOffset in (-0.01, 0.0, 7.5):
                    f.setParameters((xOffset, yOffset))
                    for x in numpy.arange(-10.0, 10.1, 2.0):
                        for y in numpy.arange(-10.0, 10.1, 2.0):
                            xAdj = x - xOffset
                            yAdj = y - yOffset
                            predVal = basicLanczos1(xAdj, n) * basicLanczos1(yAdj, n)
                            if not numpy.allclose(predVal, f(x, y)):
                                self.fail("%s = %s != %s for n=%s, x=%s, " +
                                          "xOffset=%s, yOffset=%s, xAdj=%s, yAdj=%s" % 
                                          (type(f).__name__, f(x, y), predVal, n, x,
                                           xOffset, yOffset, xAdj, yAdj))
       
    def testPolynomialFunction1D(self):
        """A test for PolynomialFunction1D
        """
        def basic1DPoly(x, params):
            """1-dimensional polynomial function"""
            ii = len(params) - 1
            retVal = params[ii]
            while ii > 0:
                ii -= 1
                retVal = retVal * x + params[ii]
            return retVal
        
        maxOrder = 4
        deltaCoeff = 0.3
        allCoeffs = numpy.arange(deltaCoeff, deltaCoeff * (maxOrder + 1) + (deltaCoeff / 2.0), deltaCoeff)

        # test value using order constructor
        for order in range(maxOrder):
            coeffs = allCoeffs[0: order + 1]
            f = afwMath.PolynomialFunction1D(coeffs)
            g = afwMath.PolynomialFunction1D(order)
            g.setParameters(coeffs)
            for x in numpy.arange(-10.0, 10.1, 1.0):
                predVal = basic1DPoly(x, coeffs)
                if not numpy.allclose(predVal, f(x)):
                    self.fail("%s = %s != %s for x=%s, coeffs=%s; coeffs constructor" % \
                        (type(f).__name__, f(x), predVal, x, coeffs))
                if not numpy.allclose(predVal, g(x)):
                    self.fail("%s = %s != %s for x=%s, coeffs=%s; order constructor" % \
                        (type(f).__name__, g(x), predVal, x, coeffs))

    def testPolynomialFunction2D(self):
        """A test for PolynomialFunction2D
        """
        def basic2DPoly(x, y, params):
            """2-dimensional polynomial function"""
            retVal = 0
            nParams = len(params)
            order = 0
            ii = 0
            while True:
                for yOrder in range(order+1):
                    xOrder = order - yOrder
                    retVal += params[ii] * x**xOrder * y**yOrder
                    ii += 1
                    if ii >= nParams:
                        if xOrder != 0:
                            raise RuntimeError("invalid # params=%d" % (nParams,))
                        return retVal
                order += 1
        
        numCoeffsList = (1, 3, 6, 10)
        maxCoeffs = numCoeffsList[-1]
        deltaCoeff = 0.3
        allCoeffs = numpy.arange(deltaCoeff, deltaCoeff * (maxCoeffs + 1) + (deltaCoeff / 2.0), deltaCoeff)
        
        # test function values
        for order, numCoeffs in enumerate(numCoeffsList):
            coeffs = allCoeffs[0: numCoeffs]
            f = afwMath.PolynomialFunction2D(coeffs)
            g = afwMath.PolynomialFunction2D(order)
            g.setParameters(coeffs)
            for x in numpy.arange(-10.0, 10.1, 2.5):
                for y in numpy.arange(-10.0, 10.1, 2.5):
                    predVal = basic2DPoly(x, y, coeffs)
                    if not numpy.allclose(predVal, f(x, y)):
                        self.fail("%s = %s != %s for x=%s, y=%s, coeffs=%s; coeffs constructor" % \
                            (type(f).__name__, f(x, y), predVal, x, y, coeffs))
                    if not numpy.allclose(predVal, g(x, y)):
                        self.fail("%s = %s != %s for x=%s, y=%s, coeffs=%s; order constructor" % \
                            (type(f).__name__, g(x, y), predVal, x, y, coeffs))
        
        # test that the number of parameters is correct for the given order
        def numParamsFromOrder(order):
            return (order + 1) * (order + 2) / 2
        MaxOrder = 13
        for order in range(MaxOrder+1):
            f = afwMath.PolynomialFunction2D(order)
            predNParams = numParamsFromOrder(order)
            self.assertEqual(f.getNParameters(), predNParams)
            afwMath.PolynomialFunction2D(numpy.zeros(predNParams, dtype=float))
        
        # test that the wrong number of parameters raises an exception
        validNumParams = set()
        for order in range(MaxOrder+1):
            validNumParams.add(numParamsFromOrder(order))
        for numParams in range(numParamsFromOrder(MaxOrder)):
            if numParams in validNumParams:
                continue
            self.assertRaises(Exception, afwMath.PolynomialFunction2D, numpy.zeros(numParams, dtype=float))

    def testDFuncDParameters(self):
        """Test that we can differentiate the Function2 with respect to its parameters"""
        
        nOrder = 3
        coeffs = []
        for i in range((nOrder + 1)*(nOrder + 2)//2):
            coeffs.append(math.sin(1 + i)) # deterministic pretty-random numbers

        f = afwMath.PolynomialFunction2D(coeffs)

        for (x, y) in [(2, 1), (1, 2), (2, 2)]:
            dFdC = f.getDFuncDParameters(x, y)

            self.assertAlmostEqual(f(x, y), sum([coeffs[i]*dFdC[i] for i in range(len(coeffs))]))
                
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def suite():
    """Returns a suite containing all the test cases in this module."""
    utilsTests.init()

    suites = []
    suites += unittest.makeSuite(FunctionTestCase)
    suites += unittest.makeSuite(utilsTests.MemoryTestCase)

    return unittest.TestSuite(suites)

def run(doExit=False):
    """Run the tests"""
    utilsTests.run(suite(), doExit)

if __name__ == "__main__":
    run(True)
