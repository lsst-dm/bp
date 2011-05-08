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
  * \class LsstImpl_DC3
  *
  * \ingroup daf
  *
  * \brief The implementation of LsstImpl for DC3.
  *        
  *        While publicly available, it is intended that LsstData realizations
  *        will derive from LsstBase, and not LsstImpl_DC3. This indirection
  *        will isolate LsstData realizations from the exact base 
  *        implementation chosen for a given release of the framework.
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

#ifndef LSST_MWI_DATA_LSSTIMPL_DC3_H
#define LSST_MWI_DATA_LSSTIMPL_DC3_H

#include <typeinfo>

#include "lsst/daf/base/Citizen.h"
#include "lsst/daf/data/LsstData.h"
#include "lsst/pex/policy/Policy.h"

namespace lsst {
namespace daf {
namespace data {

class LsstImpl_DC3 : public LsstData, public lsst::daf::base::Citizen {
public:
    LsstImpl_DC3(const std::type_info & type);
    /// Virtual destructor, class may be specialized (see Stroustrup 12.4.2)
    virtual ~LsstImpl_DC3();

    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getChildren().
      *          May be overridden.
      *          Classes deriving from LsstImpl need implement a getChildren
      *          method only if they collect children.
      * \param   depth Specifies how deep to recurse the collection of
      *                children objects when creating the returned collection.
      *                see lsst::daf::data::LsstData.getChildren
      * \return  Container Will always return a pair of iterators giving a NULL range
      *          (i.e. std::distance( first, last) = 0)  
      */
    virtual LsstData::IteratorRange getChildren( unsigned depth = 1 );


    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getMetadata().
      *          May be overridden. Base implementation returns a 
      *          reference-counted pointer to the base object's cached 
      *          instance of Metadata.
      * \return  see lsst::daf::data::PropertySet
      */
    virtual lsst::daf::base::PropertySet::Ptr getMetadata() const;

    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getPersistence().
      *          May be overridden. Base implementation returns a 
      *          reference-counted pointer to the base object's cached 
      *          instance of Persistence.
      * \return  see lsst::daf::persistence::Persistence
      */
    virtual lsst::daf::persistence::Persistence::Ptr getPersistence() const;

    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getPolicy().
      *          May be overridden. Base implementation returns a 
      *          reference-counted pointer to the base object's cached 
      *          instance of Policy.
      * \return  see lsst::pex::policy::Policy
      */
    virtual lsst::pex::policy::Policy::Ptr getPolicy() const;

    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getProvenance().
      *          May be overridden. Base implementation returns a 
      *          reference-counted pointer to the base object's cached 
      *          instance of Provenance.
      * \return  see lsst::daf::data::Provenance
      */
    virtual Provenance::Ptr getProvenance() const;

    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getReleaseProcess().
      *          May be overridden. Base implementation returns a 
      *          reference-counted pointer to the base object's cached 
      *          instance of ReleaseProcess.
      * \return  see lsst::daf::data::ReleaseProcess
      */
    virtual ReleaseProcess::Ptr getReleaseProcess() const;

    /**
      * \brief   Base implementation of lsst::daf::data::LsstData.getSecurity().
      *          May be overridden. Base implementation returns a 
      *          reference-counted pointer to the base object's cached 
      *          instance of Security.
      * \return  see lsst::security::Security
      */
    virtual lsst::security::Security::PtrType getSecurity() const;

    /**
      * \brief   Base implementation of lsst::daf::data::setMetadata(). 
      *          May be overridden. Assigns the given metadata
      *          object to the base object's private data member. May result in
      *          the destruction of the currently-cached member object since
      *          the data member is a reference-counted pointer.
      */
    virtual void setMetadata(lsst::daf::base::PropertySet::Ptr metadata);

    /**
      * \brief   Base implementation of lsst::daf::data::setPersistence(). 
      *          May be overridden. Assigns the given Persistence
      *          object to the base object's private data member. May result in
      *          the destruction of the currently-cached member object since
      *          the data member is a reference-counted pointer.
      */
    virtual void setPersistence(
                lsst::daf::persistence::Persistence::Ptr persistence);

    /**
      * \brief   Base implementation of lsst::daf::data::setPolicy(). 
      *          May be overridden. Assigns the given Policy
      *          object to the base object's private data member. May result in
      *          the destruction of the currently-cached member object since
      *          the data member is a reference-counted pointer.
      */
    virtual void setPolicy(lsst::pex::policy::Policy::Ptr policy);

    /**
      * \brief   Base implementation of lsst::daf::data::setProvenance(). 
      *          May be overridden. Assigns the given Provenance
      *          object to the base object's private data member. May result in
      *          the destruction of the currently-cached member object since
      *          the data member is a reference-counted pointer.
      */
    virtual void setProvenance(Provenance::Ptr provenance);

    /**
      * \brief   Base implementation of lsst::daf::data::setReleaseProcess(). 
      *          May be overridden. Assigns the given ReleaseProcess
      *          object to the base object's private data member. May result in
      *          the destruction of the currently-cached member object since
      *          the data member is a reference-counted pointer.
      */
    virtual void setReleaseProcess(ReleaseProcess::Ptr release);

    /**
      * \brief   Base implementation of lsst::daf::data::setSecurity(). 
      *          May be overridden. Assigns the given Security
      *          object to the base object's private data member. May result in
      *          the destruction of the currently-cached member object since
      *          the data member is a reference-counted pointer.
      */
    virtual void setSecurity(lsst::security::Security::PtrType security);
 
    /**
      * \brief   Base implementation of lsst::daf::data::toString(). 
      *          May be overridden. Returns a short string representation
      *          of the object as implemented by Citizen::repr().
      */
    virtual std::string toString();

private:
    lsst::daf::base::PropertySet::Ptr _metadata;
    lsst::daf::persistence::Persistence::Ptr _persistence;
    lsst::pex::policy::Policy::Ptr _policy;
    Provenance::Ptr _provenance;
    ReleaseProcess::Ptr _releaseProcess;
    lsst::security::Security::PtrType _security;
    LsstData::Container _children;

};

} // namespace data
} // namespace daf
} // namespace lsst

#endif // LSST_MWI_DATA_LSSTIMPL_DC3_H

