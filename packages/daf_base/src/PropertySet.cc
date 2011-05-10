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
 

/** @file
  * @ingroup daf_base
  *
  * @brief Implementation for PropertySet class
  *
  * @version $Revision$
  * @date $Date$
  *
  * Contact: Kian-Tat Lim (ktl@slac.stanford.edu)
  */

#include "lsst/daf/base/PropertySet.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "lsst/pex/exceptions/Runtime.h"
#include "lsst/daf/base/DateTime.h"

namespace dafBase = lsst::daf::base;
namespace pexExcept = lsst::pex::exceptions;

/** Constructor.
  */
dafBase::PropertySet::PropertySet(void) : Citizen(typeid(*this)) {
}

/** Destructor.
  */
dafBase::PropertySet::~PropertySet(void) {
}

///////////////////////////////////////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////////////////////////////////////

/** Copy the PropertySet and all of its contents.
  * @return PropertySet::Ptr pointing to the new copy.
  */
dafBase::PropertySet::Ptr dafBase::PropertySet::deepCopy(void) const {
    Ptr n(new PropertySet);
    for (AnyMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
        if (i->second->back().type() == typeid(Ptr)) {
            for (std::vector<boost::any>::const_iterator j =
                 i->second->begin(); j != i->second->end(); ++j) {
                Ptr p = boost::any_cast<Ptr>(*j);
                if (p.get() == 0) {
                    n->add(i->first, Ptr());
                } else {
                    n->add(i->first, p->deepCopy());
                }
            }
        } else {
            boost::shared_ptr< std::vector<boost::any> > vp(
                new std::vector<boost::any>(*(i->second)));
            n->_map[i->first] = vp;
        }
    }
    return n;
}

/** Get the number of names in the PropertySet, optionally including those in
  * subproperties.  The name of the subproperty is counted in that case.
  * @param[in] topLevelOnly true (default) = don't include subproperties.
  * @return Number of names.
  */
size_t dafBase::PropertySet::nameCount(bool topLevelOnly) const {
    int n = 0;
    for (AnyMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
        ++n;
        if (!topLevelOnly && i->second->back().type() == typeid(Ptr)) {
            Ptr p = boost::any_cast<Ptr>(i->second->back());
            if (p.get() != 0) {
                n += p->nameCount(false);
            }
        }
    }
    return n;
}

/** Get the names in the PropertySet, optionally including those in
  * subproperties.  The name of the subproperty is included in that case.
  * Hierarchical subproperty names are separated by a period.
  * @param[in] topLevelOnly true (default) = don't include subproperties.
  * @return Vector of names.
  */
std::vector<std::string> dafBase::PropertySet::names(bool topLevelOnly) const {
    std::vector<std::string> v;
    for (AnyMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
        v.push_back(i->first);
        if (!topLevelOnly && i->second->back().type() == typeid(Ptr)) {
            Ptr p = boost::any_cast<Ptr>(i->second->back());
            if (p.get() != 0) {
                std::vector<std::string> w = p->names(false);
                for (std::vector<std::string>::const_iterator k = w.begin();
                     k != w.end(); ++k) {
                    v.push_back(i->first + "." + *k);
                }
            }
        }
    }
    return v;
}

/** Get the names of parameters (non-subproperties) in the PropertySet,
  * optionally including those in subproperties.  Hierarchical subproperty
  * names are separated by a period.
  * @param[in] topLevelOnly true (default) = don't include subproperties.
  * @return Vector of parameter names.
  */
std::vector<std::string>
dafBase::PropertySet::paramNames(bool topLevelOnly) const {
    std::vector<std::string> v;
    for (AnyMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
        if (i->second->back().type() == typeid(Ptr)) {
            Ptr p = boost::any_cast<Ptr>(i->second->back());
            if (p.get() != 0 && !topLevelOnly) {
                std::vector<std::string> w = p->paramNames(false);
                for (std::vector<std::string>::const_iterator k = w.begin();
                     k != w.end(); ++k) {
                    v.push_back(i->first + "." + *k);
                }
            }
        } else {
            v.push_back(i->first);
        }
    }
    return v;
}

