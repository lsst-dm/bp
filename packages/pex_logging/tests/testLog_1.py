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

"""test simple use of the Log facility."""
"""
testLog

Run with:
   python testLog
"""

import lsst.utils.tests as tests
import lsst.pex.logging as log

if __name__ == "__main__":

    # test a simple message to the default log
    dlog = log.getDefaultLog()
    dlog.log(log.Log.WARN, "this is a warning")

    # now let's create our own root log
    logger = log.ScreenLog(1)

    # test creation of child log
    tlog = log.Log(logger, "test")
    tlog.log(log.Log.INFO, "I like your hat")

    # test threshold filtering
    tlog.setThreshold(log.Log.WARN)
    tlog.log(log.Log.INFO, "I like your gloves") #  // shouldn't see this 

    # test the persistance of threshold levels
    tlog = log.Log(logger, "test")
    tlog.log(log.Log.INFO, "I like your shoes") #   // shouldn't see this 
    tlog.setThreshold(log.Log.DEBUG)
    tlog.log(log.Log.INFO, "I said, I like your shoes")

    # test descendent log and ancestor's control of threshold
    tgclog = log.Log(tlog, "grand.child")   #   // name is now "test.grand.child"
    tgclog.log(log.Log.INFO, "Let's play")
    tlog.setThreshold(log.Log.FATAL)
    tgclog.log(log.Log.INFO, "You go first")

    # test streaming
    log.LogRec(tgclog, log.Log.FATAL) << "help: I've fallen" << log.Prop("NODE", 5) << "& I can't get up" << log.endr;
    tmp = log.Prop("NODE",5)
    log.LogRec(tgclog, log.Log.FATAL) << "help: I've fallen" << tmp << "& I can't get up" << log.endr;

    # test flushing on delete
    log.Rec(tgclog, log.Log.FATAL) << "never mind"
    log.Rec(tgclog, log.Log.DEBUG) << "testing" << log.endr;

