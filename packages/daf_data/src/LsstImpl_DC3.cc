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
 
//////////////////////////////////////////////////////////////////////////////
// LsstImpl_DC3.cc
// Implementation of class LsstImpl_DC3 methods
//
// Contact: Jeff Bartels (jeffbartels@usa.net)
// 
// Created: 03-Apr-2007 5:30:00 PM
//////////////////////////////////////////////////////////////////////////////

#include <string>

#include "lsst/daf/base/Citizen.h"
#include "lsst/daf/base/PropertySet.h"
#include "lsst/daf/data/LsstImpl_DC3.h"
#include "lsst/pex/policy/Policy.h"

namespace dafBase = lsst::daf::base;
namespace dafData = lsst::daf::data;

dafData::LsstImpl_DC3::LsstImpl_DC3(const std::type_info & type) :
    dafBase::Citizen(type)
{
    _metadata = dafBase::PropertySet::Ptr();
    _persistence = lsst::daf::persistence::Persistence::Ptr();
    _policy = lsst::pex::policy::Policy::Ptr();
    _provenance = dafData::Provenance::Ptr();
    _releaseProcess = dafData::ReleaseProcess::Ptr();
    _security = lsst::security::Security::PtrType();
}


dafData::LsstImpl_DC3::~LsstImpl_DC3() {
}


dafData::LsstData::IteratorRange
dafData::LsstImpl_DC3::getChildren(unsigned depth)
{
	return dafData::LsstData::IteratorRange(
        _children.begin(), _children.end());
}


dafBase::PropertySet::Ptr dafData::LsstImpl_DC3::getMetadata() const {
	return _metadata;
}


lsst::daf::persistence::Persistence::Ptr
dafData::LsstImpl_DC3::getPersistence() const {
	return _persistence;
}


lsst::pex::policy::Policy::Ptr dafData::LsstImpl_DC3::getPolicy() const {
	return _policy;
}


dafData::Provenance::Ptr dafData::LsstImpl_DC3::getProvenance() const {
	return _provenance;
}


dafData::ReleaseProcess::Ptr dafData::LsstImpl_DC3::getReleaseProcess() const {
	return _releaseProcess;
}


lsst::security::Security::PtrType dafData::LsstImpl_DC3::getSecurity() const {
	return _security;
}


void dafData::LsstImpl_DC3::setMetadata(dafBase::PropertySet::Ptr metadata) {
	_metadata = metadata;
}


void dafData::LsstImpl_DC3::setPersistence(
    lsst::daf::persistence::Persistence::Ptr persistence) {
	_persistence = persistence;
}


void dafData::LsstImpl_DC3::setPolicy(lsst::pex::policy::Policy::Ptr policy) {
	_policy = policy;
}


void dafData::LsstImpl_DC3::setProvenance(dafData::Provenance::Ptr provenance) {
	_provenance = provenance;
}


void dafData::LsstImpl_DC3::setReleaseProcess(
    dafData::ReleaseProcess::Ptr release) {
	_releaseProcess = release;
}


void dafData::LsstImpl_DC3::setSecurity(
    lsst::security::Security::PtrType security) {
	_security = security;
}


std::string dafData::LsstImpl_DC3::toString(){
    return repr();  // In Citizen
}
