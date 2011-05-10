/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
 
#include "lsst/pex/logging/Log.h"
#include "lsst/pex/logging/ScreenLog.h"
#include <iostream>
#include <boost/shared_ptr.hpp>

using lsst::pex::logging::Log;
using lsst::pex::logging::ScreenLog;
using lsst::pex::logging::LogDestination;
using lsst::pex::logging::LogRec;
using lsst::pex::logging::Rec;
using lsst::pex::logging::Prop;
using lsst::pex::logging::RecordProperty;
using lsst::daf::base::Citizen;
using lsst::daf::base::PropertySet;
using boost::shared_ptr;
using namespace std;

void assure(bool mustBeTrue, const string& failureMsg) {
    if (! mustBeTrue)
        throw runtime_error(failureMsg);
}

int main() {

    // test a simple message to the default log
    Log dlog = Log::getDefaultLog();
    dlog.log(Log::WARN, "this is a warning");

    // now let's create our own root log
    ScreenLog log(true);

    // test creation of child log
    Log *tlog = new Log(log, "test");
    tlog->log(Log::INFO, "I like your hat");

    // test formatting on the fly:
    const char *what = "gloves";
    tlog->format(Log::INFO, "I have %s just like those", what);

    // test threshold filtering
    tlog->setThreshold(Log::WARN);
    tlog->log(Log::INFO, "I like your gloves");  // shouldn't see this 

    // test the persistance of threshold levels
    delete tlog;
    tlog = new Log(log, "test");
    tlog->log(Log::INFO, "I like your shoes");   // shouldn't see this 
    tlog->setThreshold(Log::DEBUG);
    tlog->log(Log::INFO, "I said, I like your shoes");

    // test descendent log and ancestor's control of threshold
    Log tgclog(*tlog, "grand.child");       // name is now "test.grand.child"
    tgclog.log(Log::INFO, "Let's play", Prop<string>("STATUS", "now"));
    tlog->setThreshold(Log::FATAL);
    tgclog.addPreambleProperty("RUNID", string("testRun"));
    tgclog.log(Log::INFO, "You go first");

    // test printing of the Log tree's thresholds
    cerr << "Non-default Thresholds:" << endl;
    log.printThresholds(cerr);
    cerr << endl;

    // test streaming
    LogRec(tgclog, Log::FATAL) << "help: I've fallen" 
                               << Prop<int>("NODE", 5)
                               << "& I can't get up"
                               << LogRec::endr;

    // test flushing on delete
    Rec(tgclog, Log::FATAL) << "never mind";


    
}
