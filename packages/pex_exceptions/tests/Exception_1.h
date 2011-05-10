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
 
#ifndef Exception_1_h
#define Exception_1_h

#include "lsst/pex/exceptions/Exception.h"

namespace pexExcept = lsst::pex::exceptions;

LSST_EXCEPTION_TYPE(ChildException, pexExcept::Exception, ChildException)

class DetailedException : public pexExcept::Exception {
public:
    DetailedException(LSST_EARGS_TYPED, int count) :
        Exception(LSST_EARGS_UNTYPED), _count(count) { };
    virtual std::ostream& addToStream(std::ostream& stream) const {
        // Booby-trapped to demonstrate what() exception handling.
        if (_count < 0) throw std::exception();
        // Demonstrates how to add information before and after base class.
        return pexExcept::Exception::addToStream(
            stream << "Begin DetailedException(" << _count <<
            ")" << std::endl) <<
            "End DetailedException" << std::endl;
    };
    virtual char const* getType(void) const throw() {
        return "DetailedException *";
    };
    int getCount(void) const { return _count; };
private:
    int _count;
};

#endif
