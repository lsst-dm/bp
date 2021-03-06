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

"""
Tests for offsetting images in (dx, dy)

Run with:
   python offsetImage.py
or
   python
   >>> import offsetImage; offsetImage.run()
"""
import math

import unittest
import numpy

import lsst.utils.tests as utilsTests
import lsst.daf.base
import lsst.afw.image as afwImage
import lsst.afw.math as afwMath
import lsst.afw.geom as afwGeom
import lsst.afw.display.ds9 as ds9
import lsst.afw.image.testUtils as imTestUtils

try:
    type(display)
except NameError:
    display = False

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class offsetImageTestCase(unittest.TestCase):
    """A test case for offsetImage"""

    def setUp(self):
        self.inImage = afwImage.ImageF(afwGeom.Extent2I(200, 100))
        self.background = 200
        self.inImage.set(self.background)
        self.algorithm = "lanczos5"

    def tearDown(self):
        del self.inImage

    def testSetFluxConvervation(self):
        """Test that flux is preserved"""

        outImage = afwMath.offsetImage(self.inImage, 0, 0, self.algorithm)
        self.assertEqual(outImage.get(50, 50), self.background)

        outImage = afwMath.offsetImage(self.inImage, 0.5, 0, self.algorithm)
        self.assertAlmostEqual(outImage.get(50, 50), self.background, 4)

        outImage = afwMath.offsetImage(self.inImage, 0.5, 0.5, self.algorithm)
        self.assertAlmostEqual(outImage.get(50, 50), self.background, 4)

    def testSetIntegerOffset(self):
        """Test that we can offset by positive and negative amounts"""
        
        self.inImage.set(50, 50, 400)

        if False and display:
            frame = 0
            ds9.mtv(self.inImage, frame=frame)
            ds9.pan(50, 50, frame=frame)
            ds9.dot("+", 50, 50, frame=frame)

        for delta in [-0.49, 0.51]:
            for dx, dy in [(2, 3), (-2, 3), (-2, -3), (2, -3)]:
                outImage = afwMath.offsetImage(self.inImage, dx + delta, dy + delta, self.algorithm)
                
                if False and display:
                    frame += 1
                    ds9.mtv(outImage, frame=frame)
                    ds9.pan(50, 50, frame=frame)
                    ds9.dot("+", 50 + dx + delta - outImage.getX0(), 50 + dy + delta - outImage.getY0(),
                            frame=frame)

    def calcGaussian(self, im, x, y, amp, sigma1):
        """Insert a Gaussian into the image centered at (x, y)"""

        x = x - im.getX0()
        y = y - im.getY0()

        for ix in range(im.getWidth()):
            for iy in range(im.getHeight()):
                r2 = math.pow(x - ix, 2) + math.pow(y - iy, 2)
                val = math.exp(-r2/(2.0*pow(sigma1, 2)))
                im.set(ix, iy, amp*val)

    def testOffsetGaussian(self):
        """Insert a Gaussian, offset, and check the residuals"""
        size = 100
        im = afwImage.ImageF(afwGeom.Extent2I(size, size))

        xc, yc = size/2.0, size/2.0

        amp, sigma1 = 1.0, 3
        #
        # Calculate an image with a Gaussian at (xc -dx, yc - dy) and then shift it to (xc, yc)
        #
        dx, dy = 0.5, -0.5
        self.calcGaussian(im, xc - dx, yc - dy, amp, sigma1)
        im2 = afwMath.offsetImage(im, dx, dy, "lanczos5")
        #
        # Calculate Gaussian directly at (xc, yc)
        #
        self.calcGaussian(im, xc, yc, amp, sigma1)
        #
        # See how they differ
        #
        if display:
            ds9.mtv(im, frame=0)

        im -= im2

        if display:
            ds9.mtv(im, frame=1)

        imArr = im.getArray()
        imGoodVals = numpy.ma.array(imArr, copy=False, mask=numpy.isnan(imArr)).compressed()
        imMean = imGoodVals.mean()
        imMax = imGoodVals.max()
        imMin = imGoodVals.min()

        if False:
            print "mean = %g, min = %g, max = %g" % (imMean, imMin, imMax)
            
        self.assertTrue(abs(imMean) < 1e-7)
        self.assertTrue(abs(imMin) < 1.2e-3*amp)
        self.assertTrue(abs(imMax) < 1.2e-3*amp)

