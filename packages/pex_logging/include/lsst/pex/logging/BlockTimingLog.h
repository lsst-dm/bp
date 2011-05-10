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
 * @file BlockTimingLog.h
 * @brief definition of the BlockTimingLog class
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOGGING_BLOCKTIMINGLOG_H
#define LSST_PEX_LOGGING_BLOCKTIMINGLOG_H

#include "lsst/pex/logging/LogRecord.h"
#include "lsst/pex/logging/Log.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <boost/scoped_ptr.hpp>

namespace lsst {
namespace pex {
namespace logging {

/**
 * a specialized Log that assists the harness in tracing execution flow.
 *
 * The motivation for this class is to provide uniformity in the log 
 * messages that indicate the start and finish of some section of code.  
 * This makes it easier to locate these records after execution and
 * calculate the time spent in the block of code.  
 * 
 * This class can optionally be used to simultaneously capture usage data.
 * In particular, it can add as properties the following system informtation:
 * user cpu time, system cpu time, memory usage (as maximum resident size),
 * the number of swaps, the number of block input operations, and the number 
 * of output operations.  Which of these are save with the log message 
 * is controlled by a bit map.  
 */
class BlockTimingLog : public Log {
public:

    /**
     * an enumeration for controlling which usage data to collect
     */
    enum usageData { 
        /**
         * flag to indicate that no usage data should be captured
         */
        NOUDATA = 0,

        /**
         * flag to enable collecting the usage datum, user time 
         */
        UTIME = 1,

        /**
         * flag to enable collecting the usage datum, system time 
         */
        STIME = 2,

        /**
         * flag to enable collecting the usage datum, user and system time.
         * This is equal to UTIME|STIME.
         */
        SUTIME = 3,

        /**
         * flag to enable collecting the usage datum, memory size (as
         * max resident size)
         */
        MEMSZ = 4,

        /**
         * flag to enable collecting the usage datum, number of swaps.
         */
        NSWAP = 16,

        /**
         * flag to enable collecting the usage datum, number of block reads.
         */
        BLKIN = 32,

        /**
         * flag to enable collecting the usage datum, number of block writes.
         */
        BLKOUT = 64,

        /**
         * flag to enable collecting the usage data, the number of block 
         * reads and writes.  This is equal to BLKIN|BLKOUT
         */
        BLKIO = 96,

        /**
         * flag to enable collecting the usage datum, the number of minor 
         * page faults since the start of the process.
         */
        MINFLT = 128, 

        /**
         * flag to enable collecting the usage datum, the number of major 
         * page faults since the start of the process.
         */
        MAJFLT = 256, 

        /**
         * flag to enable collecting the usage data supported by Linux
         * (as of kernal version 2.6):  SUTIME|MINFLT|MAJFLT
         */
        LINUXUDATA = 387,

        /**
         * flag to enable collecting all usage data
         */
        ALLUDATA = 511,

        /**
         * flag to indicate that the usages flags should be inherited from
         * the parent log.  
         */
        PARENTUDATA = 8192
    };

    /**
     * default message level for messages that instrument execution flow
     */
    static const int INSTRUM;  // = Log::INFO - 3

    /**
     * the property name that will indicate what an instrumenting message 
     * is marking
     */
    static const std::string STATUS;

    /**
     * the property to use to mark the start of a block of code being 
     * instrumented
     */
    static const std::string START;

    /**
     * the property to use to mark the start of a block of code being 
     * instrumented
     */
    static const std::string END;

    /**
     * construct a BlockTimingLog.  
     * 
     * @param parent     the parent log to inherit attributes from 
     * @param name       the name to give the log relative to its parent
     * @param tracelev   the default level to use for the tracing messages
     * @param funcName   the name of the block of code (e.g. function) being 
     *                      traced.  If empty, the value given by name will 
     *                      be used instead.  This value will be used in the 
     *                      log message only.
     * @param usageFlags a or-ed list of usageData values that indicate
     *                      which usage data to capture.
     */
    BlockTimingLog(const Log& parent, const std::string& name, 
                   int tracelev=BlockTimingLog::INSTRUM, 
                   int usageFlags=BlockTimingLog::PARENTUDATA, 
                   const std::string& funcName="");

    /**
     * create a copy of a BlockTimingLog
     */
    BlockTimingLog(const BlockTimingLog& that) 
        : Log(*this), _tracelev(that._tracelev), _funcName(that._funcName)
    { }

