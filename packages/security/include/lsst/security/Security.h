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
  * \class Security
  *
  * \ingroup security
  *
  * \brief Implements the logic that enforces the access and authorization 
  *  rules that apply to an LsstData Realization.
  * 
  * 
  * \note OUT OF SCOPE FOR DC3 - stub implementation
  * 
  * \author  $Author::                                                        $
  * \version $Rev::                                                           $
  * \date    $Date::                                                          $
  * 
  * $Id::                                                                     $
  * 
  * Contact: Jeff Bartels (jeffbartels@usa.net)
  * 
  * Created: 03-Apr-2007 5:30:00 PM
  * 
  */

#ifndef LSST_SECURITY_SECURITY_H
#define LSST_SECURITY_SECURITY_H

#include <boost/shared_ptr.hpp>

#include "lsst/daf/base/Citizen.h"

namespace lsst {
namespace security {

class Security : public lsst::daf::base::Citizen {
public:
    /// Default constructor
    Security();
    /// Copy initialization semantics (NIL in this revision)
    Security(const Security&);
    /// Copy assignment semantics (NIL in this revision)
    Security& operator= (const Security&);
    /// Virtual destructor, class may be specialized (see Stroustrup 12.4.2)
    virtual ~Security();

    /// Reference-counted pointer typedef forinstances of this class
    typedef boost::shared_ptr<Security> PtrType;
    
    /// A short string representation of an instance
    std::string toString();
};
    
}} // namespace lsst::security

#endif // LSST_SECURITY_SECURITY_H

