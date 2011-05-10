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
 
/**
 * @file DualLog.cc
 * @author Ray Plante
 */
#include "lsst/pex/logging/DualLog.h"

#include <iostream>
#include <boost/shared_ptr.hpp>

namespace lsst {
namespace pex {
namespace logging {

//@cond

using namespace std;
using boost::shared_ptr;
using lsst::daf::base::PropertySet;

///////////////////////////////////////////////////////////
//  DualLog
///////////////////////////////////////////////////////////

/*
 * create a Log that will write messages to a given file
 * @param filename      the name of the file to send messages to.  It will 
 *                        be opened in append mode.
 * @param filethresh    the importance threshold to set for the log file
 * @param screenthresh  the importance threshold to set for messages going
 *                        to the screen.
 * @param screenVerbose if true, all message data properties will be printed
 *                        to the screen.  If false, only the Log name 
 *                        ("LOG") and the text comment ("COMMENT") will be
 *                        printed.
 */
DualLog::DualLog(const PropertySet& preamble, const string& filename, 
                 int filethresh, int screenthresh, bool screenVerbose) 
    : ScreenLog(preamble, screenVerbose, screenthresh)
{
    _init(filename, filethresh);
}

DualLog::DualLog(const string& filename, 
                 int filethresh, int screenthresh, bool screenVerbose) 
    : ScreenLog(screenVerbose, screenthresh)
{
    _init(filename, filethresh);
}

void DualLog::_init(const string& filename, int filethresh) {

    // the DualLog destructor will close & destroy this.
    fstrm = new ofstream(filename.c_str(), ios_base::app);
    
    shared_ptr<LogFormatter> fmtr(new NetLoggerFormatter());

    // note that the shared_ptr held by LogDestination list will 
    // handle the deletion of this pointer.    
    _file = new LogDestination(fstrm, fmtr, filethresh);
    shared_ptr<LogDestination> dest(_file);
    _destinations.push_back( dest );
}

DualLog::~DualLog() { 
    fstrm->close();
    delete fstrm;
}

void DualLog::createDefaultLog(const PropertySet& preamble, 
                               const string& filename, int filethresh, 
                               int screenthresh, bool screenVerbose)
{
    Log::setDefaultLog(new DualLog(preamble, filename, filethresh, 
                                   screenthresh, screenVerbose));
}

void DualLog::createDefaultLog(const string& filename, int filethresh, 
                               int screenthresh, bool screenVerbose)
{
    Log::setDefaultLog(new DualLog(filename, filethresh, 
                                   screenthresh, screenVerbose));
}

//@endcond
}}} // end lsst::pex::logging

