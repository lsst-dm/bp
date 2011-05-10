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
// LsstDataConfigurator.cc
// Implementation of class LsstDataConfigurator methods
//
// Contact: Jeff Bartels (jeffbartels@usa.net)
// 
// Created: 03-Apr-2007 5:30:00 PM
//////////////////////////////////////////////////////////////////////////////

#include <string>

#include "lsst/daf/data/LsstDataConfigurator.h"
#include "lsst/pex/logging/Trace.h"
#include <lsst/pex/policy/Policy.h>

using namespace std;

namespace lsst {
namespace daf {
namespace data {

#define EXEC_TRACE  20
static void execTrace( string s ){
    lsst::pex::logging::Trace( "daf.data.LsstDataConfigurator", EXEC_TRACE, s );
}


LsstDataConfigurator* LsstDataConfigurator::_singleton = 0;


LsstDataConfigurator& LsstDataConfigurator::the(){
    if( _singleton == 0 ){
        execTrace( "LsstDataConfigurator::the() - creating singleton");
        _singleton = new LsstDataConfigurator();
    }
    return *(_singleton);
}


LsstDataConfigurator::LsstDataConfigurator(){
    execTrace( "Enter LsstDataConfigurator::LsstDataConfigurator()");
    execTrace( "Exit LsstDataConfigurator::LsstDataConfigurator()") ;
}


LsstDataConfigurator::~LsstDataConfigurator(){
    execTrace( "Enter LsstDataConfigurator::~LsstDataConfigurator()");
    execTrace( "Exit LsstDataConfigurator::~LsstDataConfigurator()");
}


LsstDataConfigurator::LsstDataConfigurator(const LsstDataConfigurator&){
}


LsstDataConfigurator& LsstDataConfigurator::operator= (const LsstDataConfigurator&){
	return the();
}


void LsstDataConfigurator::configureSupport(
    LsstData::Ptr data,
    lsst::pex::policy::Policy::Ptr policy
){
    execTrace( 
        boost::str( 
            boost::format( "LsstDataConfigurator::configureSupport(%s, %s)")
                % data->toString() % policy->toString() ) );
    return;
}

}}} // namespace lsst::daf::data

