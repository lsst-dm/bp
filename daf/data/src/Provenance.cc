// -*- lsst-c++ -*-

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
 
//////////////////////////////////////////////////////////////////////////////
// Provenance.cc
// Implementation of class Provenance methods
//
// Contact: Jeff Bartels (jeffbartels@usa.net)
// 
// Created: 03-Apr-2007 5:30:00 PM
//////////////////////////////////////////////////////////////////////////////


#include "lsst/daf/base/Citizen.h"
#include "lsst/daf/data/Provenance.h"
#include "lsst/pex/logging/Trace.h"

#include <string>
using namespace std;


#define EXEC_TRACE  20
static void execTrace( string s, int level = EXEC_TRACE ){
    lsst::pex::logging::Trace( "daf.data.Provenance", level, s );
}


namespace lsst {
namespace daf {
namespace data {


Provenance::Provenance() : lsst::daf::base::Citizen( typeid(this) ){
    execTrace("Enter Provenance::Provenance()");
    execTrace( boost::str( 
        boost::format( 
            "Exit Provenance::Provenance() : %s") % this->toString()));
}


Provenance::Provenance(const Provenance& from) : lsst::daf::base::Citizen( typeid(this) ){
   execTrace("Enter Provenance::Provenance(Provenance&)");
   execTrace("Exit Provenance::Provenance(Provenance&)");
}


Provenance& Provenance::operator= (const Provenance&){
    execTrace("Provenance::operator= (const Provenance&)");
    return *this;
}


Provenance::~Provenance(){
    execTrace( boost::str( 
        boost::format(
            "Enter Provenance::~Provenance() : %s") % this->toString()));
    execTrace("Exit Provenance::~Provenance()");
}


std::string Provenance::toString(){
    return repr();  // In Citizen
}


} // namespace data
} // namespace daf
} // namespace lsst

