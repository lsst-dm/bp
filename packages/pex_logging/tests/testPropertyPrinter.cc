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
 
#include "lsst/pex/logging/PropertyPrinter.h"
#include <iostream>
#include <vector>
#include <stdexcept>

using lsst::pex::logging::PropertyPrinter;
using lsst::daf::base::PropertySet;
using namespace std;

void assure(bool mustBeTrue, const string& failureMsg) {
    if (! mustBeTrue)
        throw runtime_error(failureMsg);
}

int main() {

    PropertySet ps;
    ps.set("count", 4);
    ps.set("done", true);
    ps.set("name", "Ray");

    std::vector<std::string>::iterator ni;
    std::vector<std::string> names(ps.paramNames(true));

    for(ni = names.begin(); ni != names.end(); ++ni) {
        PropertyPrinter pp(ps, *ni);
        for(PropertyPrinter::iterator i=pp.begin(); i.notAtEnd(); ++i) {
            cout << *ni << ": ";
            i.write(&cout) << endl;
        }
    }

    return 0;
}
    
