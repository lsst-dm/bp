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
 

#ifndef RA_DEC_STR_H
#define RA_DEC_STR_H

#include <string>
#include <cmath>

#include "boost/format.hpp"
#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"

#include "lsst/pex/exceptions/Runtime.h"

namespace lsst { namespace utils {


std::string raRadToStr(double raRad);
std::string decRadToStr(double decRad);

std::string raDegToStr(double raDeg);
std::string decDegToStr(double decDeg);

std::string raDecRadToStr(double raRad, double decRad);
std::string raDecDegToStr(double raDeg, double decDeg);


double raStrToRad(std::string raStr, std::string delimiter=":");
double raStrToDeg(std::string raStr, std::string delimiter=":"); 
    
double decStrToRad(std::string decStr, std::string delimiter=":");
double decStrToDeg(std::string decStr, std::string delimiter=":"); 

}}                 

#endif
