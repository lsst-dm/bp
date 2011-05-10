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
 
/** \file    Trace.h
  *
  * \ingroup pex
  *
  * \brief  definition of the Trace messaging facilities
  *
  */

#if !defined(LSST_PEX_LOGGING_TRACE_H)     //!< multiple inclusion guard macro
#define LSST_PEX_LOGGING_TRACE_H 1

#include <iostream>
#include <string>
#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <boost/format.hpp>

#include "lsst/pex/logging/Debug.h"

namespace lsst {
namespace pex {
namespace logging {

#ifndef LSST_DEBUGGING_ON
#define LSST_NO_TRACE 1
#endif

#if !defined(LSST_NO_TRACE)
#  define LSST_NO_TRACE 0    //!< True => turn off all tracing
#endif

#if !defined(LSST_MAX_TRACE)
#  define LSST_MAX_TRACE -1  //!< Maximum level to trace (only works for TTrace)
#endif

/**
 * \brief  limited backward compatibility to the 
 * DC2 run-time trace facilities 
 *
 * This class replaces the original Trace facility and is provided for 
 * backward-compatibility.  This fully in-lined implementation sends Trace
 * calls to the logging framework.
 *
 * Developers should prefer the use of DEBUG macros in available via 
 * Debug.h in the future which sends debugging messages through the 
 * logging framework.
 *
 * From the original Trace.h (by Robert Lupton):
 * 
 * \verbatim
 *      Tracing is controlled on a per "component" basis, where a "component" 
 *      is a name of the form aaa.bbb.ccc where aaa is the Most significant 
 *      part; for example, the utilities library might be called "utils", 
 *      the doubly-linked list "utils.dlist", and the code to destroy a 
 *      list "utils.dlist.del".
 *
 *      All tracing may be disabled by recompiling with LSST_NO_TRACE defined
 *      to be non-zero
 * \endverbatim
 *
 * Old Trace capabilities not supported were chose based on current use
 * at the time this class was developed.  These include
 * \verbatim
 *   o  stream printing to Trace objects via << operator
 *   o  TTrace templated functions
 * \endverbatim
 */
class Trace {
public:
#if !LSST_NO_TRACE

    /**
     * Print fmt if verbosity is high enough for name
     *
     * fmt is actually a printf format, so you can specify %d (etc) and provide
     * the required arguments.  This format will not actually be evaluated if
     * the trace is not active, so it's more efficient than the boost::format variant
     */
    Trace(const std::string& name,      //!< Name of component
          const int verbosity,          //!< Desired verbosity
          const std::string& fmt,       //!< Message to write as a printf format
          ...
          ) 
    {
        if (-1*verbosity >= Log::getDefaultLog().getThresholdFor(name)) {
            va_list ap;
            va_start(ap, fmt);
            const int len = vsnprintf(NULL, 0, fmt.c_str(), ap) + 1; // "+ 1" for the '\0'
            va_end(ap);

            char msg[len];
            va_start(ap, fmt);
            (void)vsnprintf(msg, len, fmt.c_str(), ap);
            va_end(ap);
            
            Debug out(name);
            out.debug(verbosity, msg);
        }
    }

    Trace(const std::string& name,      //!< Name of component
          const int verbosity,          //!< Desired verbosity
          const char *msg               //!< Message to write 
          ) 
    {
        if (-1*verbosity >= Log::getDefaultLog().getThresholdFor(name)) {
            Debug out(name);
            out.debug(verbosity, msg);
        }
    }

    /**
     * Print msg if verbosity is high enough for name
     */
    Trace(const std::string& name,      //!< Name of component
          const int verbosity,          //!< Desired verbosity
          const boost::format& msg      //!< Message to write
          )
    {
        if (-1*verbosity >= Log::getDefaultLog().getThresholdFor(name)) {
            Debug out(name);
            out.debug(verbosity, msg.str());
        }
    }

#else
/*
    Trace(const std::string& name, const int verbosity) {}
*/
    Trace(const std::string& name, const int verbosity,
          const std::string& msg, ...) {}
    Trace(const std::string& name, const int verbosity,
          const boost::format& msg) {}

#endif

    /// set where logging is sent; ignored in new implementation
    static void setDestination(std::ostream &) {
    }

    static void setVerbosity(const std::string &name) {
        if (name.length() == 0 || name == ".") 
            Log::getDefaultLog().setThreshold(Log::INHERIT_THRESHOLD);
        else
            Log::getDefaultLog().setThresholdFor(name, Log::INHERIT_THRESHOLD);
    }
    static void setVerbosity(const std::string &name, const int verbosity) {
        if (name.length() == 0 || name == ".") 
            Log::getDefaultLog().setThreshold(-1*verbosity);
        else
            Log::getDefaultLog().setThresholdFor(name, -1*verbosity);
    }
    static int getVerbosity(const std::string &name) {
        if (name.length() == 0 || name == ".") 
            return -1*Log::getDefaultLog().getThreshold();
        else 
            return -1*Log::getDefaultLog().getThresholdFor(name);
    }
    static void printVerbosity(std::ostream& out) {
        Log::getDefaultLog().printThresholds(out);
    }
    static void reset() {
        Log::getDefaultLog().reset();
    }
};

template<int VERBOSITY>
void TTrace(const char *name,           //!< Name of component
            const char *fmt,            //!< Message to write as a printf format
            ...
           ) {
    if (LSST_MAX_TRACE < 0 || VERBOSITY <= LSST_MAX_TRACE) {
        va_list ap;

        // first determine the length of the message
        va_start(ap, fmt);
        const int len = vsnprintf(NULL, 0, fmt, ap) + 1; // "+ 1" for the '\0'
        va_end(ap);

        char msg[len];
        va_start(ap, fmt);
        (void)vsnprintf(msg, len, fmt, ap);
        va_end(ap);

        Trace(name, VERBOSITY, msg);
    }
}

template<int VERBOSITY>
void TTrace(const std::string& name,      //!< Name of component
            const std::string& fmt,       //!< Message to write as a printf form at
            ...
           ) {
    if (LSST_MAX_TRACE < 0 || VERBOSITY <= LSST_MAX_TRACE) {
        va_list ap;
        va_start(ap, fmt);
        Trace(name, VERBOSITY, fmt, ap);
        va_end(ap);
    }
}


} // namespace logging
} // namespace pex
} // namespace lsst
#endif    // end LSST_PEX_LOGGING_TRACE_H