/** Get the names of subproperties in the PropertySet, optionally including
  * those in subproperties.  Hierarchical subproperty names are separated by a
  * period.
  * @param[in] topLevelOnly true (default) = don't include subproperties.
  * @return Vector of subproperty names.
  */
std::vector<std::string>
dafBase::PropertySet::propertySetNames(bool topLevelOnly) const {
    std::vector<std::string> v;
    for (AnyMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
        if (i->second->back().type() == typeid(Ptr)) {
            v.push_back(i->first);
            Ptr p = boost::any_cast<Ptr>(i->second->back());
            if (p.get() != 0 && !topLevelOnly) {
                std::vector<std::string> w = p->propertySetNames(false);
                for (std::vector<std::string>::const_iterator k = w.begin();
                     k != w.end(); ++k) {
                    v.push_back(i->first + "." + *k);
                }
            }
        }
    }
    return v;
}

/** Determine if a name (possibly hierarchical) exists.
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return true if property exists.
  */
bool dafBase::PropertySet::exists(std::string const& name) const {
    return _find(name) != _map.end();
}

/** Determine if a name (possibly hierarchical) has multiple values.
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return true if property exists and has more than one value.
  */
bool dafBase::PropertySet::isArray(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    return i != _map.end() && i->second->size() > 1U;
}

/** Determine if a name (possibly hierarchical) is a subproperty.
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return true if property exists and its values are PropertySet::Ptrs.
  */
bool dafBase::PropertySet::isPropertySetPtr(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    return i != _map.end() && i->second->back().type() == typeid(Ptr);
}

/** Get number of values for a property name (possibly hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Number of values for that property.  0 if it doesn't exist.
  */
size_t dafBase::PropertySet::valueCount(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) return 0;
    return i->second->size();
}

/** Get the type of values for a property name (possibly hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Type of values for that property.
  * @throws NotFoundException Property does not exist.
  */
std::type_info const& dafBase::PropertySet::typeOf(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        throw LSST_EXCEPT(pexExcept::NotFoundException, name + " not found");
    }
    return i->second->back().type();
}

// The following throw an exception if the type does not match exactly.

/** Get the last value for a property name (possibly hierarchical). @bpdox{label:nodefault}
  * Note that the type must be explicitly specified for this template:
  * @code int i = propertySet.get<int>("foo") @endcode
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Last value set or added.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value does not match desired type.
  */
template <typename T>
T dafBase::PropertySet::get(std::string const& name) const { /* parasoft-suppress LsstDm-3-4a LsstDm-4-6 "allow template over bool" */
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        throw LSST_EXCEPT(pexExcept::NotFoundException, name + " not found");
    }
    try {
        return boost::any_cast<T>(i->second->back());
    }
    catch (boost::bad_any_cast) {
        throw LSST_EXCEPT(TypeMismatchException, name);
    }
    // not reached
    return boost::any_cast<T>(i->second->back());
}

/** Get the last value for a property name (possibly hierarchical). @bpdox{label:withdefault}
  * Returns the provided @a defaultValue if the property does not exist.
  * Note that the type must be explicitly specified for this template:
  * @code int i = propertySet.get<int>("foo", 42) @endcode
  * @param[in] name Property name to examine, possibly hierarchical.
  * @param[in] defaultValue Default value to return if property does not exist.
  * @return Last value set or added.
  * @throws TypeMismatchException Value does not match desired type.
  */
template <typename T>
T dafBase::PropertySet::get(std::string const& name, T const& defaultValue) const { /* parasoft-suppress LsstDm-3-4a LsstDm-4-6 "allow template over bool" */
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        return defaultValue;
    }
    try {
        return boost::any_cast<T>(i->second->back());
    }
    catch (boost::bad_any_cast) {
        throw LSST_EXCEPT(TypeMismatchException, name);
    }
    // not reached
    return boost::any_cast<T>(i->second->back());
}

