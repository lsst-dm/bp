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
 * @file PropertyPrinter.h
 * @brief definition of the PropertyPrinter class and its helpers
 * @author Ray Plante
 */
#ifndef LSST_PEX_PROPERTYPRINTER_H
#define LSST_PEX_PROPERTYPRINTER_H

#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <typeinfo>

#include "lsst/daf/base/PropertySet.h"
#include "lsst/daf/base/DateTime.h"
#include "boost/any.hpp"

namespace lsst {
namespace pex {
namespace logging {

namespace dafBase = lsst::daf::base;

/**
 * @brief an abstract iterator class used to print out property values
 *
 * The main purpose of implementations of this class is to provide (via 
 * its write(std::ostream*) function) the means for printing the values.
 * The other functions support iteration.  
 */
class PrinterIter {
public:

    virtual ~PrinterIter()=0;

    /**
     * write the value pointed to by this iterator to the given stream
     */
    virtual std::ostream& write(std::ostream *strm) const=0;

    /**
     * move the iterator forward one position
     */
    virtual PrinterIter& operator++()=0;

    /**
     * move the iterator backward one position
     */
    virtual PrinterIter& operator--()=0;

    /**
     * return true if this iterator is pointing to the same value as 
     * another pointer.  This method is not recommend for testing whether
     * an iterator is past the beginning or end of the list; notAtEnd() 
     * and notLTBegin() are more efficient.
     */
    virtual bool operator==(const PrinterIter& that) const=0;

    /**
     * return true if this iterator is not pointing to the same value as 
     * another pointer.  This method is not recommend for testing whether
     * an iterator is past the beginning or end of the list; notAtEnd() 
     * and notLTBegin() are more efficient.
     */
    virtual bool operator!=(const PrinterIter& that) const=0;

    /**
     * return true if this iterator is not pointing just past the last value
     * in the list.  
     */
    virtual bool notAtEnd() const=0;

    /**
     * return true if this iterator is not pointing before the first value
     * in the list.  Thus, *this should return a legal string.
     */
    virtual bool notLTBegin() const=0;

