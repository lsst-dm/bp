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
// lsstdata01.cc
//      lsst::daf::data classes sanity check program (see comments below)
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


// 
// lsstdata01.cc demonstrates specialization on LsstBase, 
// object creation and configuration, exercising the factories, 
// the required lsst/daf/data includes, use of trace verbosity to control debug 
// output
//
// This program should be built by scons when run from the overlying
//    directory.
// Run this program to do a sanity check on the lsst::daf::data classes. 
// Examine the output of the program for the following:
// 1) Proper pairing of constructors/destructors, 
// 2) Only one initialization of singleton classes SupportFactory and
//    LsstDataConfigurator
// 3) The last item output should be the message "No memory leaks detected"
// 4) The program should run with no errors. 
// 5) You can specify a trace verbosity level with the command line (default = 100)
// 
// With that check out of the way, you are ready to use the lsst::daf::data classes to 
// develop your LsstBase derivations and application code
//
// SAMPLE OUTPUT (verbosity=100):
//
// Explicitly creating a policy object
//                     SupportFactory::the() - creating singleton
//                     Enter SupportFactory::SupportFactory()
//                     Exit SupportFactory::SupportFactory()
//                     Enter SupportFactory::createPolicy()
//                     Enter Policy::Policy()
//                     Exit Policy::Policy() : 1: 0x985454c lsst::pex::policy::Policy
//                     Exit SupportFactory::createPolicy()
// Done: Created policy object '1: 0x985454c lsst::pex::policy::Policy'
// Creating an LsstData realization
//                     Enter LsstImpl_DC2::LsstImpl_DC2(P6MyLsst)
//                     Enter SupportFactory::createMetadata()
//                     Enter Metadata::Metadata()
//                     Exit Metadata::Metadata() : 3: 0x98545bc lsst::daf::data::Metadata
//                     Exit SupportFactory::createMetadata()
//                     Enter SupportFactory::createPersistence()
//                     Enter Persistence::Persistence()
//                     Exit Persistence::Persistence() : 4: 0x98548ec lsst::daf::data::Persistence
//                     Exit SupportFactory::createPersistence()
//                     Enter SupportFactory::createPolicy()
//                     Enter Policy::Policy()
//                     Exit Policy::Policy() : 5: 0x98547bc lsst::pex::policy::Policy
//                     Exit SupportFactory::createPolicy()
//                     Enter SupportFactory::createProvenance()
//                     Enter Provenance::Provenance()
//                     Exit Provenance::Provenance() : 6: 0x9854614 lsst::daf::data::Provenance
//                     Exit SupportFactory::createProvenance()
//                     Enter SupportFactory::createReleaseProcess()
//                     Enter ReleaseProcess::ReleaseProcess()
//                     Exit ReleaseProcess::ReleaseProcess() : 7: 0x985457c lsst::daf::data::ReleaseProcess
//                     Exit SupportFactory::createReleaseProcess()
//                     Enter SupportFactory::createSecurity()
//                     Enter Security::Security()
//                     Exit Security::Security() : 8: 0x9854714 lsst::daf::data::Security
//                     Exit SupportFactory::createSecurity()
//                     Exit LsstImpl_DC2::LsstImpl_DC2() : 2: 0x9854664 MyLsst
//           In MyLsst::MyLsst(An LsstData realization)
//  Done: Created MyLsst object '2: 0x936f754 MyLsst'
//  Configuring the LsstData object
//                     LsstDataConfigurator::configureSupport(1: 0x985454c lsst::pex::policy::Policy)
//  Configured MyLsst object '2: 0x9854664 MyLsst'
//                     Enter LsstImpl_DC2::getChildren(1)
//                     Exit LsstImpl_DC2::getChildren()
//  OK: MyLsst::getChildren returned 0
//  Destroy MyLsst object '2: 0x9854664 MyLsst'
//           MyLsst::~MyLsst()
//                     Enter LsstImpl_DC2::~LsstImpl_DC2() : 2: 0x9854664 MyLsst
//                     Exit LsstImpl_DC2::~LsstImpl_DC2()
//                     Enter Security::~Security() : 8: 0x9854714 lsst::daf::data::Security
//                     Exit Security::~Security()
//                     Enter ReleaseProcess::~ReleaseProcess() : 7: 0x985457c lsst::daf::data::ReleaseProcess
//                     Exit ReleaseProcess::~ReleaseProcess()
//                     Enter Provenance::~Provenance() : 6: 0x9854614 lsst::daf::data::Provenance
//                     Exit Provenance::~Provenance()
//                     Enter Policy::~Policy() : 5: 0x98547bc lsst::pex::policy::Policy
//                     Exit Policy::~Policy()
//                     Enter Persistence::~Persistence() : 4: 0x98548ec lsst::daf::data::Persistence
//                     Exit Persistence::~Persistence()
//                     Enter Metadata::~Metadata() : 3: 0x98545bc lsst::daf::data::Metadata
//                     Exit Metadata::~Metadata()
//  Done destroying MyLsst object
//  Policy object '1: 0x985454c lsst::pex::policy::Policy' going out of scope
//                     Enter Policy::~Policy() : 1: 0x985454c lsst::pex::policy::Policy
//                     Exit Policy::~Policy()
//  No leaks detected
//
 
