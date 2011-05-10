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
  * @brief Implementation for PropertyList class
  *
  * @version $Revision$
  * @date $Date$
  *
  * Contact: Kian-Tat Lim (ktl@slac.stanford.edu)
  */

#include "lsst/daf/base/PropertyList.h"

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
dafBase::PropertyList::PropertyList(void) : PropertySet() {
}

/** Destructor.
  */
dafBase::PropertyList::~PropertyList(void) {
}

///////////////////////////////////////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////////////////////////////////////

/** Copy the PropertyList and all of its contents.
  * @return PropertySet::Ptr pointing to the new copy.
  */
dafBase::PropertySet::Ptr dafBase::PropertyList::deepCopy(void) const {
    Ptr n(new PropertyList);
    n->PropertySet::combine(this->PropertySet::deepCopy());
    n->_order = _order;
    n->_comments = _comments;
    return n;
}

// The following throw an exception if the type does not match exactly.

/** Get the last value for a property name (possibly hierarchical). @bpdox{label:nodefault}
  * Note that the type must be explicitly specified for this template:
  * @code int i = propertyList.get<int>("foo") @endcode
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Last value set or added.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value does not match desired type.
  */
template <typename T>
T dafBase::PropertyList::get(std::string const& name) const { /* parasoft-suppress LsstDm-3-4a LsstDm-4-6 "allow template over bool" */
    return PropertySet::get<T>(name);
}

/** Get the last value for a property name (possibly hierarchical). @bpdox{label:withdefault}
  * Returns the provided @a defaultValue if the property does not exist.
  * Note that the type must be explicitly specified for this template:
  * @code int i = propertyList.get<int>("foo", 42) @endcode
  * @param[in] name Property name to examine, possibly hierarchical.
  * @param[in] defaultValue Default value to return if property does not exist.
  * @return Last value set or added.
  * @throws TypeMismatchException Value does not match desired type.
  */
template <typename T>
T dafBase::PropertyList::get(std::string const& name, T const& defaultValue) const { /* parasoft-suppress LsstDm-3-4a LsstDm-4-6 "allow template over bool" */
    return PropertySet::get<T>(name, defaultValue);
}

/** Get the vector of values for a property name (possibly hierarchical).
  * Note that the type must be explicitly specified for this template:
  * @code vector<int> v = propertyList.getArray<int>("foo") @endcode
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Vector of values.
  * @throws NotFoundException Property does not exist.
  * @throws TypeMismatchException Value does not match desired type.
  */
template <typename T>
std::vector<T> dafBase::PropertyList::getArray(std::string const& name) const {
    return PropertySet::getArray<T>(name);
}


/** Get the comment for a string property name (possibly hierarchical).
  * @param[in] name Property name to examine, possibly hierarchical.
  * @return Comment string.
  * @throws NotFoundException Property does not exist.
  */
std::string const& dafBase::PropertyList::getComment(
    std::string const& name) const {
    return _comments.find(name)->second;
}

std::vector<std::string> dafBase::PropertyList::getOrderedNames(void) const {
    std::vector<std::string> v;
    for (std::list<std::string>::const_iterator i = _order.begin();
         i != _order.end(); ++i) {
        v.push_back(*i);
    }
    return v;
}

std::list<std::string>::const_iterator
dafBase::PropertyList::begin(void) const {
    return _order.begin();
}

std::list<std::string>::const_iterator
dafBase::PropertyList::end(void) const {
    return _order.end();
}

/** Generate a string representation of the PropertyList.
  * Use this for debugging, not for serialization/persistence.
  * @param[in] topLevelOnly false (default) = do include subproperties.
  * @param[in] indent String to indent lines by (default none).
  * @return String representation of the PropertyList.
  */
std::string dafBase::PropertyList::toString(bool topLevelOnly,
                                           std::string const& indent) const {
    std::ostringstream s;
    for (std::list<std::string>::const_iterator i = _order.begin();
         i != _order.end(); ++i) {
        s << _format(*i);
        std::string const& comment = _comments.find(*i)->second;
        if (comment.size()) {
            s << "// " << comment << std::endl;
        }
    }
    return s.str();
}


///////////////////////////////////////////////////////////////////////////////
// Modifiers
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Normal versions of set/add with placement control