    /**
     * return a string version of the property value.  This simply uses
     * write(std::ostream*) to write to a string; consequently using 
     * write(std::ostream*) is more efficient for most purposes.
     */
    const std::string operator*() const {
        std::ostringstream out;
        write(&out);
        return out.str();
    }
};

using std::vector;

template <class T>
class BaseTmplPrinterIter : public PrinterIter {
public:
    BaseTmplPrinterIter(typename std::vector<T>::const_iterator listiter,
                        typename std::vector<T>::const_iterator beginiter,
                        typename std::vector<T>::const_iterator enditer) 
        : _it(listiter), _begin(beginiter), _end(enditer) { }
    virtual ~BaseTmplPrinterIter();
    virtual PrinterIter& operator++();
    virtual PrinterIter& operator--();
    virtual bool operator==(const PrinterIter& that) const;
    virtual bool operator!=(const PrinterIter& that) const;
    virtual bool notAtEnd() const;
    virtual bool notLTBegin() const;
    bool operator==(const BaseTmplPrinterIter& that) const { 
        return (this->_it == that._it); 
    }
    bool operator!=(const BaseTmplPrinterIter& that) const { 
        return (this->_it != that._it); 
    }
protected:
    typename std::vector<T>::const_iterator _it, _begin, _end;
};

template <class T>
BaseTmplPrinterIter<T>::~BaseTmplPrinterIter() { }
template <class T>
PrinterIter& BaseTmplPrinterIter<T>::operator++() { ++_it; return *this; }
template <class T>
PrinterIter& BaseTmplPrinterIter<T>::operator--() { --_it; return *this; }
template <class T>
bool BaseTmplPrinterIter<T>::operator==(const PrinterIter& that) const {
    const BaseTmplPrinterIter<T> *thit = 
        dynamic_cast<const BaseTmplPrinterIter<T>*>(&that);
    return (thit == 0) ? false : (*this == *thit);
}
template <class T>
bool BaseTmplPrinterIter<T>::operator!=(const PrinterIter& that) const {
    const BaseTmplPrinterIter<T> *thit = 
        dynamic_cast<const BaseTmplPrinterIter<T>*>(&that);
    return (thit == 0) ? true : (*this != *thit);
}
template <class T>
bool BaseTmplPrinterIter<T>::notAtEnd() const { return (_it != _end); }
template <class T>
bool BaseTmplPrinterIter<T>::notLTBegin() const { return (_it >= _begin); } 

template <class T>
class TmplPrinterIter : public BaseTmplPrinterIter<T> {
public:
    TmplPrinterIter(typename std::vector<T>::const_iterator listiter,
                    typename std::vector<T>::const_iterator beginiter,
                    typename std::vector<T>::const_iterator enditer) 
        : BaseTmplPrinterIter<T>(listiter, beginiter, enditer) { }
    virtual std::ostream& write(std::ostream *strm) const;
};

template <class T>
std::ostream& TmplPrinterIter<T>::write(std::ostream *strm) const { 
    (*strm) << *(this->_it); 
    return *strm;
}


/**
 * @brief  a wrapper PrinterIter class that hides the polymorphic (and 
 * possibly templatized) nature of an underlying PrinterIter subclass.  
 *
 * This is the iterator type available from the PropertyPrinter class.
 */
class WrappedPrinterIter : public PrinterIter {
public:
    WrappedPrinterIter(boost::shared_ptr<PrinterIter> iter) : _it(iter) { }
    virtual ~WrappedPrinterIter();
    virtual std::ostream& write(std::ostream *strm) const;
    virtual PrinterIter& operator++();
    virtual PrinterIter& operator--();
    virtual bool operator==(const PrinterIter& that) const;
    virtual bool operator!=(const PrinterIter& that) const;
    virtual bool notAtEnd() const;
    virtual bool notLTBegin() const;
private:
    boost::shared_ptr<PrinterIter> _it;
};

/**
 * @brief an abstract class that encapsulates a list of property values
 * to be printed out.
 * 
 * The purpose of this class is wrap a list of property values (all of the 
 * same type) and provide an iterator for printing out those values.  This 
 * class is not used directly by applications but internally via the 
 * PropertyPrinter.  Since PropertyPrinter does not support all types by 
 * default, an application may need to subclass this class in order to 
 * support printing of a new type.  
 *
 * To support a new type in PropertyPrinter (via PrinterFactory) one must 
 * subclass this class and the PrinterIter class (the latter actually 
 * controls how the type gets printed to an output stream).  
 *
 * For types that already support the output operator (<<), an explicit
 * subclass need not be defined.  Instead, one can instantiate the 
 * makePrinter<T> factory function; the required subclasses will be 
 * generated automatically.  
 *
 * The PropertyPrinter.h file also provide support for printing DateTime
 * properties via the explicitly defined DateTimePrinterList and 
 * DateTimePrinterIter because DateTime does not support the output (<<)
 * operator.  See the implementation of these classes for a good example 
 * supporting other types of this sort.  
 */
class PrinterList {
public:
    typedef WrappedPrinterIter iterator;

    virtual ~PrinterList();

    /**
     * return a PrinterIter set at the first property value 
     */
    virtual iterator begin() const=0;

    /**
     * return a PrinterIter set at the last property value 
     */
    virtual iterator last() const=0;

    /**
     * return the number values available in the internal property value list
     */
    virtual size_t valueCount() const=0;
};

template <class T>
class BaseTmplPrinterList : public PrinterList {
public:
    BaseTmplPrinterList(const dafBase::PropertySet& prop, 
                        const std::string& name)
        : _list(prop.getArray<T>(name)) { }

    virtual ~BaseTmplPrinterList();
    virtual size_t valueCount() const;

protected:
    std::vector<T> _list;
};

template <class T>
BaseTmplPrinterList<T>::~BaseTmplPrinterList() { }
template <class T>
size_t BaseTmplPrinterList<T>::valueCount() const { return _list.size(); }

template <class T>
class TmplPrinterList : public BaseTmplPrinterList<T> {
public:
    TmplPrinterList(const dafBase::PropertySet& prop, const std::string& name)
        : BaseTmplPrinterList<T>(prop, name) { }

