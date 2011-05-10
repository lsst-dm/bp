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
 * @file LogRecord.h
 * @brief definition of the LogRecord, RecordProperty and Prop classes
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOGRECORD_H
#define LSST_PEX_LOGRECORD_H

#include "lsst/daf/base/PropertySet.h"

#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <string>
#include <sys/time.h>

#define LSST_LP_COMMENT     "COMMENT"
#define LSST_LP_TIMESTAMP   "TIMESTAMP"
#define LSST_LP_DATE        "DATE"
#define LSST_LP_LOG         "LOG"
#define LSST_LP_LEVEL       "LEVEL"

namespace lsst {
namespace pex {
namespace logging {

namespace dafBase = lsst::daf::base;

/**
 * @brief a container for a named data property for a LogRecord
 *
 * This light-weight container is meant to facilitate adding an arbitrary 
 * named data item to a LogRecord.  The value will be stored as a reference 
 * to original item passed in; thus, this should be used only in the same
 * scope as the arguments.
 *
 * This class is usually accessed by applications via RecData, a typedef
 * defined in Log.h.  
 */
template <class T>
class RecordProperty {
public:
    typedef T ValueType;

    /**
     * wrap a name and value.  The value will be stored as a reference to
     * original item passed in; thus, this should be used only in the same
     * scope as the arguments
     */
    RecordProperty(const std::string& pname, const T& pvalue) 
        : name(pname), value(pvalue) { }

    /**
     * add the name-value pair to a PropertySet
     */
    void addTo(dafBase::PropertySet& set) { set.add(this->name, this->value); }

    const std::string name;
    const T& value;
};

/*
 * a shorthand version of the RecordProperty class
 */
template <class T>
class Prop : public RecordProperty<T> { 
public:
    Prop(const std::string& pname, const T& value) 
        : RecordProperty<T>(pname, value) { }
};

/**
 * @brief a container for constructing a single Log record
 *
 * The purpose of this class is to collect data for inclusion in a message
 * to a Log.  
 */
class LogRecord {
public:

    /**
     * Create a log record to be sent to a given log.  The current time is 
     * recorded and set as the DATE property.
     * @param threshold  the importance threshold that determines if a message
     *                     is printed.
     * @param importance  the loudness of the record.  If this value is 
     *                     greater than or equal to the given importance 
     *                     threshold, the message will be recorded.
     * @param showAll    if true, prefer showing all properties when 
     *                     rendering this record.  The default is false.
     *                     (See willShow().)
     */
    LogRecord(int threshold, int importance, bool showAll=false);

    /**
     * Create a log record to be sent to a given log.  The current time is 
     * recorded and set as the DATE property.
     * @param threshold  the importance threshold that determines if a message
     *                     is printed.
     * @param importance  the loudness of the record.  If this value is 
     *                     greater than or equal to the given importance 
     *                     threshold, the message will be recorded.
     * @param preamble   an ordered set of properties that constitute the 
     *                     preamble of this message.  This should not include
     *                     the current time.  
     * @param showAll    if true, prefer showing all properties when 
     *                     rendering this record.  The default is false.
     *                     (See willShowAll().)
     */
    LogRecord(int threshold, int importance, 
              const dafBase::PropertySet& preamble, bool showAll=false);

    /**
     * create a copy of a record
     */
    LogRecord(const LogRecord& that) 
        : _send(that._send), _showAll(that._showAll), _vol(that._vol), _data()
    { 
        _data = that._data->deepCopy();
    }   

    /**
     * delete this log record
     */
    virtual ~LogRecord();

    /**
     * copy another record into this one.
     */
    LogRecord& operator=(const LogRecord& that) {
        _send = that._send; 
        _showAll = that._showAll; 
        _vol = that._vol; 
        _data = that._data;
        return *this;
    }

    /**
     * add a string comment to this record.  The comment will get stored 
     * in the data property under the key name, "COMMENT".  The comment will 
     * only get added when willRecord() is returns true which is set when
     * the record is constructed (usually by a Log object).  
     */
    void addComment(const std::string& comment) {
        if (_send) _data->add(LSST_LP_COMMENT, comment);
    }

    /**
     * add a string comment to this record.  This version is provided 
     * as a convenience and is equivalent to addComment(comment.str()).
     * @bpdox{ignore}
     */
    void addComment(const boost::format& comment) {
        if (_send) addComment(comment.str());
    }

    /**
     * attach a named item of data to this record.
     */
    template <class T>
    void addProperty(const RecordProperty<T>& property);

    /**
     * attach a named item of data to this record.
     * @bpdox{label:separate}
     */
    template <class T>
    void addProperty(const std::string& name, const T& val);

