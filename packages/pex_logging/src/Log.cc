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
 * @file Log.cc
 * @author Ray Plante
 */

#include <cstring>
#include <cstdio>

#include "lsst/pex/logging/Log.h"
#include "lsst/pex/logging/ScreenLog.h"

#include <boost/shared_ptr.hpp>

using namespace std;

namespace lsst {
namespace pex {
namespace logging {

//@cond

using std::string;
using std::list;
using std::ostream;
using boost::shared_ptr;
using lsst::daf::base::PropertySet;

///////////////////////////////////////////////////////////
//  Log
///////////////////////////////////////////////////////////

/*
 * the conventional importance level for messages that aid in debugging.  
 * This value is set to -10 with the intention that messages with 
 * negative importance levels (or more precisely, >= -10) will be printed 
 * when the threshold is set to this value.  
 */
const int Log::DEBUG = -10;

/*
 * the conventional importance level for messages that are informational
 * and which report on normal behavior.  The value is set to 0 with the 
 * intention that this is the default threshold for logs and their 
 * destinations.  
 */
const int Log::INFO = 0;

/*
 * the conventional importance level for messages that warn about 
 * abnormal but non-fatal behavior.  The value is set to 10.
 */
const int Log::WARN = 10;

/*
 * the conventional importance level for messages that report on fatal
 * behavior.  The value is set to 20.  Note that the logging module 
 * makes no attempt to shutdown execution or other wise affect control 
 * flow when a message having a importance exceeding this level.  
 */
const int Log::FATAL = 20;

/*
 * a magic threshold value that indicates that a threshold of a Log
 * should be set to its nearest ancestor
 */
const int Log::INHERIT_THRESHOLD = threshold::INHERIT;

/*
 * create a null log.  This constructor should 
 * not normally be employed to obtain a Log; the static getDefaultLog() 
 * method should be used instead.  This is provided primarily for 
 * subclasses and containers that require a no-arg constructor.  
 * @param threshold   the initial importance threshold for this log
 * @param name        the initial name for this log.  An empty string 
 *                    (the default) denotes a root log.
 */
Log::Log(const int threshold, const string& name) 
    : _threshold(threshold), _defShowAll(new bool(false)), _myShowAll(), 
      _name(name), _thresholds(new threshold::Memory(Log::_sep)), 
      _destinations(), _preamble(new PropertySet())
{
    _thresholds->setRootThreshold(threshold);
    if (name.length() > 0) _thresholds->setThresholdFor(name, threshold);
    _myShowAll = _defShowAll;
    completePreamble();
}

/*
 * create a fully configured Log.  This constructor is 
 * not normally employed to obtain a Log; the static getDefaultLog() 
 * method or the createChildLog() method of should be used instead.  
 * @param destinations   the list of LogDestinations to attach to this 
 *                         Log.
 * @param preamble       a list of data properties that should be included 
 *                         with every recorded message to the Log.  This
 *                         constructor will automatically a property ("LOG")
 *                         giving the Log name.  
 * @param name           the name to give this log.  By default, the 
 *                         name will be an empty string, signifying a 
 *                         root log.  
 * @param threshold      the importance threshold to assign to this Log.  
 *                         Messages sent to this log must have a importance
 *                         level equal to or greater than this value to 
 *                         be recorded.  (Note that thresholds associated
 *                         with destinations have their own thresholds that
 *                         will override this one.)
 */
Log::Log(const list<shared_ptr<LogDestination> > &destinations, 
         const PropertySet &preamble,
         const string &name, const int threshold, bool defaultShowAll)
    : _threshold(threshold), _defShowAll(new bool(defaultShowAll)), 
      _myShowAll(), _name(name), _thresholds(new threshold::Memory(Log::_sep)),
      _destinations(destinations), _preamble(preamble.deepCopy())
{  
    _thresholds->setRootThreshold(threshold);
    if (name.length() > 0) _thresholds->setThresholdFor(name, threshold);
    completePreamble();
}

/*
 * create a copy
 */
Log::Log(const Log& that) 
    : _threshold(that._threshold), _defShowAll(that._defShowAll), 
      _myShowAll(that._myShowAll), _name(that._name), 
      _thresholds(that._thresholds), _destinations(that._destinations), 
      _preamble(that._preamble->deepCopy())
{ }

/* 
 * delete this Log
 */
Log::~Log() { }

/*
 * create a copy
 */
Log& Log::operator=(const Log& that) {
    _threshold = that._threshold; 
    _defShowAll = that._defShowAll;
    _myShowAll = that._myShowAll;
    _name = that._name;
    _thresholds = that._thresholds;
    _destinations = that._destinations;
    _preamble = that._preamble->deepCopy();
    return *this;
}

void Log::completePreamble() {
    _preamble->set<string>("LOG", _name);
}

/*
 * create a child of a given Log
 * @param parent        the Log that will serve as its parent.  
 * @param childName     the name of the child log, relative to the given 
 *                          parent.  The full child name will be 
 *                          constructed from this name prepended by the 
 *                          parent's name and a ".".  If the parent Log 
 *                          is a root Log, then the "." is not prepended.
 * @param threshold     the threshold for the new child Log.  The default
 *                          value is Log::INHERIT_THRESHOLD which means that 
 *                          it will be set to the threshold of the parent.  
 */
Log::Log(const Log& parent, const string& childName, int threshold)
    : _threshold(threshold), _defShowAll(parent._defShowAll), 
      _myShowAll(), _name(parent.getName()), _thresholds(parent._thresholds), 
      _destinations(parent._destinations), 
      _preamble(parent._preamble->deepCopy())  
{ 
    if (_name.length() > 0) _name += _sep;
    _name += childName;

    if (_threshold > INHERIT_THRESHOLD) 
        _thresholds->setThresholdFor(_name, _threshold);

    completePreamble();
}

/*
 * set the importance threshold for a child Log.  When a child Log of the
 * same name is created, it will be assigned this threshold.  Any existing
 * Log object with name that has already explicitly set its importance
 * threshold will not be affected; however, those that are set to inherit
 * the threshold will be. 
 * @param name       the relative name of the child log.  This cannot be an
 *                      empty string.  
 * @param threshold  the importance threshold to set Logs with this name to.
 */
void Log::setThresholdFor(const string& name, int threshold) {
    string fullname(getName());
    if (fullname.length() > 0) fullname += _sep;
    fullname += name;
    _thresholds->setThresholdFor(fullname, threshold);
}

int Log::getThresholdFor(const string& name) const {
    string fullname(getName());
    if (_name.length() > 0) fullname += _sep;
    return _thresholds->getThresholdFor(fullname+name);
}

/*
 * create a child Log.  A child Log is a Log that is attached to a sub
 * channel of this log.  Its full name will be formed from this parent's 
 * name followed by a dot-delimiter (".") followed by the childName 
 * parameter (e.g. "parentName.childName"), unless the parent is the 
 * root Log (whose name is an empty string), in which case the full child
 * name will just be the value of childName.  
 *
 * This method will create a new Log object and return it as a pointer.  
 * The caller is responsible for deleting the object.
 * 
 * If the threshold for the child Log is not provided, it is set to the 
 * threshold of this Log, its parent.  
 *
 * If the threshold for the child Log is not provided, it is set to the 
 * threshold the channel had the last it was accessed.  If this is the 
 * first time the channel will be used it will be set to the current 
 * threshold of its parent.  
 * 
 * @param childName   the name of the new child log relative to this one.
 *                    The full name of the child log will be the parent's 
 *                    name followed by a "." (if the length of the parent's 
 *                    name is non-zero), followed by the given child name. 
 *                    This cannot be an empty string.  
 * @param threshold   the importance threshold to set for this Log.  If not 
 *                    provided, it will default to the threshold of this 
 *                    log, its parent.  
 */
Log *Log::createChildLog(const string& childName, int threshold) const {
    return new Log(*this, childName, threshold);
}

/*
 * send a message to the log
 * @param importance    how loud the message should be
 * @param message      a simple bit of text to send in the message
 * @param properties   a list of properties to include in the message.
 */
void Log::log(int importance, const string& message, 
              const PropertySet& properties) 
{
    int threshold = getThreshold();
    if (importance < threshold)
        return;
    LogRecord rec(threshold, importance, *_preamble, willShowAll());
    rec.addComment(message);
    rec.addProperties(properties);
    send(rec);
}

/*
 * send a simple message to the log
 * @param importance    how loud the message should be
 * @param message      a simple bit of text to send in the message
 */
void Log::log(int importance, const string& message) {
    int threshold = getThreshold();
    if (importance < threshold)
        return;
    LogRecord rec(threshold, importance, *_preamble, willShowAll());
    rec.addComment(message);
    send(rec);
}

/*
 * send a simple, formatted message.  Use of this function tends to 
 * perform better than log(int, boost::format) as the formatting is 
 * only done if the message will actually get recorded.
 * @param importance    how loud the message should be
 * @param fmt          a printf-style format string
 * @param ...          the inputs to the formatting.
 */
void Log::format(int importance, const char *fmt, ...) {
    int threshold = getThreshold();
    if (importance < threshold) return;
    va_list ap;
    va_start(ap, fmt);
    const int len = vsnprintf(NULL, 0, fmt, ap) + 1; // "+ 1" for the '\0'
    va_end(ap);

    char msg[len];
    va_start(ap, fmt);
    (void)vsnprintf(msg, len, fmt, ap);
    va_end(ap);

    log(importance, msg);
}

/*
 * format and send a message using a variable argument list.  This does 
 * not check the Log threshold; it assumes this has already been done.
 */
void Log::_send(int threshold, int importance, const char *fmt, va_list ap) {
    const int len = strlen(fmt) + 100;  // guess the length
    char message[len];
    vsnprintf(message, len, fmt, ap);

    LogRecord rec(threshold, importance, *_preamble, willShowAll());
    rec.addComment(message);
    send(rec);
}


/*
 * send a fully formed LogRecord to the log destinations
 */
void Log::send(const LogRecord& record) {
    if (record.getImportance() < getThreshold()) 
        return;
    list<shared_ptr<LogDestination> >::iterator i;
    for(i = _destinations.begin(); i != _destinations.end(); i++) {
        (*i)->write(record);
    }
}

/*
 * add a destination to this log.  The destination stream will included
 * in all child Logs created from this log after a call to this function.
 * All previously created logs, including ancestor logs, will be 
 * unaffected.  The IndentedFormatter format will be used with this new 
 * destination.  
 * @param destination   the stream to send messages to
 * @param threshold     the importance threshold to use to filter messages
 *                         sent to the stream.
 */
void Log::addDestination(ostream& destination, int threshold) {
    shared_ptr<LogFormatter> frmtr(new IndentedFormatter());
    addDestination(destination, threshold, frmtr);
}

/*
 * add a destination to this log.  The destination stream will included
 * in all child Logs created from this log after a call to this function.
 * All previously created logs, including ancestor logs, will be 
 * unaffected.  
 * @param destination   a pointer to the stream to send messages to.  The
 *                         caller is responsible for ensuring that the 
 *                         stream is neither closed nor its memory freed
 *                         for the life of this Log.  
 * @param threshold     the importance threshold to use to filter messages
 *                         sent to the stream.
 * @param formatter     the log formatter to use.
 */
void Log::addDestination(ostream& destination, int threshold, 
                         const shared_ptr<LogFormatter> &formatter) 
{
    shared_ptr<LogDestination> dest( 
                new LogDestination(&destination, formatter, threshold));
    addDestination(dest);
}

Log& Log::getDefaultLog() {
    if (defaultLog == 0) {
        Log::setDefaultLog(new ScreenLog());
    }
    
    return *defaultLog;
}

void Log::setDefaultLog(Log *deflog) {
    if (defaultLog != 0) delete defaultLog;
    defaultLog = deflog;
    if (defaultLog != 0) {
        defaultLog->_preamble->markPersistent();
    }
}

void Log::createDefaultLog(const list<shared_ptr<LogDestination> >& dests, 
                           const PropertySet& preamble,
                           const string& name, const int threshold)
{
    Log::setDefaultLog(new Log(dests, preamble, name, threshold));
}

void Log::closeDefaultLog()  {  setDefaultLog(0);  }

Log *Log::defaultLog = 0;
const string Log::_sep(".");

LogRec::~LogRec() {
    if (! _sent) *this << endr;
}

// this used to be inlined but it was causing problems in SWIG/Python-land.
// I'm not sure that the problem has really been solved by un-inlining it 
// or just hidden it away.  
LogRec& LogRec::operator<<(const string& comment) {
    addComment(comment);
    return *this;
}

// @endcond

}}} // end lsst::pex::logging

