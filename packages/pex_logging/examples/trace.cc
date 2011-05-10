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
 
/*
 * NOTE:  Use of Trace is now deprecated.  This file demonstrates use of 
 * remaining Trace capabilities provided for backward-compatibility.  
 * 
 */

// If you want to turn off debug messages, uncomment this define before
// you include any LSST includes.
//
//#define LSST_NO_TRACE 1

#define LSST_MAX_TRACE 5     // the maximum trace level that'll be compiled in

#include "lsst/pex/logging/Trace.h"

using namespace lsst::pex::logging;

namespace {
void work() {
    std::cout << "\nThreshold levels:\n";
    Trace::printVerbosity(std::cout);
    std::cout << "traces:\n";

    Trace("foo", 1, "foo 1");
    Trace("foo.bar.goo", 4, "foo.bar.goo 4");
    Trace("foo.bar.goo", 4, boost::format("foo.bar.goo %d") % 4);
    Trace("foo.bar.goo.hoo", 3, "foo.bar.goo.hoo %d", 3);

    TTrace<3>("foo.tar", "foo.tar %d", 3);
    TTrace<5>("foo.tar", "foo.tar 5");
}
}

int main() {
    Trace::setVerbosity(".", 100);
    work();

    Trace::setVerbosity(".", 0);
    Trace::setVerbosity("foo.bar", 3);
    Trace::setVerbosity("foo.bar.goo", 10);
    Trace::setVerbosity("foo.tar", 5);
    work();

    Trace::setVerbosity("foo.tar");
    Trace::setVerbosity("foo.bar");
    work();
    
    std::cout << "\nReset.";
    Trace::reset();
    work();

    Trace::setVerbosity("", 1);
    Trace::setVerbosity("foo.bar.goo.hoo", 10);
    work();

    Trace::setVerbosity("", 2);
    work();

    Trace::setVerbosity("");
    Trace::setVerbosity("foo.bar.goo.hoo");
    Trace::setVerbosity("foo.bar.goo.hoo.joo", 10);
    Trace::setVerbosity("foo.bar.goo", 3);
    work();
    
    return 0;
}
