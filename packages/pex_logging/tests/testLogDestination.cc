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
 
#include "lsst/pex/logging/LogDestination.h"
#include "lsst/pex/logging/LogRecord.h"
#include <iostream>
#include <boost/shared_ptr.hpp>

using lsst::pex::logging::LogRecord;
using lsst::pex::logging::LogFormatter;
using lsst::pex::logging::LogDestination;
using lsst::pex::logging::BriefFormatter;
using lsst::daf::base::PropertySet;
using namespace std;

void assure(bool mustBeTrue, const string& failureMsg) {
    if (! mustBeTrue)
        throw runtime_error(failureMsg);
}

int main() {

    PropertySet preamble;
    preamble.add("HOST", string("localhost.localdomain"));
    preamble.add("IP", string("111.111.111.111"));
    preamble.add("LOG", string("tester"));

    LogRecord lr1(1, 5, preamble);
    LogRecord lr2(5, 1, preamble);

    lr1.addComment("This is a test");
    lr2.addComment("This is a test");

    boost::shared_ptr<LogFormatter> brief(new BriefFormatter());

    LogDestination shy(&cerr, brief, 10);
    cerr << "Shy: " << endl;
    shy.write(lr1);
    cerr << "-------------" << endl;
    shy.write(lr2);
    cerr << "=============" << endl;

    LogDestination chatty(&cerr, brief);
    cerr << "Chatty: " << endl;
    chatty.write(lr1);
    cerr << "-------------" << endl;
    chatty.write(lr2);
    cerr << "=============" << endl;

}
