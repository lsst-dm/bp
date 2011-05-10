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
#include <exception>
#include <boost/shared_ptr.hpp>

using lsst::pex::logging::Log;
using lsst::daf::base::Citizen;

int main() {
    
    if (true) {
        Log dlog = Log::getDefaultLog();
    }

    if (Citizen::census(0) != 0) {
        Citizen::census(std::cout);
        throw std::runtime_error("Logging left active Citizen");
    }

}