    /**
     * reset this instance to another
     */
    BlockTimingLog& operator=(const BlockTimingLog& that) {
        Log::operator=(that);
        _tracelev = that._tracelev;
        _funcName = that._funcName;
        return *this;
    }

    /**
     * return an OR-ed list of flags indicating the usage data this object
     * is set to collect with each message. 
     */
    int getUsageFlags() const { return _usageFlags; }

    /**
     * set the usage data that will be collected.
     * @param flags   an integer of OR-ed usageData values for the specific 
     *                   data that should be captured.  Use NOUDATA to turn
     *                   off the capturing of this data.  Use PARENTUDATA 
     *                   to revert to the parent log's usage data flags.
     */
    void setUsageFlags(int flags) { 
        _usageFlags = flags; 
        if ((flags & PARENTUDATA) > 0) _usageFlags |= _pusageFlags;
    }

    /**
     * add to the list of usage data that will be collected.  Previously 
     * set values will be preserved.
     * @param flags   an integer of OR-ed usageData values for the specific 
     *                   data that should be captured.  Use PARENTUDATA 
     *                   to include the parent log's usage data flags (what
     *                   ever they were when this log was created).
     */
    void addUsageFlags(int flags) { 
        _usageFlags |= flags; 
        if ((flags & PARENTUDATA) > 0) _usageFlags |= _pusageFlags;
    }

    /**
     * create and return a new child that should be used while tracing a 
     * function.  A "start" message will be logged to the new log as part
     * of the call.  Like createChildLog(), the caller is responsible for
     * deleting this new instance when finished with it.
     * @param name       the name to give the log relative to its parent
     * @param tracelev   the default level to use for the tracing messages.
     *                      If equal to Log::INHERIT_THRESHOLD (default),
     *                      it will be set to this log's tracing level.
     * @param funcName   the name of the block of code (e.g. function) being 
     *                      traced.  If empty, the value given by name will 
     *                      be used instead.  This value will be used in the 
     *                      log message only.
     */
    BlockTimingLog *createForBlock(const std::string& name, 
                                   int tracelev=Log::INHERIT_THRESHOLD,
                                   const std::string& funcName="") 
    {
        BlockTimingLog *out = new BlockTimingLog(*this, name, tracelev, 
                                                 PARENTUDATA, funcName);
        // out->setShowAll(true);
        out->start();
        return out;
    }

    /**
     * a synonym for createForBlock
     */
    BlockTimingLog *timeBlock(const std::string& name, 
                               int tracelev=Log::INHERIT_THRESHOLD,
                               const std::string& funcName="") 
    {
        return createForBlock(name, tracelev, funcName);
    }

    /**
     * delete the log
     */
    virtual ~BlockTimingLog();

    /**
     * Indicate that the section of code being traced (e.g. a function body)
     * is starting.
     */
    void start() {
        if (sends(_tracelev)) {
            std::string msg("Starting ");
            msg += _funcName;

            LogRecord rec(getThreshold(), _tracelev, getPreamble(), 
                          willShowAll());
            rec.addComment(msg);
            rec.addProperty(STATUS, START);
            if (_usageFlags) addUsageProps(rec);
            send(rec);
        }
    }

    /**
     * Indicate that the section of code being instrumented (e.g. a function body)
     * is starting.
     */
    void start(const std::string& funcName) {
        if (funcName.length() > 0) _funcName = funcName;
        start();
    }

    /**
     * Indicate that the section of code being instrumented (e.g. a function body)
     * is finished.
     */
    void done() {
        if (sends(_tracelev)) {
            std::string msg("Ending ");
            msg += _funcName;

            LogRecord rec(getThreshold(), _tracelev, getPreamble(), 
                          willShowAll());
            rec.addComment(msg);
            rec.addProperty(STATUS, END);
            if (_usageFlags) addUsageProps(rec);
            send(rec);
        }
    }

    /**
     * return the instrumenting message level, the importance to be given to the 
     * start and end messages.
     */
    int getInstrumentationLevel() const { return _tracelev; }

    /**
     * return the name of the code block being traced which will appear
     * in the start and end messages.  
     */
    const std::string& getFunctionName() const { return _funcName; }

    /**
     * add usage properties to a given LogRecord according the currently
     * set usage flags.
     */
    void addUsageProps(LogRecord& rec);

private:
    int _tracelev;
    int _pusageFlags, _usageFlags;
    std::string _funcName;
    boost::scoped_ptr<struct rusage> _usage;
};

}}}     // end lsst::pex::logging
#endif  // end LSST_PEX_BLOCKTIMINGLOG_H