    virtual PrinterList::iterator begin() const;
    virtual PrinterList::iterator last() const;

private:
    typedef TmplPrinterIter<T> delegateIter;
};

template <class T>
typename PrinterList::iterator TmplPrinterList<T>::begin() const { 
    PrinterIter *it = new delegateIter(BaseTmplPrinterList<T>::_list.begin(), 
                                       BaseTmplPrinterList<T>::_list.begin(), 
                                       BaseTmplPrinterList<T>::_list.end());
    return PrinterList::iterator(boost::shared_ptr<PrinterIter>(it));
}
template <class T>
typename PrinterList::iterator TmplPrinterList<T>::last() const { 
    PrinterIter *it = new delegateIter(BaseTmplPrinterList<T>::_list.end()-1, 
                                       BaseTmplPrinterList<T>::_list.begin(), 
                                       BaseTmplPrinterList<T>::_list.end());
    return PrinterList::iterator(boost::shared_ptr<PrinterIter>(it));
}

/**
 * @brief the template factory function for supporting printing of printable 
 * types
 *
 * By instantiating this template, one automatically generates not only this 
 * function by also the PrinterList and PrinterIter classes it creates.  This 
 * will work for types that can be printed to a stream via the output (<<) 
 * operator.  
 */
template <class T> 
PrinterList* makePrinter(const dafBase::PropertySet& prop, 
                         const std::string& name) {
    return new TmplPrinterList<T>(prop, name);
}

class DateTimePrinterIter : public BaseTmplPrinterIter<dafBase::DateTime> {
public:
    DateTimePrinterIter(std::vector<dafBase::DateTime>::const_iterator listiter,
                        std::vector<dafBase::DateTime>::const_iterator beginiter,
                        std::vector<dafBase::DateTime>::const_iterator enditer)
        : BaseTmplPrinterIter<dafBase::DateTime>(listiter, beginiter, enditer) 
    { }

    virtual ~DateTimePrinterIter();
    virtual std::ostream& write(std::ostream *strm) const;
};

class DateTimePrinterList : public BaseTmplPrinterList<dafBase::DateTime> {
public:
    DateTimePrinterList(const dafBase::PropertySet& prop, 
                        const std::string& name) 
        : BaseTmplPrinterList<dafBase::DateTime>(prop, name) { }

    virtual ~DateTimePrinterList();
    virtual iterator begin() const;
    virtual iterator last() const;
};

PrinterList* makeDateTimePrinter(const dafBase::PropertySet& prop, 
                                 const std::string& name);

class BoolPrinterIter : public BaseTmplPrinterIter<bool> {
public:
    BoolPrinterIter(std::vector<bool>::const_iterator listiter,
                        std::vector<bool>::const_iterator beginiter,
                        std::vector<bool>::const_iterator enditer)
        : BaseTmplPrinterIter<bool>(listiter, beginiter, enditer) { }
    virtual ~BoolPrinterIter();
    virtual std::ostream& write(std::ostream *strm) const;
};

class BoolPrinterList : public BaseTmplPrinterList<bool> {
public:
    BoolPrinterList(const dafBase::PropertySet& prop, const std::string& name) 
        : BaseTmplPrinterList<bool>(prop, name) { }

    virtual ~BoolPrinterList();
    virtual iterator begin() const;
    virtual iterator last() const;
};

PrinterList* makeBoolPrinter(const dafBase::PropertySet& prop, 
                             const std::string& name);

/**
 * @brief a factory used to create PrinterList instances to be used by 
 * a PropertyPrinter instance.   
 *
 * PrinterList classes are used to print out property values of a particular 
 * type.  Typically, a default PrinterFactory is filled with support for 
 * the common primitive types and type classes that are important for 
 * logging.  Other types can be supported by providing an implementation
 * of PrinterList and corresponding iterator that support the type as well 
 * as an implementation of a factory function that creates a pointer to 
 * the specialized PrinterList.  A pointer to the factory function is what 
 * one registers with this general PrinterFactory.  
 * 
 * PrinterFactory::factoryFuncPtr defines the form of the factory function:
 * @code
 *   PrinterList* (*makePrinterList)(const PropertySet&, const string&)
 * @endcode
 * 
 * See PrinterList for more details about supporting a new type.
 */
class PrinterFactory {
public: 
    typedef PrinterList* 
        (*factoryFuncPtr)(const dafBase::PropertySet&, const std::string&);
        