/** Get the vector of values for a property name (possibly hierarchical).
  * Note that the type must be explicitly specified for this template:
  * @code vector<int> v = propertySet.getArray<int>("foo") @endcode
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Vector of values.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value does not match desired type.
  */
template <typename T>
std::vector<T> dafBase::PropertySet::getArray(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        throw LSST_EXCEPT(pexExcept::NotFoundException, name + " not found");
    }
    std::vector<T> v;
    for (std::vector<boost::any>::const_iterator j = i->second->begin();
         j != i->second->end(); ++j) {
        try {
            v.push_back(boost::any_cast<T>(*j));
        }
        catch (boost::bad_any_cast) {
            throw LSST_EXCEPT(TypeMismatchException, name);
        }
    }
    return v;
}

// The following throw an exception if the conversion is inappropriate.

/** Get the last value for a bool property name (possibly hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Value as a bool.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value is not a bool.
  */
bool dafBase::PropertySet::getAsBool(std::string const& name) const { /* parasoft-suppress LsstDm-3-4a LsstDm-4-6 "for symmetry with other types" */
    return get<bool>(name);
}

/** Get the last value for a bool/char/short/int property name (possibly
  * hierarchical).  Unsigned int properties are not acceptable, but unsigned
  * versions of smaller types are.
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Value as an int.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value cannot be converted to int.
  */
int dafBase::PropertySet::getAsInt(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        throw LSST_EXCEPT(pexExcept::NotFoundException, name + " not found");
    }
    boost::any v = i->second->back();
    std::type_info const& t = v.type();
    if (t == typeid(bool)) {
        return boost::any_cast<bool>(v);
    } else if (t == typeid(char)) {
        return boost::any_cast<char>(v);
    } else if (t == typeid(signed char)) {
        return boost::any_cast<signed char>(v);
    } else if (t == typeid(unsigned char)) {
        return boost::any_cast<unsigned char>(v);
    } else if (t == typeid(short)) {
        return boost::any_cast<short>(v);
    } else if (t == typeid(unsigned short)) {
        return boost::any_cast<unsigned short>(v);
    }
    try {
        return boost::any_cast<int>(v);
    }
    catch (boost::bad_any_cast) {
        throw LSST_EXCEPT(TypeMismatchException, name);
    }
    // not reached
    return boost::any_cast<int>(v);
}

/** Get the last value for a bool/char/short/int/int64_t property name
  * (possibly hierarchical).  Unsigned int64_t properties are not acceptable,
  * but unsigned versions of smaller types are, except possibly unsigned long,
  * depending on compiler.
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Value as an int64_t.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value cannot be converted to int64_t.
  */
int64_t dafBase::PropertySet::getAsInt64(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        throw LSST_EXCEPT(pexExcept::NotFoundException, name + " not found");
    }
    boost::any v = i->second->back();
    std::type_info const& t = v.type();
    if (t == typeid(bool)) return boost::any_cast<bool>(v);
    if (t == typeid(char)) return boost::any_cast<char>(v);
    if (t == typeid(signed char)) return boost::any_cast<signed char>(v);
    if (t == typeid(unsigned char)) return boost::any_cast<unsigned char>(v);
    if (t == typeid(short)) return boost::any_cast<short>(v);
    if (t == typeid(unsigned short)) return boost::any_cast<unsigned short>(v);
    if (t == typeid(int)) return boost::any_cast<int>(v);
    if (t == typeid(unsigned int)) return boost::any_cast<unsigned int>(v);
    if (t == typeid(long)) return boost::any_cast<long>(v);
    if (t == typeid(long long)) return boost::any_cast<long long>(v);
    try {
        return boost::any_cast<int64_t>(v);
    }
    catch (boost::bad_any_cast) {
        throw LSST_EXCEPT(TypeMismatchException, name);
    }
    // not reached
    return boost::any_cast<int64_t>(v);
}