#include <iostream>
#include <string>
#include <boost/format.hpp>

#include <lsst/daf/base/Citizen.h>
#include "lsst/daf/data/LsstBase.h"
#include "lsst/daf/data/SupportFactory.h"
#include "lsst/daf/data/LsstDataConfigurator.h"
#include "lsst/pex/logging/Trace.h"
#include <lsst/pex/policy/Policy.h>

using namespace std;
using namespace lsst::daf::data;
using lsst::pex::logging::Trace;

class MyLsst : public LsstBase
{
public:
    typedef boost::shared_ptr<MyLsst> Ptr;
    MyLsst(string s);
    virtual ~MyLsst();
};

MyLsst::MyLsst(string s) : LsstBase(typeid(this)) 
{ 
	Trace("lsstdata01.MyLsst", 10, boost::format("In MyLsst::MyLsst(%s)") % s);
}
MyLsst::~MyLsst()
{
	Trace("lsstdata01.MyLsst", 10, "MyLsst::~MyLsst()");
}

int main( int argc, char** argv )
{
    int verbosity = 100;

    if( argc > 1 )
    {
        try{
            int x = atoi(argv[1]);
            verbosity = x;
        }    
        catch(...)
        {
            verbosity = 0;
        }
    }
    
    Trace::setVerbosity("",verbosity);
    {
        //
        // Create a free Policy object to use for LsstData object 
        //    configuration.
        //
        Trace("lsstdata01", 1, "Explicitly creating a policy object");
        lsst::pex::policy::Policy::Ptr sp = lsst::pex::policy::Policy::Ptr(
            new lsst::pex::policy::Policy());
        Trace("lsstdata01", 1, 
            boost::format("Created policy object '%s'") % sp.get()->toString());
         
        //
        // Create a new instance of an LsstData realization
        // Configure the instance via a given Policy object
        //
        Trace("lsstdata01", 1, "Creating an LsstData realization");
        MyLsst::Ptr x = MyLsst::Ptr(new MyLsst( "An LsstData realization" ));
        Trace("lsstdata01", 1, 
            boost::format("Done: Created MyLsst object '%s'") % x->toString());

        Trace("lsstdata01", 1, "Configuring the LsstData object");
        LsstDataConfigurator::configureSupport(x,sp);
        Trace("lsstdata01", 1, 
            boost::format("Done: Configured MyLsst object '%s'") % x->toString());

        //
        // Exercise the configured LsstData realization via the default
        // methods on the base object
        //
        
        LsstData::IteratorRange range = x->getChildren();
        if( std::distance( range.first, range.second) != 0 ) {
            Trace("lsstdata01", 1,
                "Error: MyLsst object is a simple object and does not " \
                "collect children. getChildren should return an iterator range of 0");
        }
        else {
            Trace("lsstdata01", 1,
                "OK: MyLsst::getChildren returned an interator range of 0");
        }
            
 
        //
        // Now destroy the configured LsstData realization to trigger
        // all of the base class' destructors and generate trace output 
        //          

        Trace("lsstdata01", 1, 
            boost::format("MyData object '%s' going out of scope") \
                % x->toString());
        Trace("lsstdata01", 1, 
            boost::format("Policy object '%s' going out of scope") \
                % sp->toString());
    }
    //
    // Check for memory leaks
    //
    if (lsst::daf::base::Citizen::census(0) == 0) {
        Trace("lsstdata01", 1, "No leaks detected");
    } else {
        Trace("lsstdata01", 1, "ERROR: Memory leaks detected!");
        lsst::daf::base::Citizen::census(cerr);
    }

    return 0;
}


