#!/usr/bin/env python
"""
Tests of the BlockTestingLog
"""
from __future__ import with_statement

import pdb                              # we may want to say pdb.set_trace()
import os
import sys
import unittest
import time
import copy
import random

from lsst.pex.logging import Log, BlockTimingLog, LogRecord

class BlockTimingLogTestCase(unittest.TestCase):

    def setUp(self):
        self.log = BlockTimingLog(Log.getDefaultLog(), "test")
    def tearDown(self):
        pass

    def testINSTRUMValue(self):
        self.assertEquals(BlockTimingLog.INSTRUM, -3)

    def testCreate(self):
        btl = self.log.createForBlock("api")
        self.assertEquals(btl.getInstrumentationLevel(), BlockTimingLog.INSTRUM)
        self.assertEquals(btl.getFunctionName(), "api")

        btl = self.log.createForBlock("api", BlockTimingLog.INSTRUM+1)
        self.assertEquals(btl.getInstrumentationLevel(), -2)

    def testBlock(self):
        log = BlockTimingLog(self.log, "test")
        log.setThreshold(BlockTimingLog.INSTRUM)
        btl = log.createForBlock("api")
        btl.log(Log.INFO, "message")
        btl.done()
        del btl

    def testStart(self):
        log = BlockTimingLog(self.log, "test")
        log.setThreshold(BlockTimingLog.INSTRUM)
        log.start("Test run")
        log.done()

    def testSysData(self):
        lr = LogRecord(0, 0, True)
        self.log.setUsageFlags(self.log.SUTIME)
        self.log.addUsageProps(lr)
        ps = lr.getProperties();
        self.assert_(ps.exists("usertime"))
        self.assert_(ps.exists("systemtime"))
        self.assert_(not ps.exists("nswap"))

        lr = LogRecord(0, 0, True)
        self.log.setUsageFlags(self.log.ALLUDATA)
        self.log.addUsageProps(lr)
        ps = lr.getProperties();
        self.assert_(ps.exists("usertime"))
        self.assert_(ps.exists("systemtime"))
        self.assert_(ps.exists("maxrss"))
        self.assert_(ps.exists("nswap"))
        self.assert_(ps.exists("blocksin"))
        self.assert_(ps.exists("blocksout"))

__all__ = "BlockTimingLogTestCase".split()

if __name__ == "__main__":
    unittest.main()

        