/** Get the last value for any arithmetic property name (possibly
  * hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Value as a double.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value cannot be converted to double.
  */
double dafBase::PropertySet::getAsDouble(std::string const& name) const {
    AnyMap::const_iterator i = _find(name);
    if (i == _map.end()) {
        throw LSST_EXCEPT(pexExcept::NotFoundException, name + " not found");
    }
    boost::any v = i->second->back();
    std::type_info const& t = v.type();
    if (t == typeid(bool)) return boost::any_cast<bool>(v);
    if (t == typeid(char)) return boost::any_cast<char>(v);
    if (t == typeid(signed char)) return boost::any_cast<signed char>(v);
    if (t == typeid(unsigned char)) return boost::any_cast<unsigned char>(v);
    if (t == typeid(short)) return boost::any_cast<short>(v);
    if (t == typeid(unsigned short)) return boost::any_cast<unsigned short>(v);
    if (t == typeid(int)) return boost::any_cast<int>(v);
    if (t == typeid(unsigned int)) return boost::any_cast<unsigned int>(v);
    if (t == typeid(long)) return boost::any_cast<long>(v);
    if (t == typeid(unsigned long)) return boost::any_cast<unsigned long>(v);
    if (t == typeid(long long)) return boost::any_cast<long long>(v);
    if (t == typeid(unsigned long long)) return boost::any_cast<unsigned long long>(v);
    if (t == typeid(float)) return boost::any_cast<float>(v);
    try {
        return boost::any_cast<double>(v);
    }
    catch (boost::bad_any_cast) {
        throw LSST_EXCEPT(TypeMismatchException, name);
    }
    // not reached
    return boost::any_cast<double>(v);
}

/** Get the last value for a string property name (possibly hierarchical).
  * Note that properties set with <tt>char const*</tt> can be retrieved as
  * strings using this method.
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return String value.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value is not a string.
  */
std::string dafBase::PropertySet::getAsString(std::string const& name) const {
    return get<std::string>(name);
}

/** Get the last value for a subproperty name (possibly hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return PropertySet::Ptr value.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value is not a PropertySet::Ptr.
  */
dafBase::PropertySet::Ptr
dafBase::PropertySet::getAsPropertySetPtr(std::string const& name) const {
    return get<Ptr>(name);
}

/** Get the last value for a Persistable name (possibly hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Persistable::Ptr value.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value is not a Persistable::Ptr.
  */
dafBase::Persistable::Ptr
dafBase::PropertySet::getAsPersistablePtr(std::string const& name) const {
    return get<Persistable::Ptr>(name);
}

/** Generate a string representation of the PropertySet.
  * Use this for debugging, not for serialization/persistence.
  * @param[in] topLevelOnly false (default) = do include subproperties.
  * @param[in] indent String to indent lines by (default none).
  * @return String representation of the PropertySet.
  */
std::string dafBase::PropertySet::toString(bool topLevelOnly,
                                           std::string const& indent) const {
    std::ostringstream s;
    std::vector<std::string> nv = names();
    sort(nv.begin(), nv.end());
    for (std::vector<std::string>::const_iterator i = nv.begin(); i != nv.end(); ++i) {
        boost::shared_ptr< std::vector<boost::any> > vp = _map.find(*i)->second;
        std::type_info const& t = vp->back().type();
        if (t == typeid(Ptr)) {
            s << indent << *i << " = ";
            if (topLevelOnly) {
                s << "{ ... }";
            } else {
                Ptr p = boost::any_cast<Ptr>(vp->back());
                if (p.get() == 0) {
                    s << "{ NULL }";
                } else {
                    s << '{' << std::endl;
                    s << p->toString(false, indent + "..");
                    s << indent << '}';
                }
            }
            s << std::endl;
        }
        else {
            s << indent << _format(*i);
        }
    }
    return s.str();
}

