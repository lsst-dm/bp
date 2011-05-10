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
 * @file LogDestination.h
 * @brief definition of the LogDestination class
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOGDESTINATION_H
#define LSST_PEX_LOGDESTINATION_H

#include "lsst/pex/logging/LogFormatter.h"
#include "lsst/pex/logging/threshold/enum.h"

#include <string>
#include <ostream>
#include <boost/shared_ptr.hpp>

namespace lsst {
namespace pex {
namespace logging {

// forward declaration of LogRecord
class LogRecord;

/**
 * @brief an encapsulation of a logging stream that will filter messages
 * based on their volume (importance) level.  
 *
 * Multiple destinations can be added to a Log either at its
 * contruction time or later using one of its addDestination()
 * methods.  A LogDestination can be added to multiple Logs; however,
 * actual writing to streams is not synchronized, so the output will
 * get garbled if multiple threads or processes write to the same
 * stream simultaneously.
 * 
 * A LogDestination has its own importance threshold associated with it that 
 * is in addition to a Log's threshold.  If the threshold of a destination
 * is higher than that associated with its Log, then the destination threshold
 * will override the Log's in preventing message from being recorded.  This 
 * allows some destinations to be more verbose than others.  
 */
class LogDestination {
public:

    /**
     * @brief create a destination with a threshold.  
     * @param strm       the output stream to send messages to.  If the pointer
     *                       id null, this LogDestination will act as a null-op
     *                       destination.
     * @param formatter  the LogFormatter to use to format the messages 
     * @param threshold  the minimum volume level required to pass a message
     *                       to the stream.  If not provided, it would be set
     *                       to 0.  
     * @bpdox{ignore}
     */
    LogDestination(std::ostream *strm, 
                   const boost::shared_ptr<LogFormatter>& formatter, 
                   int threshold=threshold::PASS_ALL);

    /**
     * create a copy
     * @bpdox{label:copy}
     */
    LogDestination(const LogDestination& that);

    /**
     * delete this destination
     */
    virtual ~LogDestination();

    /**
     * copy a destination into this one
     */
    LogDestination& operator=(const LogDestination& that);

    /**
     * return the importance threshold associated with this stream.  
     */
    int getThreshold() const { return _threshold; }

    /**
     * set the importance threshold associated with this stream.  If this 
     * threshold is higher than the threshold associated with the 
     * LogDestination's Log, it will override it.
     */
    void setThreshold(int threshold) { _threshold = threshold; }

    /**
     * record a given log record to this destinations output stream. The 
     * record will be sent to the stream attached to this class if (a)
     * there is actually an attached stream, (b) there is an attached
     * formatter, and (c) the importance level associated with the
     * record is equal to or greater than the threshold associated
     * with this destination. 
     * @return  true if the record was actually passed to the
     *          associated stream. 
     */
    bool write(const LogRecord& rec);

protected:
    int _threshold;   // the stream's threshold
    std::ostream *_strm;   // the output stream
    boost::shared_ptr<LogFormatter> _frmtr;    // the formatter to use
};

}}}     // end lsst::pex::logging

#endif  // LSST_PEX_LOGDESTINATION_H
