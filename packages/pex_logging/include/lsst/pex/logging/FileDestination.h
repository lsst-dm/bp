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
 * @file FileDestination.h
 * @brief definition of the FileDestination class
 * @author Ray Plante
 */
#ifndef LSST_PEX_FILEDESTINATION_H
#define LSST_PEX_FILEDESTINATION_H

#include "lsst/pex/logging/LogDestination.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>

namespace fs = boost::filesystem;

namespace lsst {
namespace pex {
namespace logging {

/**
 * @brief  a LogDestination represented by a file.  
 *
 * This class makes it easier to attach files to Logs via Python via 
 * Log.addDestination().
 */
class FileDestination : public LogDestination {
public:

    //@{
    /**
     * create a file destination.  If the file does not exist, it will 
     * be created; otherwise, messages will be appended.
     * @param filepath    the path to the log file to write messages to.
     * @param formatter   the LogFormatter to use to format the messages 
     * @param threshold   the minimum volume level required to pass a message
     *                       to the stream.  If not provided, it would be set
     *                       to 0.  
     * @param truncate    if True, overwrite the previous contents; otherwise,
     *                       new messages will be appended to the file.
     */
    /// @bpdox{label:formatter}
    FileDestination(const std::string& filepath,
                    const boost::shared_ptr<LogFormatter>& formatter, 
                    int threshold=threshold::PASS_ALL,
                    bool truncate=false) 
        : LogDestination(new std::ofstream(filepath.c_str(),
                                           truncate ? std::ios::out 
                                                    : std::ios::app), 
                         formatter, threshold),
          _path(filepath) 
    { }
    /// @bpdox{ignore}
    FileDestination(const char *filepath,
                    const boost::shared_ptr<LogFormatter>& formatter, 
                    int threshold=threshold::PASS_ALL,
                    bool truncate=false)
        : LogDestination(new std::ofstream(filepath,
                                           truncate ? std::ios::out 
                                                    : std::ios::app), 
                         formatter, threshold),
          _path(filepath) 
    { }
    /// @bpdox{ignore}
    FileDestination(const fs::path& filepath,
                    const boost::shared_ptr<LogFormatter>& formatter, 
                    int threshold=threshold::PASS_ALL,
                    bool truncate=false)
        : LogDestination(new std::ofstream(filepath.string().c_str(),
                                           truncate ? std::ios::out 
                                                    : std::ios::app), 
                         formatter, threshold),
          _path(filepath) 
    { }
    //@}

    //@{
    /**
     * create a file destination.  If the file does not exist, it will 
     * be created; otherwise, messages will be appended.  The IndentedFormatter
     * will be used to format the messages
     * @param filepath    the path to the log file to write messages to.
     * @param verbose     if True, make sure all properties are printed 
     *                       out to the destination
     * @param threshold   the minimum volume level required to pass a message
     *                       to the stream.  If not provided, it would be set
     *                       to 0.  
     * @param truncate    if True, overwrite the previous contents; otherwise,
     *                       new messages will be appended to the file.
     */
    /// @bpdox{label:verbose}
    FileDestination(const std::string& filepath, bool verbose=false, 
                    int threshold=threshold::PASS_ALL, bool truncate=false);
    /// @bpdox{ignore}
    FileDestination(const char *filepath, bool verbose=false, 
                    int threshold=threshold::PASS_ALL, bool truncate=false);
    /// @bpdox{ignore}
    FileDestination(const fs::path& filepath, bool verbose=false, 
                    int threshold=threshold::PASS_ALL, bool truncate=false);
    //@}

    virtual ~FileDestination();

    const fs::path& getPath() const { return _path; }

protected:
    fs::path _path;
};

}}}     // end lsst::pex::logging

#endif  // LSST_PEX_LOGDESTINATION_H