    PrinterFactory(bool loadDefaults=false) : _factFuncs() { 
        if (loadDefaults) _loadDefaults();
    }

    void add(const std::type_info& tp, factoryFuncPtr func) { 
        _factFuncs[tp.name()] = func; 
    }

    PrinterList* makePrinter(const dafBase::PropertySet& prop, 
                             const std::string& name) const 
    {
        Lookup::const_iterator fi = _factFuncs.find(prop.typeOf(name).name());
        return (fi == _factFuncs.end()) ? 0 : (*(fi->second))(prop, name);
    }

private:
    void _loadDefaults();

    typedef std::map<std::string, factoryFuncPtr> Lookup;
    Lookup _factFuncs;
};

/**
 * @brief  an class for printing the values associated with a name in a 
 * PropertySet.
 * 
 * This class is vital if one needs to print out all the values in a 
 * PropertySet without knowing the data types associated with the names.  
 * In particular, it gives one the ability to write each value associated 
 * with a name individually and it any order.  
 * A typical use looks like this:
 * @code
 *   PropertySet ps;
 *   std::vector<std::string>::iterator ni;
 *   std::vector<std::string> names(ps.paramNames(true));
 * 
 *   for(ni = names.begin(); ni != names.end(); ++ni) {
 *     PropertyPrinter pp(ps, *ni);
 * 
 *     for(PropertyPrinter::iterator i=pp.begin(); i.notAtEnd(); ++i) {
 *       strm << *ni << ": ";
 *       i.write(&strm) << endl;
 *     }
 *   }
 * @endcode
 *
 * A PropertyPrinter is constructed for a particular name found in the 
 * PropertySet.  (If a property with that name doesn't exist, a
 * pex::exceptions::NotFoundException is thrown.)  The PropertyPrinter is 
 * wrapped around the array of values associated with the name.  One can 
 * get an Iterator for the values and send each one to a stream.
 *
 * A PropertyPrinter can only print out property types that it has been 
 * configured for via the PrinterFactory instance that is passed in during 
 * construction.  If one is not provided, a default is set which supports 
 * the following types:
 * @verbatim
 *   short          char              float
 *   int            signed char       double
 *   long           unsigned char     bool
 *   long long      string            DateTime
 * @endverbatim
 * Other types can be supported by passing the name of factory function 
 * that can create a printer instance to a PrinterFactory that will eventually 
 * be used with a PropertyPrinter (see PrinterFactory for details.  
 */
class PropertyPrinter {
public:

    /**
     * the iterator type used for iterating through values.
     */
    typedef PrinterList::iterator iterator;

    /**
     * the default look-up table for creating iterators for different types.
     * Users are allowsd add new printers.  See PropertyFactory for more 
     * details.
     */
    static PrinterFactory defaultPrinterFactory;

    /**
     * create the PropertyPrinter for a given name
     */
    PropertyPrinter(const dafBase::PropertySet& prop, const std::string& name, 
                    const PrinterFactory& fact=defaultPrinterFactory);

    /**
     * return an iterator set at the start of the list of values
     */
    iterator begin();

    /**
     * return an iterator set at the last of the list of values
     */
    iterator last();

    /**
     * return the number of values associated with the property
     */
    size_t valueCount() { return _list.get()->valueCount(); }


private:
    boost::shared_ptr<PrinterList> _list;
};

}}}     // end lsst::pex::logging

#endif  // end LSST_PEX_PROPERTYPRINTER_H