    /**
     * add all of the properties found in the given PropertySet.  
     * This will make sure not to overwrite critical properties, 
     * LEVEL, LOG, TIMESTAMP, and DATE.  
     */
    void addProperties(const dafBase::PropertySet& props);

    /**
     * add all of the properties found in the given PropertySet.  
     * This will make sure not to overwrite critical properties, 
     * LEVEL, LOG, TIMESTAMP, and DATE.  
     * @bpdox{ignore}
     */
    void addProperties(const dafBase::PropertySet::Ptr& props) {
        addProperties(*props);
    }

    /**
     * return the read-only data properties that make up this log message.
     * This is a synonym for data().
     * @bpdox{label:const}
     */
    const dafBase::PropertySet& getProperties() const { return data(); }

    /**
     * return the data properties that make up this log message.  
     * This is a synonym for data().
     * @bpdox{label:nonconst}
     */
    dafBase::PropertySet& getProperties() { return data(); }

    /**
     * return the data properties that make up this log message.  
     * This is a synonym for getProperties().
     * @bpdox{label:const}
     */
    const dafBase::PropertySet& data() const { return *_data; }

    /**
     * return the data properties that make up this log message.  
     * This is a synonym for getProperties().
     * @bpdox{label:nonconst}
     */
    dafBase::PropertySet& data() { return *_data; }

    /**
     * return the number available property parameter names (i.e. ones 
     * that return non-PropertySet values). 
     */
    size_t countParamNames() {
        std::vector<std::string> names = data().paramNames(false);
        return names.size();
    }

    /**
     * return the total number of data property values currently contained 
     * in this log record.  This function will sum the number values 
     * associated with a name, summed over all available names (including 
     * subproperties).  This total does not include PropertySet values.  
     */
    size_t countParamValues() const;

    /**
     * return the importance level--a measure of "loudness"--associated with 
     * this record.  
     */
    int getImportance() const { return _vol; }

    /**
     * return true if the importance level is set loud enough to cause 
     * this record to be actually recorded.  This value is set at
     * construction time only.  
     */
    bool willRecord() const { return _send; }

    /**
     * return true if there is a preference by the creator of this record
     * to show all the properties associated with this record.  A
     * LogFormatter may or may not choose to honor this preference when 
     * this LogRecord is rendered.
     */
    bool willShowAll() const { return _showAll; } 

    /**
     * set whether all of the properties attached to this record should 
     * be displayed when the record is rendered.  A LogFormatter may or may 
     * not choose to honor the preference, according to the purposes of 
     * its implmentation.  
     * @param yesno    the preference for showing all.  willShowAll() will 
     *                    return this value.  
     */
    void setShowAll(bool yesno) {  _showAll = yesno;  }

    /**
     * set the TIMESTAMP property to the current time.  The value is stored as 
     * a lsst::daf::base::DateTime instance.  
     * 
     * This is called automatically in the constructor, but it can be reset 
     * at any time.  If there is also a DATE property, it will be updated 
     * as well (see setDate()). 
     */  
    void setTimestamp();

    /**
     * set the DATE property to the current value of the TIMESTAMP property.
     * The value is a string representation of the TIMESTAMP property, 
     * formatted for read-able display.
     *
     * Unlike setTimestamp(), this function is not called automatically within 
     * the LogRecord constructor; thus, a user of this class must call it 
     * explicitly to have this property.  
     *
     * This function is intended for use by a Log class that sets the value
     * just before sending this record to the LogDestinations.
     */
    virtual void setDate();

    /**
     * return the current UTC time in nanosecs since Jan 1, 1970.  This value
     * is suitable for passing to a DateTime constructor.  
     */
    static long long utcnow();

protected: 
    LogRecord() : _send(false), _vol(10), _data(new dafBase::PropertySet()) { }

    /**
     * initialize this record with the DATE and LEVEL properties
     */
    void _init() {
        if (_send) {
            _data->set("LEVEL", _vol);
            setDate();
        }
    }

    bool _send;    // true if this record should be sent to the log
    bool _showAll; // true if there is preference to have all data displayed
    int _vol;      // the importance volume of this message
    dafBase::PropertySet::Ptr _data;
};

template <class T>
void LogRecord::addProperty(const RecordProperty<T>& property) {
    if (_send) property.addTo(_data);
}

template <class T>
void LogRecord::addProperty(const std::string& name, const T& val) {
    if (_send) data().add(name, val);
}



}}} // end lsst::pex::logging

#endif // end LSST_PEX_LOGRECORD_H
