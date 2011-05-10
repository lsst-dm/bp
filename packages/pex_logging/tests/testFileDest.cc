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
#include "lsst/pex/logging/FileDestination.h"
#include <iostream>
#include <boost/shared_ptr.hpp>

using lsst::pex::logging::Log;
using lsst::pex::logging::FileDestination;
using lsst::pex::logging::LogDestination;
using lsst::pex::logging::LogFormatter;
using lsst::pex::logging::IndentedFormatter;
using namespace std;

void assure(bool mustBeTrue, const string& failureMsg) {
    if (! mustBeTrue)
        throw runtime_error(failureMsg);
}

int main() {

    std::string filepath("tests/testFileDestination-out.txt");
    int threshold = 0;
    lsst::pex::logging::LogDestination *ld = new lsst::pex::logging::FileDestination(filepath, threshold);
    delete ld;

    boost::shared_ptr<LogFormatter> brief(new IndentedFormatter());

    boost::shared_ptr<LogDestination> 
        shy(new FileDestination("tests/testFileDestination-out.txt", true));
    Log log;
    log.addDestination(shy);
    log.log(Log::INFO, "hello");
    log.log(Log::DEBUG, "debugging");
    log.setThreshold(Log::DEBUG);
    log.log(Log::DEBUG, "hear this");
}
