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
 * @file BlockTimingLog.cc
 * @author Ray Plante
 */

#include "lsst/pex/logging/BlockTimingLog.h"
#include <fstream>

namespace lsst {
namespace pex {
namespace logging {

const int BlockTimingLog::INSTRUM = logging::Log::INFO - 3;

const std::string BlockTimingLog::STATUS("STATUS");
const std::string BlockTimingLog::START("start");
const std::string BlockTimingLog::END("end");

BlockTimingLog::BlockTimingLog(const Log& parent, const std::string& name, 
                               int tracelev, int usageFlags, 
                               const std::string& funcName) 
    : Log(parent, name), _tracelev(tracelev), _pusageFlags(0), 
      _usageFlags(usageFlags), _funcName(funcName), _usage()
      
{
    if (_funcName.length() == 0) _funcName = name;
    const BlockTimingLog *p = dynamic_cast<const BlockTimingLog*>(&parent);
    if (_tracelev == Log::INHERIT_THRESHOLD) {
        if (p) {
            _tracelev = p->getInstrumentationLevel();
        }
        else {
            _tracelev = INSTRUM;
        }
    }
    if (_usageFlags == PARENTUDATA) {
        if (p) addUsageFlags(p->getUsageFlags());
    }
}

BlockTimingLog::~BlockTimingLog() { }

void BlockTimingLog::addUsageProps(LogRecord& rec) {
    if (! _usage.get()) _usage.reset(new struct rusage());

    if (getrusage(RUSAGE_SELF, _usage.get()) == 0) {
        double d =  0;
        if (_usageFlags & UTIME)  {
            d = _usage->ru_utime.tv_sec + _usage->ru_utime.tv_usec/1.0e6;
            rec.addProperty("usertime", d);
        }
        if (_usageFlags & STIME)  {
            d = _usage->ru_utime.tv_sec + _usage->ru_utime.tv_usec/1.0e6;
            rec.addProperty("systemtime", d);
        }
        if (_usageFlags & MEMSZ)  rec.addProperty("maxrss", _usage->ru_maxrss);
        if (_usageFlags & MINFLT) rec.addProperty("minflt", _usage->ru_minflt);
        if (_usageFlags & MAJFLT) rec.addProperty("majflt", _usage->ru_majflt);
        if (_usageFlags & NSWAP)  rec.addProperty("nswap", _usage->ru_nswap);
        if (_usageFlags & BLKIN)  rec.addProperty("blocksin", _usage->ru_inblock);
        if (_usageFlags & BLKOUT) rec.addProperty("blocksout", _usage->ru_oublock);
    }
}

}}} // end lsst::pex::logging
