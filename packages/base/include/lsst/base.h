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
 
#if !defined(LSST_BASE_BASE_H)
#define LSST_BASE_BASE_H 1
/**
 * @file
 *
 * Basic LSST definitions
 */
#include "boost/shared_ptr.hpp"

/**
 * A shared pointer to an object
 *
 * \note Using this macro is preferable to the Ptr typedef in type T as no definition of T need be provided,
 * a forward definition (<tt>class T;</tt>) is sufficient
 *
 * \sa CONST_PTR
 */
#define LSST_WHITESPACE /* White space to avoid swig converting vector<PTR(XX)> into vector<shared_ptr<XX>> */
#define PTR(...) boost::shared_ptr<__VA_ARGS__ LSST_WHITESPACE > LSST_WHITESPACE
/**
 * A shared pointer to a const object
 *
 * \sa PTR
 */
#define CONST_PTR(...) boost::shared_ptr<const __VA_ARGS__ LSST_WHITESPACE > LSST_WHITESPACE

#endif
