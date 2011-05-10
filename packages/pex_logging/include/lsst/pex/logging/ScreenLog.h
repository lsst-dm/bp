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
 
/**
 * @file ScreenLog.h
 * @brief definition of the ScreenLog class
 * @author Ray Plante
 */
#ifndef LSST_PEX_SCREENLOG_H
#define LSST_PEX_SCREENLOG_H

#include "lsst/pex/logging/Log.h"

namespace lsst {
namespace pex {
namespace logging {

namespace dafBase = lsst::daf::base;

/**
 * @brief  a Log configured to send messages to the screen.  The 
 * BriefFormatter is used to format the messages.
 */
class ScreenLog : public Log {
public:

    /**
     * create a Log that will write messages to a given file
     * @param threshold     the importance threshold to set for messages going
     *                        to the screen.
     * @param verbose     if true, all message data properties will be printed
     *                        to the screen.  If false, only the Log name 
     *                        ("LOG") and the text comment ("COMMENT") will be
     *                        printed.
     */
    ScreenLog(bool verbose=false, int threshold=Log::INFO);

    /**
     * create a Log that will write messages to a given file
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
    ScreenLog(const dafBase::PropertySet& preamble, 
              bool verbose=false, int threshold=Log::INFO);

    /**
     * create a copy
     */
    ScreenLog(const ScreenLog& that) 
        : Log(that), _screen(that._screen), _screenFrmtr(that._screenFrmtr)
    { }

    /**
     * delete this log
     */
    virtual ~ScreenLog();

    /** 
     *  copy another ScreenLog into this one
     */
    ScreenLog& operator=(const ScreenLog& that);

    /**
     * return the importance threshold current set for the screen
     */
    int getScreenThreshold() { return _screen->getThreshold(); }

    /**
     * set the importance threshold current set for the screen
     */
    void setScreenThreshold(int thresh) { _screen->setThreshold(thresh); }

    /**
     * set whether all data properties will be printed to the screen or
     * just the Log name ("LOG") and the text comment ("COMMENT").
     * @param printAll   true if all properties should be printed.
     */
    void setScreenVerbose(bool printAll) { _screenFrmtr->setVerbose(printAll); }

    /**
     * return true if all data properties will be printed to the screen 
     * or false if just the Log name ("LOG") and the text comment 
     * ("COMMENT") will be printed.
     */
    bool isScreenVerbose() { return _screenFrmtr->isVerbose(); }

    /**
     * create a new log and set it as the default Log
     * @param threshold     the importance threshold to set for messages going
     *                        to the screen.
     * @param verbose     if true, all message data properties will be printed
     *                        to the screen.  If false, only the Log name 
     *                        ("LOG") and the text comment ("COMMENT") will be
     *                        printed.
     */
    static void createDefaultLog(bool verbose=false, int threshold=Log::INFO);

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
    static void createDefaultLog(const dafBase::PropertySet& preamble,
                                 bool verbose=false, int threshold=Log::INFO);

private:
    void configure(bool verbose);

    LogDestination *_screen;
    BriefFormatter *_screenFrmtr;
};

}}}     // end lsst::pex::logging

#endif  // end LSST_PEX_SCREENLOG_H
