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
 * @file ScreenLog.cc
 * @author Ray Plante
 */
#include "lsst/pex/logging/ScreenLog.h"

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace lsst {
namespace pex {
namespace logging {

//@cond
using boost::shared_ptr;
using lsst::daf::base::PropertySet;

///////////////////////////////////////////////////////////
//  ScreenLog
///////////////////////////////////////////////////////////

/*
 * create a Log that will write messages to a given file
 * @param threshold     the importance threshold to set for messages going
 *                        to the screen.
 * @param verbose     if true, all message data properties will be printed
 *                        to the screen.  If false, only the Log name 
 *                        ("LOG") and the text comment ("COMMENT") will be
 *                        printed.
 * @param preamble       a list of data properties that should be included 
 *                         with every recorded message to the Log.  This
 *                         constructor will automatically add a property 
 *                         ("LOG") giving the Log name.  
 */
ScreenLog::ScreenLog(const PropertySet& preamble, bool verbose, int threshold) 
    : Log(threshold), _screen(0), _screenFrmtr(0)
{
    configure(verbose);
    _preamble->combine(preamble.deepCopy());
}

/*
 * create a Log that will write messages to a given file
 * @param threshold     the importance threshold to set for messages going
 *                        to the screen.
 * @param verbose     if true, all message data properties will be printed
 *                        to the screen.  If false, only the Log name 
 *                        ("LOG") and the text comment ("COMMENT") will be
 *                        printed.
 * @param preamble       a list of data properties that should be included 
 *                         with every recorded message to the Log.  This
 *                         constructor will automatically add a property 
 *                         ("LOG") giving the Log name.  
 */
ScreenLog::ScreenLog(bool verbose, int threshold) 
    : Log(threshold), _screen(0), _screenFrmtr(0)
{
    configure(verbose);
}

void ScreenLog::configure(bool verbose) {
    // note that the shared_ptr held by the screen LogDestination will 
    // handle the deletion of this pointer.
    _screenFrmtr = new IndentedFormatter(verbose);
    shared_ptr<LogFormatter> fmtr(_screenFrmtr);

    // note that the shared_ptr held by LogDestination list will 
    // handle the deletion of this pointer.    
    _screen = new LogDestination(&clog, fmtr, INHERIT_THRESHOLD);
    shared_ptr<LogDestination> dest(_screen);
    _destinations.push_back( dest );
}

ScreenLog::~ScreenLog() { }

/*
 *  copy another ScreenLog into this one
 */
ScreenLog& ScreenLog::operator=(const ScreenLog& that) {
    if (this == &that) return *this;

    dynamic_cast<Log*>(this)->operator=(that);
    _screen = that._screen;
    _screenFrmtr = that._screenFrmtr;
    return *this;
}

/**
 * create a new log and set it as the default Log
 * @param preamble       a list of data properties that should be included 
 *                         with every recorded message to the Log.  This
 *                         constructor will automatically add a property 
 *                         ("LOG") giving the Log name.  
 * @param threshold     the importance threshold to set for messages going
 *                        to the screen.
 * @param verbose     if true, all message data properties will be printed
 *                        to the screen.  If false, only the Log name 
 *                        ("LOG") and the text comment ("COMMENT") will be
 *                        printed.
 */
void ScreenLog::createDefaultLog(const PropertySet& preamble, 
                                 bool verbose, int threshold)
{
    Log::setDefaultLog(new ScreenLog(preamble, verbose, threshold));
}

/*
 * create a new log and set it as the default Log
 * @param preamble       a list of data properties that should be included 
 *                         with every recorded message to the Log.  This
 *                         constructor will automatically add a property 
 *                         ("LOG") giving the Log name.  
 * @param threshold     the importance threshold to set for messages going
 *                        to the screen.
 * @param verbose     if true, all message data properties will be printed
 *                        to the screen.  If false, only the Log name 
 *                        ("LOG") and the text comment ("COMMENT") will be
 *                        printed.
 */
void ScreenLog::createDefaultLog(bool verbose, int threshold) {
    Log::setDefaultLog(new ScreenLog(verbose, threshold));
}

//@endcond
}}} // end lsst::pex::logging