std::string dafBase::PropertySet::_format(std::string const& name) const {
    std::ostringstream s;
    s << std::showpoint; // Always show a decimal point for floats
    AnyMap::const_iterator j = _map.find(name);
    s << j->first << " = ";
    boost::shared_ptr< std::vector<boost::any> > vp = j->second;
    if (vp->size() > 1) {
        s << "[ ";
    }
    std::type_info const& t = vp->back().type();
    for (std::vector<boost::any>::const_iterator k = vp->begin();
         k != vp->end(); ++k) {
        if (k != vp->begin()) {
            s << ", ";
        }
        boost::any const& v(*k);
        if (t == typeid(bool)) {
            s << boost::any_cast<bool>(v);
        } else if (t == typeid(char)) {
            s << '\'' << boost::any_cast<char>(v) << '\'';
        } else if (t == typeid(signed char)) {
            s << '\'' << boost::any_cast<signed char>(v) << '\'';
        } else if (t == typeid(unsigned char)) {
            s << '\'' << boost::any_cast<unsigned char>(v) << '\'';
        } else if (t == typeid(short)) {
            s << boost::any_cast<short>(v);
        } else if (t == typeid(unsigned short)) {
            s << boost::any_cast<unsigned short>(v);
        } else if (t == typeid(int)) {
            s << boost::any_cast<int>(v);
        } else if (t == typeid(unsigned int)) {
            s << boost::any_cast<unsigned int>(v);
        } else if (t == typeid(long)) {
            s << boost::any_cast<long>(v);
        } else if (t == typeid(unsigned long)) {
            s << boost::any_cast<unsigned long>(v);
        } else if (t == typeid(long long)) {
            s << boost::any_cast<long long>(v);
        } else if (t == typeid(unsigned long long)) {
            s << boost::any_cast<unsigned long long>(v);
        } else if (t == typeid(float)) {
            s << std::setprecision(7) << boost::any_cast<float>(v);
        } else if (t == typeid(double)) {
            s << std::setprecision(14) << boost::any_cast<double>(v);
        } else if (t == typeid(std::string)) {
            s << '"' << boost::any_cast<std::string>(v) << '"';
        } else if (t == typeid(dafBase::DateTime)) {
            s << boost::any_cast<dafBase::DateTime>(v).toString();
        } else if (t == typeid(Ptr)) {
            s << "{ ... }";
        } else if (t == typeid(Persistable::Ptr)) {
            s << "<Persistable>";
        } else {
            s << "<Unknown>";
        }
    }
    if (j->second->size() > 1) {
        s << " ]";
    }
    s << std::endl;
    return s.str();
}

///////////////////////////////////////////////////////////////////////////////
// Modifiers
///////////////////////////////////////////////////////////////////////////////

/** Replace all values for a property name (possibly hierarchical) with a new
  * value. @bpdox{label:scalar}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Value to set.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertySet::set(std::string const& name, T const& value) {
    boost::shared_ptr< std::vector<boost::any> > vp(new std::vector<boost::any>);
    vp->push_back(value);
    _set(name, vp);
}

/** Replace all values for a property name (possibly hierarchical) with a
  * vector of new values. @bpdox{label:vector}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Vector of values to set.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertySet::set(std::string const& name,
                               std::vector<T> const& value) {
    if (value.empty()) return;
    boost::shared_ptr< std::vector<boost::any> > vp(new std::vector<boost::any>);
    vp->insert(vp->end(), value.begin(), value.end());
    _set(name, vp);
}

/** Replace all values for a property name (possibly hierarchical) with a
  * string value. @bpdox{label::string}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Character string (converted to \c std::string ).
  */
void dafBase::PropertySet::set(std::string const& name, char const* value) {
    set(name, std::string(value));
}