/** Replace all values for a property name (possibly hierarchical) with a new
  * value. @bpdox{label:scalar}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Value to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertyList::set(
    std::string const& name, T const& value, bool inPlace) {
    PropertySet::set(name, value);
    if (!inPlace) {
        _moveToEnd(name);
    }
}

/** Replace all values for a property name (possibly hierarchical) with a
  * string value. @bpdox{label:string}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Character string (converted to \c std::string ).
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertyList::set(
    std::string const& name, char const* value, bool inPlace) {
    set(name, std::string(value), inPlace);
    if (!inPlace) {
        _moveToEnd(name);
    }
}

/** Replace all values for a property name (possibly hierarchical) with a
  * vector of new values. @bpdox{label:vector}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Vector of values to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertyList::set(
    std::string const& name, std::vector<T> const& value, bool inPlace) {
    PropertySet::set(name, value);
    if (!inPlace) {
        _moveToEnd(name);
    }
}

/** Appends a single value to the vector of values for a property name
  * (possibly hierarchical).  Sets the value if the property does not exist. @bpdox{label:scalar}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Value to append.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertyList::add(
    std::string const& name, T const& value, bool inPlace) {
    PropertySet::add(name, value);
    if (!inPlace) {
        _moveToEnd(name);
    }
}

/** Appends a <tt>char const*</tt> value to the vector of values for a
  * property name (possibly hierarchical).  Sets the value if the property
  * does not exist.  @bpdox{label:string}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Value to append.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertyList::add(
    std::string const& name, char const* value, bool inPlace) {
    add(name, std::string(value), inPlace);
}

/** Appends a vector of values to the vector of values for a property name
  * (possibly hierarchical).  Sets the values if the property does not exist. @bpdox{label:vector}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Vector of values to append.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  * @note
  * May only partially add the vector if an exception occurs.
  */
