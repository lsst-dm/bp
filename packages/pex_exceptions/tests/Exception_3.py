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
Tests for ticket656

Run with:
   python ticket656.py
or
   python
   >>> import ticket656; ticket656.run()
"""

import unittest
import failer

class Ticket656TestCase(unittest.TestCase):
    """A test case for Measure"""
    def setUp(self):
        """Create a wrapped C++ instance that throws an exception."""
        self.x = failer.Failer()

    def tearDown(self):
        del self.x

    def testFail2(self):
        try:
            self.x.fail2()
        except Exception, e:
            print e.__class__
            print dir(e)
            print len(e.args)
            print e
            
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
def run():
    unittest.main()

if __name__ == "__main__":
    run()