/** Appends a single value to the vector of values for a property name
  * (possibly hierarchical).  Sets the value if the property does not exist. @bpdox{label:scalar}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Value to append.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertySet::add(std::string const& name, T const& value) {
    AnyMap::iterator i = _find(name);
    if (i == _map.end()) {
        set(name, value);
    }
    else {
        if (i->second->back().type() != typeid(T)) {
            throw LSST_EXCEPT(TypeMismatchException,
                              name + " has mismatched type");
        }
        i->second->push_back(value);
    }
}

// Specialize for Ptrs to check for cycles.
template <> void dafBase::PropertySet::add<dafBase::PropertySet::Ptr>(
    std::string const& name, Ptr const& value) {
    AnyMap::iterator i = _find(name);
    if (i == _map.end()) {
        set(name, value);
    }
    else {
        if (i->second->back().type() != typeid(Ptr)) {
            throw LSST_EXCEPT(TypeMismatchException,
                              name + " has mismatched type");
        }
        _cycleCheckPtr(value, name);
        i->second->push_back(value);
    }
}

/** Appends a vector of values to the vector of values for a property name
  * (possibly hierarchical).  Sets the values if the property does not exist. @bpdox{label:vector}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Vector of values to append.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  * @note
  * May only partially add the vector if an exception occurs.
  */
template <typename T>
void dafBase::PropertySet::add(std::string const& name,
                               std::vector<T> const& value) {
    AnyMap::iterator i = _find(name);
    if (i == _map.end()) {
        set(name, value);
    }
    else {
        if (i->second->back().type() != typeid(T)) {
            throw LSST_EXCEPT(TypeMismatchException,
                              name + " has mismatched type");
        }
        i->second->insert(i->second->end(), value.begin(), value.end());
    }
}

// Specialize for Ptrs to check for cycles.
template<> void dafBase::PropertySet::add<dafBase::PropertySet::Ptr>(
    std::string const& name, std::vector<Ptr> const& value) {
    AnyMap::iterator i = _find(name);
    if (i == _map.end()) {
        set(name, value);
    }
    else {
        if (i->second->back().type() != typeid(Ptr)) {
            throw LSST_EXCEPT(TypeMismatchException,
                              name + " has mismatched type");
        }
        _cycleCheckPtrVec(value, name);
        i->second->insert(i->second->end(), value.begin(), value.end());
    }
}

/** Appends a <tt>char const*</tt> value to the vector of values for a
  * property name (possibly hierarchical).  Sets the value if the property
  * does not exist. @bpdox{label:string}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Character string value to append.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertySet::add(std::string const& name, char const* value) {
    add(name, std::string(value));
}

/** Replaces a single value vector in the destination with one from the
  * \a source.
  * @param[in] dest Destination property name.
  * @param[in] source PropertySet::Ptr for the source PropertySet.
  * @param[in] name Property name to extract.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Name does not exist in source.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertySet::copy(std::string const& dest,
                                ConstPtr source, std::string const& name) {
    if (source.get() == 0) {
        throw LSST_EXCEPT(pexExcept::InvalidParameterException,
                          "Missing source");
    }
    AnyMap::const_iterator sj = source->_find(name);
    if (sj == source->_map.end()) {
        throw LSST_EXCEPT(pexExcept::InvalidParameterException,
                          name + " not in source");
    }
    remove(dest);
    boost::shared_ptr< std::vector<boost::any> > vp(
        new std::vector<boost::any>(*(sj->second)));
    _set(dest, vp);
}

/** Appends all value vectors from the \a source to their corresponding
  * properties.  Sets values if a property does not exist.
  * @param[in] source PropertySet::Ptr for the source PropertySet.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  * @note
  * May only partially combine the PropertySets if an exception occurs.
  */
