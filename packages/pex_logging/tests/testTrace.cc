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
 
#define LSST_MAX_TRACE 3

#include "lsst/pex/logging/Trace.h"
#include "lsst/pex/logging/DualLog.h"
#include <iostream>
#include <time.h>

using lsst::pex::logging::LogRecord;
using lsst::pex::logging::Log;
using lsst::pex::logging::DualLog;
using lsst::pex::logging::Trace;
namespace lg = lsst::pex::logging;
using namespace std;

void assure(bool mustBeTrue, const string& failureMsg) {
    if (! mustBeTrue)
        throw runtime_error(failureMsg);
}

int main(int argc, char* argv[]) {
    long long t0, t1;
    int verb = LSST_MAX_TRACE;

    cout << "overhead of time call: " 
         << (LogRecord::utcnow() - LogRecord::utcnow())/-1000 << " usecs."
         << endl;

    if (argc > 1) {
        DualLog::createDefaultLog(string(argv[1]), -2);
    }
    if (argc > 2) verb = atoi(argv[2]);

    Trace::setVerbosity("myapp", verb);

    t0 = LogRecord::utcnow();
    Trace("myapp", 1, "I'm starting this routine");
    t1 = LogRecord::utcnow();
    cout << "message printed in " << (t1-t0)/1000 << " usesc" << endl;

    t0 = LogRecord::utcnow();
    Trace("myapp.foo.bar", 2, "Testing arbitrary verbosity.");
    t1 = LogRecord::utcnow();
    cout << "Trace message printed in " << (t1-t0)/1000 << " usesc" << endl;

    t0 = LogRecord::utcnow();
    Trace("myapp.foo.bar", 5, "Testing arbitrary verbosity.");
    t1 = LogRecord::utcnow();
    cout << "Trace message not printed in " <<(t1-t0)/1000 << " usesc" << endl;

    t0 = LogRecord::utcnow();
    lg::TTrace<2>("myapp", "Testing arbitrary verbosity again.");
    t1 = LogRecord::utcnow();
    cout << "TTrace message printed in " << (t1-t0)/1000 << " usesc" << endl;
    
    t0 = LogRecord::utcnow();
    lg::TTrace<2>("myapp", "Testing arbitrary verbosity again.");
    t1 = LogRecord::utcnow();
    cout << "TTrace message printed in " << (t1-t0)/1000 << " usesc" << endl;
    

    if (argc > 1) {
        t0 = LogRecord::utcnow();
        lg::TTrace<2>("myapp", "Testing file writing at verbosity 2.");
        t1 = LogRecord::utcnow();
        cout << "Cost of printing to file: " << (t1-t0)/1000 
             << " usesc" << endl;
    }

    Trace("myapp.foo.bar", 4, "Testing verbosity past limit.");

    t0 = LogRecord::utcnow();
    for (int k=0; k < 10; k++)
        lg::TTrace<4>("myapp", "Testing verbosity past limit again.");
    t1 = LogRecord::utcnow();
    cout << "TTrace message not printed in " << (t1-t0)/10000 << " usesc" <<endl;
    
    char msg[111];
    for(int i=0; i < 110; i++) msg[i] = '0' + (i % 10);
    msg[110] = '\0';
    lg::TTrace<3>("myapp", "Long message: %s", msg);
}
