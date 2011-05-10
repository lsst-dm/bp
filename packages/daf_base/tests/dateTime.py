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


import unittest

from lsst.daf.base import DateTime
import lsst.pex.exceptions as pexExcept
import time

class DateTimeTestCase(unittest.TestCase):
    """A test case for DateTime."""

    def testMJD(self):
        ts = DateTime(45205.125, DateTime.MJD, DateTime.UTC)
        self.assertEqual(ts.nsecs(DateTime.UTC), 399006000000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 399006021000000000L)
        self.assertAlmostEqual(ts.get(DateTime.MJD, DateTime.UTC), 45205.125)
        self.assertAlmostEqual(ts.get(DateTime.MJD, DateTime.TAI), 45205.125 + 21.0/86400.0)
        # Following interface is deprecated
        self.assertAlmostEqual(ts.mjd(DateTime.UTC), 45205.125)
        self.assertAlmostEqual(ts.mjd(DateTime.TAI), 45205.125 + 21.0/86400.0)

    def testNsecs(self):
        ts = DateTime(1192755473000000000L, DateTime.UTC)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1192755473000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 1192755506000000000L)
        self.assertEqual(ts.nsecs(), 1192755506000000000L)
        self.assertAlmostEqual(ts.get(DateTime.MJD, DateTime.UTC), 54392.040196759262)

    def testBoundaryMJD(self):
        ts = DateTime(47892.0, DateTime.MJD, DateTime.UTC)
        self.assertEqual(ts.nsecs(DateTime.UTC), 631152000000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 631152025000000000L)
        self.assertEqual(ts.get(DateTime.MJD, DateTime.UTC), 47892.0)

    def testCrossBoundaryNsecs(self):
        ts = DateTime(631151998000000000L, DateTime.UTC)
        self.assertEqual(ts.nsecs(DateTime.UTC), 631151998000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 631152022000000000L)

    def testNsecsTAI(self):
        ts = DateTime(1192755506000000000L, DateTime.TAI)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1192755473000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 1192755506000000000L)
        self.assertEqual(ts.nsecs(), 1192755506000000000L)
        self.assertAlmostEqual(ts.get(DateTime.MJD, DateTime.UTC), 54392.040196759262)

    def testNsecsDefault(self):
        ts = DateTime(1192755506000000000L)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1192755473000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 1192755506000000000L)
        self.assertEqual(ts.nsecs(), 1192755506000000000L)
        self.assertAlmostEqual(ts.get(DateTime.MJD, DateTime.UTC), 54392.040196759262)

    def testNow(self):
        for i in xrange(100):       # pylint: disable-msg=W0612
            secs = time.time()
            ts = DateTime.now()
            diff = ts.nsecs(DateTime.UTC) / 1.0e9 - secs 
            self.assertAlmostEqual(diff, 0, places=2)

    def testIsoEpoch(self):
        ts = DateTime("19700101T000000Z")
        self.assertEqual(ts.nsecs(DateTime.UTC), 0L)
        self.assertEqual(ts.toString(), "1970-01-01T00:00:00.000000000Z")

    def testIsoBasic(self):
        ts = DateTime("20090402T072639.314159265Z")
        self.assertEqual(ts.nsecs(DateTime.TAI), 1238657233314159265L)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1238657199314159265L)
        self.assertEqual(ts.toString(), "2009-04-02T07:26:39.314159265Z")

    def testIsoExpanded(self):
        ts = DateTime("2009-04-02T07:26:39.314159265Z")
        self.assertEqual(ts.nsecs(DateTime.TAI), 1238657233314159265L)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1238657199314159265L)
        self.assertEqual(ts.toString(), "2009-04-02T07:26:39.314159265Z")

    def testIsoNoNSecs(self):
        ts = DateTime("2009-04-02T07:26:39Z")
        self.assertEqual(ts.nsecs(DateTime.TAI), 1238657233000000000L)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1238657199000000000L)
        self.assertEqual(ts.toString(), "2009-04-02T07:26:39.000000000Z")

    def testIsoThrow(self):
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("20090401"))
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("20090401T"))
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("2009-04-01T"))
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("2009-04-01T23:36:05"))
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("20090401T23:36:05-0700"))
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("2009/04/01T23:36:05Z"))
        self.assertRaises(pexExcept.DomainErrorException, lambda: DateTime("2009/04/01T23:36:05Z"))

    def testNsecsTT(self):
        ts = DateTime(1192755538184000000L, DateTime.TT)
        self.assertEqual(ts.nsecs(DateTime.UTC), 1192755473000000000L)
        self.assertEqual(ts.nsecs(DateTime.TAI), 1192755506000000000L)
        self.assertEqual(ts.nsecs(), 1192755506000000000L)
        self.assertAlmostEqual(ts.get(DateTime.MJD, DateTime.UTC), 54392.040196759262)

    def testFracSecs(self):
        ts = DateTime("2004-03-01T12:39:45.1Z")
        self.assertEqual(ts.toString(), '2004-03-01T12:39:45.100000000Z')
        ts = DateTime("2004-03-01T12:39:45.01Z")
        self.assertEqual(ts.toString(), '2004-03-01T12:39:45.010000000Z')
        ts = DateTime("2004-03-01T12:39:45.000000001Z") # nanosecond
        self.assertEqual(ts.toString(), '2004-03-01T12:39:45.000000001Z')
        ts = DateTime("2004-03-01T12:39:45.0000000001Z") # too small
        self.assertEqual(ts.toString(), '2004-03-01T12:39:45.000000000Z')

    def testNegative(self):
        ts = DateTime("1969-03-01T00:00:32Z")
        self.assertEqual(ts.toString(), '1969-03-01T00:00:32.000000000Z')
        ts = DateTime("1969-01-01T00:00:00Z")
        self.assertEqual(ts.toString(), '1969-01-01T00:00:00.000000000Z')
        ts = DateTime("1969-01-01T00:00:40Z")
        self.assertEqual(ts.toString(), '1969-01-01T00:00:40.000000000Z')
        ts = DateTime("1969-01-01T00:00:38Z")
        self.assertEqual(ts.toString(), '1969-01-01T00:00:38.000000000Z')
        ts = DateTime("1969-03-01T12:39:45Z")
        self.assertEqual(ts.toString(), '1969-03-01T12:39:45.000000000Z')
        ts = DateTime("1969-03-01T12:39:45.000000001Z")
        self.assertEqual(ts.toString(), '1969-03-01T12:39:45.000000001Z')

        # Note slight inaccuracy in UTC-TAI-UTC round-trip
        ts = DateTime("1969-03-01T12:39:45.12345Z")
        self.assertEqual(ts.toString(), '1969-03-01T12:39:45.123449996Z')
        ts = DateTime("1969-03-01T12:39:45.123456Z")
        self.assertEqual(ts.toString(), '1969-03-01T12:39:45.123455996Z')

        ts = DateTime()
        self.assertEqual(ts.toString(), '1969-12-31T23:59:51.999918240Z')

        ts = DateTime(-1L, DateTime.TAI)
        self.assertEqual(ts.toString(), '1969-12-31T23:59:51.999918239Z')
        ts = DateTime(0L, DateTime.TAI)
        self.assertEqual(ts.toString(), '1969-12-31T23:59:51.999918240Z')
        ts = DateTime(1L, DateTime.TAI)
        self.assertEqual(ts.toString(), '1969-12-31T23:59:51.999918241Z')

        ts = DateTime(-1L, DateTime.UTC)
        self.assertEqual(ts.toString(), '1969-12-31T23:59:59.999999999Z')
        ts = DateTime(0L, DateTime.UTC)
        self.assertEqual(ts.toString(), '1970-01-01T00:00:00.000000000Z')
        ts = DateTime(1L, DateTime.UTC)
        self.assertEqual(ts.toString(), '1970-01-01T00:00:00.000000001Z')

if __name__ == '__main__':
    unittest.main()