void dafBase::PropertySet::combine(ConstPtr source) {
    if (source.get() == 0) {
        return;
    }
    std::vector<std::string> names = source->paramNames(false);
    for (std::vector<std::string>::const_iterator i = names.begin();
         i != names.end(); ++i) {
        AnyMap::const_iterator sj = source->_find(*i);
        AnyMap::const_iterator dj = _find(*i);
        if (dj == _map.end()) {
            boost::shared_ptr< std::vector<boost::any> > vp(
                new std::vector<boost::any>(*(sj->second)));
            _set(*i, vp);
        }
        else {
            if (sj->second->back().type() != dj->second->back().type()) {
                throw LSST_EXCEPT(TypeMismatchException,
                                  *i + " has mismatched type");
            }
            // Check for cycles
            if (sj->second->back().type() == typeid(Ptr)) {
                _cycleCheckAnyVec(*(sj->second), *i);
            }
            dj->second->insert(dj->second->end(),
                               sj->second->begin(), sj->second->end());
        }
    }
}

/** Removes all values for a property name (possibly hierarchical).  Does
  * nothing if the property does not exist.
  * @param[in] name Property name to remove, possibly hierarchical.
  */
void dafBase::PropertySet::remove(std::string const& name) {
    std::string::size_type i = name.find('.');
    if (i == name.npos) {
        _map.erase(name);
        return;
    }
    std::string prefix(name, 0, i);
    AnyMap::iterator j = _map.find(prefix);
    if (j == _map.end() || j->second->back().type() != typeid(Ptr)) {
        return;
    }
    Ptr p = boost::any_cast<Ptr>(j->second->back());
    if (p.get() != 0) {
        std::string suffix(name, i + 1);
        p->remove(suffix);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Private member functions
///////////////////////////////////////////////////////////////////////////////

/** Finds the property name (possibly hierarchical).
  * @param[in] name Property name to find, possibly hierarchical.
  * @return unordered_map::iterator to the property or end() if nonexistent.
  */
dafBase::PropertySet::AnyMap::iterator
dafBase::PropertySet::_find(std::string const& name) {
    std::string::size_type i = name.find('.');
    if (i == name.npos) {
        return _map.find(name);
    }
    std::string prefix(name, 0, i);
    AnyMap::iterator j = _map.find(prefix);
    if (j == _map.end() || j->second->back().type() != typeid(Ptr)) {
        return _map.end();
    }
    Ptr p = boost::any_cast<Ptr>(j->second->back());
    if (p.get() == 0) {
        return _map.end();
    }
    std::string suffix(name, i + 1);
    AnyMap::iterator x = p->_find(suffix);
    if (x == p->_map.end()) {
        return _map.end();
    }
    return x;
}

/** Finds the property name (possibly hierarchical).  Const version.
  * @param[in] name Property name to find, possibly hierarchical.
  * @return unordered_map::const_iterator to the property or end().
  */
dafBase::PropertySet::AnyMap::const_iterator
dafBase::PropertySet::_find(std::string const& name) const {
    std::string::size_type i = name.find('.');
    if (i == name.npos) {
        return _map.find(name);
    }
    std::string prefix(name, 0, i);
    AnyMap::const_iterator j = _map.find(prefix);
    if (j == _map.end() || j->second->back().type() != typeid(Ptr)) {
        return _map.end();
    }
    Ptr p = boost::any_cast<Ptr>(j->second->back());
    if (p.get() == 0) {
        return _map.end();
    }
    std::string suffix(name, i + 1);
    AnyMap::const_iterator x = p->_find(suffix);
    if (x == p->_map.end()) {
        return _map.end();
    }
    return x;
}

/** Finds the property name (possibly hierarchical) and sets or replaces its
  * value with the given vector of values.  Hook for subclass overrides of
  * top-level setting.
  * @param[in] name Property name to find, possibly hierarchical.
  * @param[in] vp shared_ptr to vector of values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertySet::_set(
    std::string const& name, boost::shared_ptr< std::vector<boost::any> > vp) {
    _findOrInsert(name, vp);
}

/** Finds the property name (possibly hierarchical) and sets or replaces its
  * value with the given vector of values.
  * @param[in] name Property name to find, possibly hierarchical.
  * @param[in] vp shared_ptr to vector of values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertySet::_findOrInsert(
    std::string const& name, boost::shared_ptr< std::vector<boost::any> > vp) {
    // Check for cycles
    if (vp->back().type() == typeid(Ptr)) {
        _cycleCheckAnyVec(*vp, name);
    }

    std::string::size_type i = name.find('.');
    if (i == name.npos) {
        _map[name] = vp;
        return;
    }
    std::string prefix(name, 0, i);
    std::string suffix(name, i + 1);
    AnyMap::iterator j = _map.find(prefix);
    if (j == _map.end()) {
        PropertySet::Ptr pp(new PropertySet);
        pp->_findOrInsert(suffix, vp);
        boost::shared_ptr< std::vector<boost::any> > temp(new std::vector<boost::any>);
        temp->push_back(pp);
        _map[prefix] = temp;
        return;
    }
    else if (j->second->back().type() != typeid(Ptr)) {
        throw LSST_EXCEPT(pexExcept::InvalidParameterException,
                          prefix +
                          " exists but does not contain PropertySet::Ptrs");
    }
    Ptr p = boost::any_cast<Ptr>(j->second->back());
    if (p.get() == 0) {
        throw LSST_EXCEPT(pexExcept::InvalidParameterException,
                          prefix +
                          " exists but contains null PropertySet::Ptr");
    }
    p->_findOrInsert(suffix, vp);
}

void dafBase::PropertySet::_cycleCheckPtrVec(std::vector<Ptr> const& v,
                                             std::string const& name) {
    for (std::vector<Ptr>::const_iterator i = v.begin(); i != v.end(); ++i) {
        _cycleCheckPtr(*i, name);
    }
}

void dafBase::PropertySet::_cycleCheckAnyVec(std::vector<boost::any> const& v,
                                             std::string const& name) {
    for (std::vector<boost::any>::const_iterator i = v.begin(); i != v.end(); ++i) {
        _cycleCheckPtr(boost::any_cast<Ptr>(*i), name);
    }
}

void dafBase::PropertySet::_cycleCheckPtr(Ptr const& v,
                                         std::string const& name) {
    if (v.get() == this) {
        throw LSST_EXCEPT(pexExcept::InvalidParameterException,
                          name + " would cause a cycle");
    }
    std::vector<std::string> sets = v->propertySetNames(false);
    for (std::vector<std::string>::const_iterator i = sets.begin();
         i != sets.end(); ++i) {
        if (v->getAsPropertySetPtr(*i).get() == this) {
            throw LSST_EXCEPT(pexExcept::InvalidParameterException,
                              name + " would cause a cycle");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Explicit template instantiations
///////////////////////////////////////////////////////////////////////////////

/// @cond
// Explicit template instantiations are not well understood by doxygen.

#define INSTANTIATE(t) \
    template t dafBase::PropertySet::get<t>(std::string const& name) const; \
    template t dafBase::PropertySet::get<t>(std::string const& name, t const& defaultValue) const; \
    template std::vector<t> dafBase::PropertySet::getArray<t>(std::string const& name) const; \
    template void dafBase::PropertySet::set<t>(std::string const& name, t const& value); \
    template void dafBase::PropertySet::set<t>(std::string const& name, std::vector<t> const& value); \
    template void dafBase::PropertySet::add<t>(std::string const& name, t const& value); \
    template void dafBase::PropertySet::add<t>(std::string const& name, std::vector<t> const& value);

INSTANTIATE(bool)
INSTANTIATE(char)
INSTANTIATE(signed char)
INSTANTIATE(unsigned char)
INSTANTIATE(short)
INSTANTIATE(unsigned short)
INSTANTIATE(int)
INSTANTIATE(unsigned int)
INSTANTIATE(long)
INSTANTIATE(unsigned long)
INSTANTIATE(long long)
INSTANTIATE(unsigned long long)
INSTANTIATE(float)
INSTANTIATE(double)
INSTANTIATE(std::string)
INSTANTIATE(dafBase::PropertySet::Ptr)
INSTANTIATE(dafBase::Persistable::Ptr)
INSTANTIATE(dafBase::DateTime)

/// @endcond
