#! /usr/bin/env python

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
# A wrapper test script around testDebug
#
# The wrapped test program writes output to the screen.  This script tests
# the output by redirecting it to a file and comparing with canonical
# results.  Note that when the underlying test program is changed, the file
# containing the canonical results usually must change as well.  

import sys, os

testdir = os.path.dirname(sys.argv[0])
if len(testdir) > 0:
    sys.path.insert(0, testdir)
import verifyOutput

okay = verifyOutput.test("testDebug")
excode = 1
if okay: excode = 0
sys.exit(excode)

