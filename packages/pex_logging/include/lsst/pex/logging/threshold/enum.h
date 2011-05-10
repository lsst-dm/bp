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
 * @file enum.h
 * @brief definitions of reusable enumerations
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOGGING_THRESHOLD_ENUM_H
#define LSST_PEX_LOGGING_THRESHOLD_ENUM_H

namespace lsst {
namespace pex {
namespace logging {
namespace threshold {

/**
 * A threshold value that means use the value associated with a parent 
 * Log. 
 */
enum { INHERIT = -9999 };

/**
 * A lowest, most permissive possible threshold value, allowing all 
 * messages to pass through.  This is equivalent to the INHERIT value.
 */
enum { PASS_ALL = INHERIT };

}}}} // end lsst::pex::logging::threshold

#endif // end LSST_PEX_LOG_THRESHOLD_ENUM_H
