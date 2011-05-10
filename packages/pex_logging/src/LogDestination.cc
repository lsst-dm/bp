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
 * @file LogDestination.cc
 * @author Ray Plante
 */
#include "lsst/pex/logging/LogDestination.h"
#include "lsst/pex/logging/LogRecord.h"

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

using namespace std;

namespace lsst {
namespace pex {
namespace logging {

//@cond
using boost::shared_ptr;

/*
 * @brief create a destination with a threshold.  
 * @param strm       the output stream to send messages to.  If the pointer
 *                       id null, this LogDestination will act as a null-op
 *                       destination.
 * @param formatter  the LogFormatter to use to format the messages 
 * @param threshold  the minimum volume level required to pass a message
 *                       to the stream.  If not provided, it would be set
 *                       to 0.  
 */
LogDestination::LogDestination(ostream *strm, 
                               const shared_ptr<LogFormatter>& formatter,
                               int threshold) 
    : _threshold(threshold), _strm(strm), _frmtr(formatter)
{ }

/*
 * create a copy
 */
LogDestination::LogDestination(const LogDestination& that)
    : _threshold(that._threshold), _strm(that._strm), _frmtr(that._frmtr)
{ }

/*
 * delete this destination
 */
LogDestination::~LogDestination() { }

/*
 * copy a destination into this one
 */
LogDestination& LogDestination::operator=(const LogDestination& that) {
    _threshold = that._threshold;
    _strm = that._strm; 
    _frmtr = that._frmtr;
    return *this;
}

/*
 * record a given log record to this destinations output stream. The 
 * record will be sent to the stream attached to this class if (a)
 * there is actually an attached stream, (b) there is an attached
 * formatter, and (c) the importance level associated with the
 * record is equal to or greater than the threshold associated
 * with this destination. 
 * @return  true if the record was actually passed to the
 *          associated stream. 
 */
bool LogDestination::write(const LogRecord& rec) {
    if (_strm != 0 && _frmtr.get() != 0 && 
        rec.getImportance() >= _threshold)
    {
        _frmtr->write(_strm, rec);
        return true;
    }
    return false;
}

//@endcond
}}} // end lsst::pex::logging

