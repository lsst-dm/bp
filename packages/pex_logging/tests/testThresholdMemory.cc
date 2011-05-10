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
 
/**
 * @brief  This will test some LogRecord basics and LogFormatter.
 */

#include "lsst/pex/logging/threshold/Memory.h"
#include "lsst/pex/logging/LogRecord.h"
#include <iostream>
#include <sstream>

using lsst::pex::logging::threshold::Memory;
using namespace std;

#define Assert(b, m) tattle(b, m, __LINE__)

void tattle(bool mustBeTrue, const string& failureMsg, int line) {
    if (! mustBeTrue) {
        ostringstream msg;
        msg << __FILE__ << ':' << line << ":\n" << failureMsg << ends;
//        throw runtime_error(msg.str());
        cout << msg.str() << endl;
    }
}

using lsst::pex::logging::LogRecord;
namespace Threshold = lsst::pex::logging::threshold;

int main() {

    Threshold::Memory mem;

    mem.setRootThreshold(5);
    Assert(mem.getRootThreshold() == 5, "wrong root threshold");

    mem.setThresholdFor("valley.of.the.dolls", -2);
    Assert(mem.getThresholdFor("valley.of.the.dolls") == -2, 
           "wrong explicit threshold");
    Assert(mem.getThresholdFor("valley.of.the") == 5, 
           "wrong inherited threshold");

    long long t0=0L, t1=0L;
    t0 = LogRecord::utcnow();
    Assert(mem.getThresholdFor("valley.of.the.dolls.movie") == -2, 
           "wrong not-yet-set threshold");
    t1 = LogRecord::utcnow();
    cout << "got threshold in " << (t1-t0)/1000L << " usecs" << endl;

    mem.setThresholdFor("valley.of", -11);
    Assert(mem.getThresholdFor("valley.of.the") == -11, 
           "wrong new inherited threshold");

    mem.printThresholds(cout);
}
