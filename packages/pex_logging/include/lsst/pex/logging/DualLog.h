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
 * @file Log.h
 * @brief definition of the DualLog class
 * @author Ray Plante
 */
#ifndef LSST_PEX_DUALLOG_H
#define LSST_PEX_DUALLOG_H

#include "lsst/pex/logging/ScreenLog.h"

#include <fstream>

namespace lsst {
namespace pex {
namespace logging {

namespace defBase = lsst::daf::base;

/**
 * @brief a Log that sends message to both the screen and a file. 
 *
 * This class uses the BriefFormatter for printing to the screen (via cout) 
 * and the NetLoggerFormatter for the file.  This class provides an interface 
 * for controlling the amount printed to the screen (setPrintAllProps()) as 
 * well as the thresholds on the individual destinations.  
 */
class DualLog : public ScreenLog {
public:

    /**
     * create a Log that will write messages to a given file
     * @param preamble       a list of data properties that should be included 
     *                         with every recorded message to the Log.  This
     *                         constructor will automatically add a property 
     *                         ("LOG") giving the Log name.  
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
    DualLog(const dafBase::PropertySet& preamble, const std::string& filename, 
            int filethresh=0, int screenthresh=0, bool screenVerbose=false);
            
    /**
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
    DualLog(const std::string& filename, int filethresh=0, int screenthresh=0, 
            bool screenVerbose=false);
            

    /**
     * create a copy
     */
    DualLog(const DualLog& that) 
        : ScreenLog(that), _file(that._file) 
    { }

    /**
     * delete this log
     */
    virtual ~DualLog();

    /**
     * copy another one into this
     */
    DualLog& operator=(const DualLog& that) {
        if (this == &that) return *this;
        dynamic_cast<ScreenLog*>(this)->operator=(that);
        _file = that._file; 
        return *this;
    }

    /**
     * return the importance threshold current set for the log file. 
     * This applies only to the file stream set at construction time.
     */
    int getFileThreshold() { return _file->getThreshold(); }

    /**
     * set the importance threshold current set for the log file.
     * This only affects the file stream set at construction time; other 
     * subsequently added streams are unaffected.
     */
    void setFileThreshold(int thresh) { _file->setThreshold(thresh); }

    /**
     * create a new log and set it as the default Log
     * @param preamble      a list of data properties that should be included 
     *                        with every recorded message to the Log.  This
     *                        constructor will automatically add a property 
     *                        ("LOG") giving the Log name.  
     * @param filename      the name of the file to send messages to.  It will 
     *                        be opened in append mode.
     * @param filethresh    the importance threshold to set for the log file
     * @param screenthresh  the importance threshold to set for messages going
     *                        to the screen.
     * @param screenVerbose if true, all message data properties will be 
     *                        printed to the screen.  If false, only the Log 
     *                        name ("LOG") and the text comment ("COMMENT") 
     *                        will be printed.
     */
    static void createDefaultLog(const dafBase::PropertySet& preamble,
                                 const std::string& filename, 
                                 int filethresh=Log::INHERIT_THRESHOLD, 
                                 int screenthresh=0, bool screenVerbose=false);
    /**
     * create a new log and set it as the default Log
     * @param filename      the name of the file to send messages to.  It will 
     *                        be opened in append mode.
     * @param filethresh    the importance threshold to set for the log file
     * @param screenthresh  the importance threshold to set for messages going
     *                        to the screen.
     * @param screenVerbose if true, all message data properties will be 
     *                        printed to the screen.  If false, only the Log 
     *                        name ("LOG") and the text comment ("COMMENT") 
     *                        will be printed.
     */
    static void createDefaultLog(const std::string& filename, 
                                 int filethresh=Log::INHERIT_THRESHOLD, 
                                 int screenthresh=0, bool screenVerbose=false);


private:
    void _init(const std::string& filename, int filethresh);

    LogDestination *_file;
    std::ofstream *fstrm;
};

}}}     // end lsst::pex::logging

#endif  // end LSST_PEX_DUALLOG_H
