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
  * \class   LsstDataConfigurator
  *
  * \ingroup daf
  *
  * \brief   Configure the content of LsstData objects
  *
  * Implements a factory object to configure instances of LsstData realizations. 
  *  
  * Since this is a singleton class, it provides no
  * callable constructors nor can it be used in an assignment statement. It 
  * cannot be used to derive another class. (It is 'final' or 'sealed').
  *
  * Usage assuming using namespace lsst::daf::data:
  *       LsstDataConfigurator::method(...);
  * 
  * 
  * \note
  * 
  * \author  $Author::                                                        $
  * \version $Rev::                                                           $
  * \date    $Date::                                                          $
  * 
  * $Id::                                                                     $
  * 
  * Contact: Jeff Bartels (jeffbartels@usa.net)
  * 
  * Created: 
  * 
  */

#ifndef LSST_MWI_DATA_LSSTDATACONFIGURATOR_H
#define LSST_MWI_DATA_LSSTDATACONFIGURATOR_H

#include "lsst/pex/policy/Policy.h"
#include "lsst/daf/data/LsstData.h"

namespace lsst {
namespace daf {
namespace data {

class LsstDataConfigurator {
public:

    /**
      * \brief   Initialize the given LsstData object according to the
      *          content of the given Policy object
      * \param   data The LsstData object to initialize 
      * \param   policy The controlling policy object
      */
    static void configureSupport(
        LsstData::Ptr data,
        lsst::pex::policy::Policy::Ptr policy
    );
private:
    // All constructors/destructor, copy constructors, assignment operators
    // are private to preclude specialization and explicit creation
    // of the class
    LsstDataConfigurator();
    LsstDataConfigurator(const LsstDataConfigurator&);
    LsstDataConfigurator& operator= (const LsstDataConfigurator&);
    ~LsstDataConfigurator();
    // The singleton instance (initialized during 1st call to the()
    static LsstDataConfigurator* _singleton;
    // Returns reference to the singleton, creates on first call
    static LsstDataConfigurator& the();
};


} // namespace data
} // namespace daf
} // namespace lsst

#endif // LSST_MWI_DATA_LSSTDATACONFIGURATOR_H

