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
// Security.cc
// Implementation of class Security methods
//
// $Author::                                                                 $
// $Rev::                                                                    $
// $Date::                                                                   $
// $Id::                                                                     $
// 
// Contact: Jeff Bartels (jeffbartels@usa.net)
// 
// Created: 03-Apr-2007 5:30:00 PM
//////////////////////////////////////////////////////////////////////////////


#include "lsst/security/Security.h"
#include "lsst/daf/base/Citizen.h"
#include "lsst/pex/logging/Trace.h"

#include <string>
using namespace std;


#define EXEC_TRACE  20
static void execTrace( string s, int level = EXEC_TRACE ){
    lsst::pex::logging::Trace( "security.Security", level, s );
}


namespace lsst {
namespace security {


Security::Security() : lsst::daf::base::Citizen( typeid(this) ){
    execTrace("Enter Security::Security()");
    execTrace( boost::str( 
        boost::format( 
            "Exit Security::Security() : %s") % this->toString()));
}


Security::Security(const Security& from) : lsst::daf::base::Citizen( typeid(this) ){
    execTrace("Enter Security::Security(Security&)");
    execTrace("Exit Security::Security(Security&)");
}


Security& Security::operator= (const Security&){
    execTrace("Security::operator= (const Security&)");
    return *this;
}


Security::~Security(){
    execTrace( boost::str( 
        boost::format(
            "Enter Security::~Security() : %s") % this->toString()));
    execTrace("Exit Security::~Security()");
}


std::string Security::toString(){
    return repr();  // In Citizen
}


}} // namespace lsst::security

