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
 * @file FileDestination.cc
 * @author Ray Plante
 */
#include "lsst/pex/logging/FileDestination.h"

namespace lsst {
namespace pex {
namespace logging {

/**
 * create a file destination.  If the file does not exist, it will 
 * be created; otherwise, messages will be appended.  The IndentedFormatter
 * will be used to format the messages
 * @param filepath    the path to the log file to write messages to.
 * @param threshold   the minimum volume level required to pass a message
 *                       to the stream.  If not provided, it would be set
 *                       to 0.  
 * @param truncate    if True, overwrite the previous contents; otherwise,
 *                       new messages will be appended to the file.
 */
FileDestination::FileDestination(const fs::path& filepath, bool verbose, 
                                 int threshold, bool truncate)
    : LogDestination(new std::ofstream(filepath.string().c_str(), 
                                       truncate ? std::ios::out : std::ios::app), 
                     boost::shared_ptr<LogFormatter>(new IndentedFormatter(verbose)), 
                     threshold),
      _path(filepath) 
{ }
FileDestination::FileDestination(const std::string& filepath, bool verbose, 
                                 int threshold, bool truncate)
    : LogDestination(new std::ofstream(filepath.c_str(), 
                                       truncate ? std::ios::out : std::ios::app), 
                     boost::shared_ptr<LogFormatter>(new IndentedFormatter(verbose)), 
                     threshold),
      _path(filepath) 
{ }
FileDestination::FileDestination(const char *filepath, bool verbose, 
                                 int threshold, bool truncate)
    : LogDestination(new std::ofstream(filepath, 
                                       truncate ? std::ios::out : std::ios::app), 
                     boost::shared_ptr<LogFormatter>(new IndentedFormatter(verbose)), 
                     threshold),
      _path(filepath) 
{ }

/*
 * delete this destination
 */
FileDestination::~FileDestination() { 
    std::ofstream *ofstrm = dynamic_cast<std::ofstream*>(_strm);
    if (ofstrm != NULL && ofstrm->is_open()) {
        try {
            ofstrm->flush();
            ofstrm->close();
        }
        catch (...) { }
    }
    delete _strm;
}




}}}
