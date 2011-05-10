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
 * @file PropertyPrinter.cc
 * @author Ray Plante
 */
#include "lsst/pex/logging/PropertyPrinter.h"
#include "lsst/daf/base/DateTime.h"
#include <boost/any.hpp>

namespace lsst {
namespace pex {
namespace logging {

//@cond
using std::vector;
using std::ostream;
using lsst::daf::base::PropertySet;
using lsst::daf::base::DateTime;

PrinterIter::~PrinterIter() { }

WrappedPrinterIter::~WrappedPrinterIter() { }

std::ostream& WrappedPrinterIter::write(std::ostream *strm) const {
    _it.get()->write(strm);
    return *strm;
}

PrinterIter& WrappedPrinterIter::operator++() { ++(*_it); return *_it; }
PrinterIter& WrappedPrinterIter::operator--() { --(*_it); return *_it; }
bool WrappedPrinterIter::operator==(const PrinterIter& that) const { 
    return (*_it == that);
}
bool WrappedPrinterIter::operator!=(const PrinterIter& that) const { 
    return (*_it == that);
}
bool WrappedPrinterIter::notAtEnd() const {
    return _it.get()->notAtEnd();
}
bool WrappedPrinterIter::notLTBegin() const {
    return _it.get()->notLTBegin();
}

PrinterList::~PrinterList() { }

DateTimePrinterIter::~DateTimePrinterIter() { }

std::ostream& DateTimePrinterIter::write(std::ostream *strm) const {
    (*strm) << _it->nsecs();
    return *strm;
}

DateTimePrinterList::~DateTimePrinterList() { }

DateTimePrinterList::iterator DateTimePrinterList::begin() const { 
    PrinterIter *it = new DateTimePrinterIter(_list.begin(), 
                                              _list.begin(), _list.end());
    return iterator(boost::shared_ptr<PrinterIter>(it));
}
DateTimePrinterList::iterator DateTimePrinterList::last() const { 
    PrinterIter *it = new DateTimePrinterIter(_list.end()-1, 
                                              _list.begin(), _list.end());
    return iterator(boost::shared_ptr<PrinterIter>(it));
}

PrinterList* makeDateTimePrinter(const PropertySet& prop, 
                                 const std::string& name) 
{
    return new DateTimePrinterList(prop, name);
}

BoolPrinterIter::~BoolPrinterIter() { }

std::ostream& BoolPrinterIter::write(std::ostream *strm) const {
    (*strm) << ((*_it) ? "true" : "false");
    return *strm;
}

BoolPrinterList::~BoolPrinterList() { }

BoolPrinterList::iterator BoolPrinterList::begin() const { 
    PrinterIter *it = new BoolPrinterIter(_list.begin(), 
                                          _list.begin(), _list.end());
    return iterator(boost::shared_ptr<PrinterIter>(it));
}
BoolPrinterList::iterator BoolPrinterList::last() const { 
    PrinterIter *it = new BoolPrinterIter(_list.end()-1, 
                                          _list.begin(), _list.end());
    return iterator(boost::shared_ptr<PrinterIter>(it));
}

PrinterList* makeBoolPrinter(const PropertySet& prop, 
                             const std::string& name) 
{
    return new BoolPrinterList(prop, name);
}

#define PF_ADD(T)  add(typeid(T), &lsst::pex::logging::makePrinter<T>)

void PrinterFactory::_loadDefaults() {
    PF_ADD(short);
    PF_ADD(int);
    PF_ADD(long);
    PF_ADD(long long);
    PF_ADD(float);
    PF_ADD(double);
    PF_ADD(char);
    PF_ADD(signed char);
    PF_ADD(unsigned char);
    PF_ADD(std::string);
    add(typeid(bool), makeBoolPrinter);
    add(typeid(DateTime), makeDateTimePrinter);
}

PrinterFactory PropertyPrinter::defaultPrinterFactory(true);

PropertyPrinter::PropertyPrinter(const PropertySet& prop, 
                                 const std::string& name, 
                                 const PrinterFactory& fact) 
    : _list(fact.makePrinter(prop, name)) 
{
    if (_list.get() == 0) {
        PropertySet tmp;
        tmp.set(name, "<unprintable>");
        _list = boost::shared_ptr<PrinterList>(fact.makePrinter(tmp, name));
    }
}
 
PropertyPrinter::iterator PropertyPrinter::begin() {
    return _list.get()->begin();
}

PropertyPrinter::iterator PropertyPrinter::last() {
    return _list.get()->last();
}

//@endcond
}}} // end lsst::pex::logging

