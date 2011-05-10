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
 
#if !defined(LSST_RUNTIME_EXCEPTION)      //! multiple inclusion guard macro
#define LSST_RUNTIME_EXCEPTION 1

#include "lsst/pex/exceptions/Exception.h"

namespace pexExcept = lsst::pex::exceptions;

namespace lsst {
namespace pex {
namespace exceptions {

LSST_EXCEPTION_TYPE(LogicErrorException, Exception, lsst::pex::exceptions::LogicErrorException)
LSST_EXCEPTION_TYPE(DomainErrorException, LogicErrorException, lsst::pex::exceptions::DomainErrorException)
LSST_EXCEPTION_TYPE(InvalidParameterException, LogicErrorException, lsst::pex::exceptions::InvalidParameterException)
LSST_EXCEPTION_TYPE(LengthErrorException, LogicErrorException, lsst::pex::exceptions::LengthErrorException)
LSST_EXCEPTION_TYPE(OutOfRangeException, LogicErrorException, lsst::pex::exceptions::OutOfRangeException)
LSST_EXCEPTION_TYPE(RuntimeErrorException, Exception, lsst::pex::exceptions::RuntimeErrorException)
LSST_EXCEPTION_TYPE(RangeErrorException, RuntimeErrorException, lsst::pex::exceptions::RangeErrorException)
LSST_EXCEPTION_TYPE(OverflowErrorException, RuntimeErrorException, lsst::pex::exceptions::OverflowErrorException)
LSST_EXCEPTION_TYPE(UnderflowErrorException, RuntimeErrorException, lsst::pex::exceptions::UnderflowErrorException)
LSST_EXCEPTION_TYPE(NotFoundException, Exception, lsst::pex::exceptions::NotFoundException)
LSST_EXCEPTION_TYPE(MemoryException, RuntimeErrorException, lsst::pex::exceptions::MemoryException)
LSST_EXCEPTION_TYPE(IoErrorException, RuntimeErrorException, lsst::pex::exceptions::IoErrorException)
LSST_EXCEPTION_TYPE(TimeoutException, RuntimeErrorException, lsst::pex::exceptions::TimeoutException)

}
}
}

#endif
