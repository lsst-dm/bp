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
// SupportFactory.cc
// Implementation of class SupportFactory methods
//
// Contact: Jeff Bartels (jeffbartels@usa.net)
// 
// Created: 03-Apr-2007 5:30:00 PM
//////////////////////////////////////////////////////////////////////////////

#include "lsst/daf/data/SupportFactory.h"
#include "lsst/pex/logging/Trace.h"

using namespace std;

namespace lsst {
namespace daf {
namespace data {


SupportFactory* SupportFactory::_singleton = 0;


#define EXEC_TRACE  20
static void execTrace( string s, int level = EXEC_TRACE ){
    lsst::pex::logging::Trace( "daf.data.SupportFactory", level, s );
}


SupportFactory& SupportFactory::the() {
    if( _singleton == 0 )
    {
        execTrace( "SupportFactory::the() - creating singleton" );
        _singleton = new SupportFactory();
    }
    return *(_singleton);
}


SupportFactory::SupportFactory(){
    execTrace( "Enter SupportFactory::SupportFactory()" );
    execTrace( "Exit SupportFactory::SupportFactory()" );
}


SupportFactory::~SupportFactory(){
    execTrace( "Enter SupportFactory::~SupportFactory()");
    execTrace( "Exit SupportFactory::~SupportFactory()" );
}


SupportFactory::SupportFactory(const SupportFactory&){
}


SupportFactory& SupportFactory::operator= (const SupportFactory&){
    return the();
}


}}} // namespace lsst::daf::data