template <typename T>
void dafBase::PropertyList::add(
    std::string const& name, std::vector<T> const& value, bool inPlace) {
    PropertySet::add(name, value);
    if (!inPlace) {
        _moveToEnd(name);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Commented versions of set/add

/** Replace all values for a property name (possibly hierarchical) with a new
  * value. @bpdox{label:scalar_c}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Value to set.
  * @param[in] comment Comment to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertyList::set(
    std::string const& name, T const& value,
    std::string const& comment, bool inPlace) {
    PropertySet::set(name, value);
    _commentOrderFix(name, comment, inPlace);
}

/** Replace all values for a property name (possibly hierarchical) with a
  * string value. @bpdox{label:string_c}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Character string value to set.
  * @param[in] comment Comment to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertyList::set(
    std::string const& name, char const* value,
    std::string const& comment, bool inPlace) {
    set(name, std::string(value), comment, inPlace);
}

/** Replace all values for a property name (possibly hierarchical) with a
  * vector of new values. @bpdox{label:vector_c}
  * @param[in] name Property name to set, possibly hierarchical.
  * @param[in] value Vector of values to set.
  * @param[in] comment Comment to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertyList::set(
    std::string const& name, std::vector<T> const& value,
    std::string const& comment, bool inPlace) {
    PropertySet::set(name, value);
    _commentOrderFix(name, comment, inPlace);
}

/** Appends a single value to the vector of values for a property name
  * (possibly hierarchical).  Sets the value if the property does not exist. @bpdox{label:scalar_c}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Value to append.
  * @param[in] comment Comment to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
template <typename T>
void dafBase::PropertyList::add(
    std::string const& name, T const& value,
    std::string const& comment, bool inPlace) {
    PropertySet::add(name, value);
    _commentOrderFix(name, comment, inPlace);
}

/** Appends a <tt>char const*</tt> value to the vector of values for a
  * property name (possibly hierarchical).  Sets the value if the property
  * does not exist. @bpdox{label:string_c}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value String value to append.
  * @param[in] comment Comment to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */

void dafBase::PropertyList::add(
    std::string const& name, char const* value,
    std::string const& comment, bool inPlace) {
    add(name, std::string(value), comment, inPlace);
}

/** Appends a vector of values to the vector of values for a property name
  * (possibly hierarchical).  Sets the values if the property does not exist. @bpdox{label:vector_c}
  * @param[in] name Property name to append to, possibly hierarchical.
  * @param[in] value Vector of values to append.
  * @param[in] comment Comment to set.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  * @note
  * May only partially add the vector if an exception occurs.
  */
template <typename T>
void dafBase::PropertyList::add(
    std::string const& name, std::vector<T> const& value,
    std::string const& comment, bool inPlace) {
    PropertySet::add(name, value);
    _commentOrderFix(name, comment, inPlace);
}


///////////////////////////////////////////////////////////////////////////////
// Other modifiers

/** Replaces a single value vector in the destination with one from the
  * \a source.
  * @param[in] dest Destination property name.
  * @param[in] source PropertySet::Ptr for the source PropertySet.
  * @param[in] name Property name to extract.
  * @param[in] inPlace If false, property is moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Name does not exist in source.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  */
void dafBase::PropertyList::copy(
    std::string const& dest, PropertySet::ConstPtr source,
    std::string const& name, bool inPlace) {
    PropertySet::copy(dest, source, name);
    ConstPtr pl =
        boost::dynamic_pointer_cast<PropertyList const, PropertySet const>(
            source);
    if (pl) {
        _comments[name] = pl->_comments.find(name)->second;
        if (!inPlace) {
            _moveToEnd(name);
        }
    }
}

/** Appends all value vectors from the \a source to their corresponding
  * properties.  Sets values if a property does not exist.
  * @param[in] source PropertySet::Ptr for the source PropertySet.
  * @param[in] inPlace If false, existing properties are moved to end of list.
  * @throws TypeMismatchException Type does not match existing values.
  * @throws InvalidParameterException Hierarchical name uses non-PropertySet.
  * @note
  * May only partially combine the PropertySets if an exception occurs.
  */
void dafBase::PropertyList::combine(PropertySet::ConstPtr source,
                                    bool inPlace) {
    ConstPtr pl =
        boost::dynamic_pointer_cast<PropertyList const, PropertySet const>(
            source);
    std::list<std::string> newOrder;
    if (pl) {
        newOrder = _order;
        for (std::list<std::string>::const_iterator i = pl->begin();
             i != pl->end(); ++i) {
            bool present = _comments.find(*i) != _comments.end();
            if (!present) {
                newOrder.push_back(*i);
            }
            else if (!inPlace) {
                newOrder.remove(*i);
                newOrder.push_back(*i);
            }
        }
    }
    PropertySet::combine(source);
    if (pl) {
        _order = newOrder;
        for (std::list<std::string>::const_iterator i = pl->begin();
             i != pl->end(); ++i) {
            _comments[*i] = pl->_comments.find(*i)->second;
        }
    }
}

/** Removes all values for a property name (possibly hierarchical).  Does
  * nothing if the property does not exist.
  * @param[in] name Property name to remove, possibly hierarchical.
  */
void dafBase::PropertyList::remove(std::string const& name) {
    PropertySet::remove(name);
    _comments.erase(name);
    _order.remove(name);
}

///////////////////////////////////////////////////////////////////////////////
// Private member functions
///////////////////////////////////////////////////////////////////////////////

void dafBase::PropertyList::_set(std::string const& name,
          boost::shared_ptr< std::vector<boost::any> > vp) {
    PropertySet::_set(name, vp);
    if (_comments.find(name) == _comments.end()) {
        _comments.insert(std::make_pair(name, std::string()));
        _order.push_back(name);
    }
}

void dafBase::PropertyList::_moveToEnd(std::string const& name) {
    _order.remove(name);
    _order.push_back(name);
}

void dafBase::PropertyList::_commentOrderFix(
    std::string const& name, std::string const& comment, bool inPlace) {
    _comments[name] = comment;
    if (!inPlace) {
        _moveToEnd(name);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Explicit template instantiations
///////////////////////////////////////////////////////////////////////////////

/// @cond
// Explicit template instantiations are not well understood by doxygen.

#define INSTANTIATE(t) \
    template t dafBase::PropertyList::get<t>(std::string const& name) const; \
    template t dafBase::PropertyList::get<t>(std::string const& name, t const& defaultValue) const; \
    template std::vector<t> dafBase::PropertyList::getArray<t>(std::string const& name) const; \
    template void dafBase::PropertyList::set<t>(std::string const& name, t const& value, bool inPlace); \
    template void dafBase::PropertyList::set<t>(std::string const& name, std::vector<t> const& value, bool inPlace); \
    template void dafBase::PropertyList::add<t>(std::string const& name, t const& value, bool inPlace); \
    template void dafBase::PropertyList::add<t>(std::string const& name, std::vector<t> const& value, bool inPlace); \
    template void dafBase::PropertyList::set<t>(std::string const& name, t const& value, std::string const& comment, bool inPlace); \
    template void dafBase::PropertyList::set<t>(std::string const& name, std::vector<t> const& value, std::string const& comment, bool inPlace); \
    template void dafBase::PropertyList::add<t>(std::string const& name, t const& value, std::string const& comment, bool inPlace); \
    template void dafBase::PropertyList::add<t>(std::string const& name, std::vector<t> const& value, std::string const& comment, bool inPlace); \
    template void dafBase::PropertyList::set<t>(std::string const& name, t const& value, char const* comment, bool inPlace); \
    template void dafBase::PropertyList::set<t>(std::string const& name, std::vector<t> const& value, char const* comment, bool inPlace); \
    template void dafBase::PropertyList::add<t>(std::string const& name, t const& value, char const* comment, bool inPlace); \
    template void dafBase::PropertyList::add<t>(std::string const& name, std::vector<t> const& value, char const* comment, bool inPlace);

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
