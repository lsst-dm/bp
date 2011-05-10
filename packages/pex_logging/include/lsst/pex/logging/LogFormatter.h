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
 * @file LogFormatter.h
 * @ingroup pex
 * @brief definitions of the LogFormatter.h abstract class and its 
 * implementing classes, BriefFormatter, NetLoggerFormatter
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOGFORMATTER_H
#define LSST_PEX_LOGFORMATTER_H

#include <string>
#include <map>
#include <ostream>

#include "lsst/daf/base/PropertySet.h"
#include "boost/any.hpp"

namespace lsst {
namespace pex {
namespace logging {

// forward declaration
class LogRecord;

/**
 * @brief  an abstract class for formatting log records into a text stream.
 * 
 * Implementations of this class actually write log messages to a stream 
 * in a particular format.  The messages come in as LogRecord objects, which 
 * stores its data in a PropertySet object.  The formatter can
 * expect certain properties to be given specific names.  The following are 
 * standard names:
 *
 *   Name        type      meaning 
 *    LOG        string    the name of the Log producing the message
 *    COMMENT    string    a simple text message
 *    TIMESTAMP  DateTime  the timestamp when the message was recorded
 *    DATE       string    the value of TIMESTAMP in ISO format
 *    HOST       string    the hostname of the machine
 *    IP         string    the IP address of the host
 *    PID        int       the process id of the application 
 *    NODE       int       a logical node id in a multi-process application
 * 
 * Only LOG is guaranteed to appear.  There may be multiple COMMENT properties,
 * each with a complete thought.  All other standard names should only have
 * one value associated with it.  If there are multiple values, only the last
 * one should be considered valid.  
 */
class LogFormatter {
public:

    /**
     * create a base LogFormatter
     */
    LogFormatter() { }

    /**
     * create a copy of a LogFormatter
     */ 
    LogFormatter(const LogFormatter& that) { }

    /**
     * delete the formatter
     */
    virtual ~LogFormatter();  

    /**
     * copy another formatter into this one
     */
    LogFormatter& operator=(const LogFormatter& that) { return *this; }

    /**
     * write out a log record to a stream
     * @param strm   pointer to the output stream to write the record to.  If 
     *                 the pointer is null, nothing is written.  
     * @param rec    the record to write
     */
    virtual void write(std::ostream *strm, const LogRecord &rec) = 0;

};

/**
 * @brief a formatter that renders records in a brief format for screen 
 * display.
 *
 * This formatter has a normal mode and a verbose mode.  In normal mode,
 * only the log name (LOG) and text messages (COMMENT) are printed.  In 
 * verbose mode, all other properties are printed as well.
 */
class BriefFormatter : public LogFormatter {
public:

    /**
     * create the formatter. 
     * @param verbose    all data property values will be printed.
     */
    explicit BriefFormatter(bool verbose=false) 
        : LogFormatter(), _doAll(verbose) 
    { }

    /**
     * create a copy of a LogFormatter
     */ 
    BriefFormatter(const BriefFormatter& that) 
        : LogFormatter(that), _doAll(that._doAll) 
    { }

    /**
     * delete the formatter
     */
    virtual ~BriefFormatter();

    /**
     * copy another formatter into this one
     */
    BriefFormatter& operator=(const BriefFormatter& that) { 
        if (this == &that) return *this;

        dynamic_cast<LogFormatter*>(this)->operator=(that);
        _doAll = that._doAll;
        return *this; 
    }

    /**
     * return true if all data properties will be printed or false if 
     * just the Log name ("LOG") and the text comment ("COMMENT") will 
     * be printed by default.  All properties will always be printed 
     * when the LogRecord's willShowAll() returns true. 
     */
    bool isVerbose() { return _doAll; }

    /**
     * set whether all data properties will be printed by default or
     * just the Log name ("LOG") and the text comment ("COMMENT").
     * This will be over-ridden for any LogRecord whose willShowAll() 
     * returns true.  
     * @param printAll   true if all properties should be printed.
     */
    void setVerbose(bool printAll) { _doAll = printAll; }

    /**
     * write out a log record to a stream
     * @param strm   the output stream to write the record to
     * @param rec    the record to write
     */
    virtual void write(std::ostream *strm, const LogRecord& rec);

private:

    bool _doAll;
};

/**
 * a screen-oriented formatter that indents debugging messages according 
 * to their verbosity level.  
 *
 * This class replicates the DC2 formatting of Trace messages.  
 */
class IndentedFormatter : public BriefFormatter {
public: 

    /**
     * create the formatter. 
     * @param verbose    all data property values will be printed.
     */
    explicit IndentedFormatter(bool verbose=false) 
        : BriefFormatter(verbose)
    { }

    /**
     * create a copy of a LogFormatter
     */ 
    IndentedFormatter(const IndentedFormatter& that) 
        : BriefFormatter(that)
    { }

    /**
     * delete the formatter
     */
    virtual ~IndentedFormatter();

    /**
     * copy another formatter into this one
     */
    IndentedFormatter& operator=(const IndentedFormatter& that) { 
        if (this == &that) return *this;

        dynamic_cast<BriefFormatter*>(this)->operator=(that);
        return *this; 
    }

    /**
     * write out a log record to a stream
     * @param strm   the output stream to write the record to
     * @param rec    the record to write
     */
    virtual void write(std::ostream *strm, const LogRecord& rec);
};

/**
 * \brief a formatter that renders records in a netlogger-like format.  
 * 
 * This is the format intended for use with the Event system.  
 */
class NetLoggerFormatter : public LogFormatter {
public: 
    
    /**
     * create a formatter for NetLogger-like ouput
     * @param valueDelim  the string to use as the delimiter between 
     *                      the name and the value.  The default is ":".
     * @bpdox{label:string}
     */
    explicit NetLoggerFormatter(const std::string& valueDelim = defaultValDelim);

    /**
     * create a copy
     * @bpdox{label:copy}
     */
    NetLoggerFormatter(const NetLoggerFormatter& that) 
        : LogFormatter(that), _tplookup(), _midfix(that._midfix)
    { 
        loadTypeLookup();
    }

    /**
     * delete the formatter
     */
    virtual ~NetLoggerFormatter();

    /**
     * copy another formatter into this one
     */
    NetLoggerFormatter& operator=(const NetLoggerFormatter& that);

    /**
     * return the string used to separate a property name and its value 
     * in the output stream.
     */
    const std::string& getValueDelimiter() const { return _midfix; }

    /**
     * write out a log record to a stream
     * @param strm   the output stream to write the record to
     * @param rec    the record to write
     */
    virtual void write(std::ostream *strm, const LogRecord& rec);

    static const std::string defaultValDelim;

private:
    typedef std::map<std::string, char> TypeSymbolMap;
    void loadTypeLookup();

    TypeSymbolMap _tplookup;
    std::string _midfix;
};

}}}     // end lsst::pex::logging

#endif  // end LSST_PEX_LOGFORMATTER_H