# the following would be preferable if there was an easy way to NaN pixels
#
#         stats = afwMath.makeStatistics(im, afwMath.MEAN | afwMath.MAX | afwMath.MIN)
# 
#         if not False:
#             print "mean = %g, min = %g, max = %g" % (stats.getValue(afwMath.MEAN),
#                                                      stats.getValue(afwMath.MIN),
#                                                      stats.getValue(afwMath.MAX))
#             
#         self.assertTrue(abs(stats.getValue(afwMath.MEAN)) < 1e-7)
#         self.assertTrue(abs(stats.getValue(afwMath.MIN)) < 1.2e-3*amp)
#         self.assertTrue(abs(stats.getValue(afwMath.MAX)) < 1.2e-3*amp)

class transformImageTestCase(unittest.TestCase):
    """A test case for rotating images"""

    def setUp(self):
        self.inImage = afwImage.ImageF(afwGeom.Extent2I(20, 10))
        self.inImage.set(0, 0, 100)
        self.inImage.set(10, 0, 50)

    def tearDown(self):
        del self.inImage

    def testRotate(self):
        """Test that we end up with the correct image after rotating by 90 degrees"""

        for nQuarter, x, y in [(0, 0, 0),
                               (1, 9, 0),
                               (2, 19, 9),
                               (3, 0, 19)]:
            outImage = afwMath.rotateImageBy90(self.inImage, nQuarter)
            if display:
                ds9.mtv(outImage, frame=nQuarter, title="out %d" % nQuarter)
            self.assertEqual(self.inImage.get(0, 0), outImage.get(x, y))

    def testFlip(self):
        """Test that we end up with the correct image after flipping it"""

        frame = 2
        for flipLR, flipTB, x, y in [(True, False, 19, 0),
                                     (True, True,  19, 9),
                                     (False, True, 0,  9),
                                     (False, False, 0, 0)]:
            outImage = afwMath.flipImage(self.inImage, flipLR, flipTB)
            if display:
                ds9.mtv(outImage, frame=frame, title="%s %s" % (flipLR, flipTB))
                frame += 1
            self.assertEqual(self.inImage.get(0, 0), outImage.get(x, y))

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class binImageTestCase(unittest.TestCase):
    """A test case for binning images"""

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def testBin(self):
        """Test that we can bin images"""

        inImage = afwImage.ImageF(afwGeom.Extent2I(203, 131))
        inImage.set(1)
        bin = 4

        outImage = afwMath.binImage(inImage, bin)

        self.assertEqual(outImage.getWidth(), inImage.getWidth()//bin)
        self.assertEqual(outImage.getHeight(), inImage.getHeight()//bin)

        stats = afwMath.makeStatistics(outImage, afwMath.MAX | afwMath.MIN)
        self.assertEqual(stats.getValue(afwMath.MIN), 1)
        self.assertEqual(stats.getValue(afwMath.MAX), 1)

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def suite():
    """Returns a suite containing all the test cases in this module."""

    utilsTests.init()

    suites = []
    suites += unittest.makeSuite(offsetImageTestCase)
    suites += unittest.makeSuite(transformImageTestCase)
    suites += unittest.makeSuite(binImageTestCase)
    suites += unittest.makeSuite(utilsTests.MemoryTestCase)
    return unittest.TestSuite(suites)

def run(shouldExit=False):
    """Run the tests"""
    utilsTests.run(suite(), shouldExit)

if __name__ == "__main__":
    run(True)
