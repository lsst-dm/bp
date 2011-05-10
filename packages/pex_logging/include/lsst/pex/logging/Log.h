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
 * @file Log.h
 * @brief definition of the Log and LogRec classes
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOG_H
#define LSST_PEX_LOG_H

#include "lsst/daf/base/PropertySet.h"
#include "lsst/pex/logging/LogRecord.h"
#include "lsst/pex/logging/LogDestination.h"
#include "lsst/pex/logging/threshold/Memory.h"

#include <vector>
#include <list>
#include <cstdarg>
#include <boost/shared_ptr.hpp>

namespace lsst {
namespace pex {
namespace logging {

namespace dafBase = lsst::daf::base;

/**
 * @brief a place to record messages and descriptions of the state of 
 * processing. 
 *
 * This class is the centerpiece of the logging framework.  It allows modules
 * to record plain text statements about what is happening (including errors,
 * warnings, status information, and debugging messages) as well as typed
 * data delivered as named properties.  It allows users to conveniently
 * tag their messages with a hierarchical label that can indicate where in 
 * the application the message originates.  Messages sent to a Log can be 
 * routed to multiple destinations, such as to the terminal screen, to 
 * a file and to the Event system, all simultaneously.  Messages are also 
 * tagged with an importance or "loudness" level, and Logs control their 
 * verbosity via importance thresholds.  If a message's importance level is 
 * less than the Log's threshold, it will not be recorded.  This allows 
 * applications and modules to selectively turn on and off, say, debugging 
 * messages of which there may be a great number.  Thresholds are enforced 
 * in a way to minimize the impact of unrecorded messages on overall 
 * performance, so debugging messages can remain permanently compiled into 
 * the code.  Finally, the different streams that receive log messages can 
 * have their own thresholds set as well; this allows one to, for example, 
 * send more messages to a file than to the screen.
 * 
 * Logs used by an application are organized into a hierarchy.  Applications 
 * normally get the root Log via the static method Log::getDefaultLog().  This
 * log by default is configured to print messages to the screen (although
 * a production application will typically replace the default Log with one 
 * that sends messages elsewhere).  Alternatively, an application can configure
 * its own root Log via the Log constructors, or with the convenience 
 * subclasses, ScreenLog and DualLog (used to send messages to both the screen
 * and a file).  It is possible to have multiple, independently-configured 
 * Log hierarchies, but it is intended that one is sufficient for an entire 
 * application.  
 * 
 * A root Log has an empty string name associated with it.  A module will 
 * usually create a "child" or "descendent" Log object to send its messages 
 * to, giving it a period-delimited name and a importance threshold:
 * @code
 *     Log mylog(Log::getDefaultLog(), "myapp.mymod", Log::INFO);
 * @endcode
 * or 
 * @code
 *     scoped_ptr<Log> 
 *           mylog(getDefaultLog().createChildLog("myapp.mymod", Log::INFO));
 * @endcode
 * The new log will have the name "myapp.mymod" and allows messages at least 
 * as "loud" as Log::INFO.  The defined levels, FATAL, WARN, INFO, DEBUG (set 
 * to 20, 10, 0, -10, respectively) are conventional notions to encourage 
 * uniformity of levels across modules.  The sub-names between the periods
 * indicate the levels of the Logging hierarchy.  Note that once a threshold
 * is set for a log at a particular level, it is remembered even after that
 * log is deleted; when anther log is created with that name, its previous
 * threshold is preserved.  If no threshold is provided, it will default to 
 * that of its parents log.  Furthermore, setting the threshold for a parent
 * automatically updates the threshold for all of its descendents that have 
 * not otherwise set the threshold.  For example,
 * @code
 *   Log root = Log::getDefaultLog();   // the root log; name is an empty string
 *   Log applog(root, "myapp");         // myapp: a child log
 *   Log *modlog = applog.createChildLog("mymod", Log::WARN);
 *                                      // myapp.mymod: a quieter grandchild log
 *   delete modlog;
 *   modlog = applog.createChildLog("mymod")  // still set to Log::WARN
 *   
 *   root.setThreshold(log::DEBUG);     // root and applog now set to Log::DEBUG
 *   modlog.resetThreshold();           // modlog now also set to Log::DEBUG
 * @endcode
 * As you can see in the example, names are set relative to their parent.  
 * That is, when we use a log called "myapp" to create a child called 
 * "mymod", the full name is "myapp.mymod".  This allows a Log to be made 
 * a member of a class and given a name without knowing the context in which 
 * the class is being used.  Note that it is not necessary to create
 * logs at intervening levels; the first example above, shows us directly 
 * creating a "grandchild" from the root Log.  
 *
 * Simple text log messages can be recorded with the log() functions:  
 * @code
 *     mylog.log(Log::WARN, "Skipping initialization");
 *     mylog.log(Log::WARN, boost::format("Found only %d objects") % nobj);
 * @endcode
 * For more complex messages that might include named properties, it is usually
 * more convenient to use the streaming method:
 * @code
 *     using lsst::pex::logging::Rec;
 *     using lsst::pex::logging::Prop;
 *     Rec(mylog, Log::DEBUG) << "Completed deconvolution"
 *                            << Prop("iterations", 541)
 *                            << Prop("rms", 0.0032)
 *                            << Rec::endr;   
 *
 *     Rec(mylog, Log::INFO) << boost::format("applying %s kernel") % kern
 *                           << Prop("width", width);
 *                           << Rec::endr;
 * @endcode
 * The manipulator Rec::endr triggers the recording of the message to the log. 
 * Note that Rec is a shorthand typedef for the LogRec class.  
 */
class Log {
public:

    /**
     * the conventional importance level for messages that aid in debugging.  
     * This value is set to -10 with the intention that messages with 
     * negative importance levels (or more precisely, >= -10) will be printed 
     * when the threshold is set to this value.  
     */
    static const int DEBUG;

    /**
     * the conventional importance level for messages that are informational
     * and which report on normal behavior.  The value is set to 0 with the 
     * intention that this is the default threshold for logs and their 
     * destinations.  
     */
    static const int INFO;

    /**
     * the conventional importance level for messages that warn about 
     * abnormal but non-fatal behavior.  The value is set to 10.
     */
    static const int WARN;

    /**
     * a magic threshold value that indicates that a threshold of a Log
     * should be set to its nearest ancestor
     */
    static const int INHERIT_THRESHOLD;

    /**
     * the conventional importance level for messages that report on fatal
     * behavior.  The value is set to 20.  Note that the logging module 
     * makes no attempt to shutdown execution or other wise affect control 
     * flow when a message having a importance exceeding this level.  
     */
    static const int FATAL;

    /**
     * create a null log.  This constructor should 
     * not normally be employed to obtain a Log; the static getDefaultLog() 
     * method should be used instead.  This is provided primarily for 
     * subclasses and containers that require a no-arg constructor.  
     * @param threshold   the initial importance threshold for this log
     * @param name        the initial name for this log.  An empty string 
     *                    (the default) denotes a root log.
     */
    Log(const int threshold=INFO, const std::string& name="");

    /**
     * create a fully configured Log.  This constructor is 
     * not normally employed to obtain a Log; the static getDefaultLog() 
     * method or the createChildLog() method of should be used instead.  
     * @param destinations   the list of LogDestinations to attach to this 
     *                         Log.
     * @param preamble       a data properties that should be included 
     *                         with every recorded message to the Log.  This
     *                         constructor will automatically add the properties
     *                         "LOG", giving the Log name and "DATE", giving
     *                         the timestamp the message was recorded.
     * @param name           the name to give this log.  By default, the 
     *                         name will be an empty string, signifying a 
     *                         root log.  
     * @param threshold      the importance threshold to assign to this Log.  
     *                         Messages sent to this log must have a importance
     *                         level equal to or greater than this value to 
     *                         be recorded.  (Note that thresholds associated
     *                         with destinations have their own thresholds that
     *                         will override this one.)
     * @param defaultShowAll if true, log message properties will be shown
     *                         by default when the messages are displayed.  
     *                         The default is false, indicating that their 
     *                         display must be turned on as needed.  
     */
    Log(const std::list<boost::shared_ptr<LogDestination> >& destinations, 
        const dafBase::PropertySet& preamble,
        const std::string& name="", const int threshold=INFO,
        bool defaultShowAll=false);

    /**
     * create a child of a given Log.  The child log will be attached 
     * to the same streams as the provided parent Log.  The full child 
     * name will be constructed from the given name, prepended by the 
     * parent's name and a ".".  That is, if the parent is named "coo.coo"
     * and the name given here is "ca-choo", then the full name will be 
     * "coo.coo.ca.choo".  If the parent Log is a root Log, then the "." 
     * is not prepended.
     *
     * @param parent        the Log that will serve as its parent.  
     * @param childName     the name of the child log, relative to the given 
     *                          parent.  
     * @param threshold     the threshold for the new child Log.  If not 
     *                          provided, the value will be set to the value
     *                          it had the last time it was created, or if it
     *                          is being created for the first time, it is
     *                          set to track the threshold of the parent. 
     */
    Log(const Log& parent, const std::string& childName, 
        int threshold=INHERIT_THRESHOLD);

    /**
     * create a copy
     */
    Log(const Log& that);

    /** 
     * delete this Log
     */
    virtual ~Log();

    /**
     * copy another log to this one.  
     */
    Log& operator=(const Log& that);

    /**
     * return the name associated with this Log.  The root logger 
     * will return an empty stream as its name.
     */
    const std::string& getName() const { return _name; }

    /**
     * return the importance threshold for this log.  A message sent to this 
     * Log will not be recorded if the message importance is less than the 
     * threshold. 
     */
    int getThreshold() const { 
        return ((_threshold > INHERIT_THRESHOLD || _name.length() == 0) 
                       ? _threshold
                       : _thresholds->getThresholdFor(_name) );
    }

    /**
     * set the importance threshold for this log.  A message sent to this 
     * Log will not be recorded if the message importance is less than the 
     * threshold. 
     * @param threshold    the new threshold value
     */
    void setThreshold(int threshold) { 
        _threshold = threshold;
        _thresholds->setThresholdFor(_name, threshold);
    } 

    /**
     * return true if the threshold is low enough to pass messages of a 
     * given loudness or importance.
     */
    bool sends(int importance) const { return (importance >= getThreshold()); }

    /**
     * reset the importance threshold of this log to that of its parent 
     * threshold.  If this is a root Log, the threshold will be set to INFO.
     */
    void resetThreshold() { setThreshold(INHERIT_THRESHOLD); }

    /**
     * set the importance threshold for a child Log.  When a child Log of the
     * same name is created, it will be assigned this threshold.  Any existing
     * Log object with name that has already explicitly set its importance
     * threshold will not be affected; however, those that are set to inherit
     * the threshold will be. 
     * @param name       the relative name of the child log
     * @param threshold  the importance threshold to set Logs with this name to.
     */
    void setThresholdFor(const std::string& name, int threshold);

    /**
     * get the importance threshold for a child Log.  When a child Log of the
     * same name is created, this is the threshold it will have.  
     * @param name       the relative name of the child log
     */
    int getThresholdFor(const std::string& name) const;

    /**
     * return true if this log will prefer showing all properties when
     * rendering log records.  This preference will be passed to 
     * LogFormatters via all LogRecords created by or sent via this Log.
     * A LogFormatter may or may not choose to honor this preference when 
     * the LogRecord is rendered.
     */
    bool willShowAll() const { 
        return 
            (_myShowAll.get()) ? *_myShowAll 
                               : ((_defShowAll.get()) ? *_defShowAll : false);
    } 

    /**
     * set whether all of the properties should be displayed when 
     * rendering log records.  This preference will be passed to 
     * LogFormatters via all LogRecords created by or sent via this Log.
     * A LogFormatter may or may not choose to honor the preference, 
     * according to the purposes of its implmentation.  
     *
     * Note that while this attribute's default value is inherited from 
     * the root log, it is not persistently associated with the log's 
     * name like the importance threshold.  If this log is destroyed and 
     * then recreated again with the same name, this attribute will revert 
     * to that of the root log.  
     * @param yesno    the preference for showing all.  willShowAll() will 
     *                    return this value.  
     */
    void setShowAll(bool yesno) {  
        if (_myShowAll.get()) 
            *_myShowAll = yesno;
        else
            _myShowAll.reset(new bool(yesno));
    }

    /**
     * reset whether to all properties are display to what ever the root
     * log is set to do.
     */
    void resetShowAll() {  
        if (_defShowAll.get() == _myShowAll.get()) 
            *_myShowAll = false;
        else
            _myShowAll.reset();  
    }

    /**
     * add a property to the preamble
     */
    template <class T>
    void addPreambleProperty(const std::string& name, const T& val);

    /**
     * set a property to the preamble, overwriting any value with 
     * the same name.
     */
    template <class T>
    void setPreambleProperty(const std::string& name, const T& val);

    /**
     * create a child of a given Log.  The child log will be attached 
     * to the same streams as the provided parent Log.  The full child 
     * name will be constructed from the given name, prepended by the 
     * parent's name and a ".".  That is, if the parent is named "coo.coo"
     * and the name given here is "ca-choo", then the full name will be 
     * "coo.coo.ca.choo".  If the parent Log is a root Log, then the "." 
     * is not prepended.
     *
     * @param childName     the name of the child log, relative to the given 
     *                          parent.  
     * @param threshold     the threshold for the new child Log.  If not 
     *                          provided, the value will be set to the value
     *                          it had the last time it was created, or if it
     *                          is being created for the first time, it is
     *                          set to track the threshold of the parent. 
     */
    Log *createChildLog(const std::string& childName, 
                        int threshold=INHERIT_THRESHOLD) const;

    /**
     * send a message to the log
     * @param importance    how loud the message should be
     * @param message      a simple bit of text to send in the message
     * @param properties   a list of properties to include in the message.
     *
     * @bpdox{label:properties}
     */
    void log(int importance, const std::string& message, 
             const dafBase::PropertySet& properties);

    /**
     * send a message to the log
     * @param importance    how loud the message should be
     * @param message      a simple bit of text to send in the message
     * @param name         the name of a property to include in the message.
     * @param val          the value of the property to include
     *
     * @bpdox{label:template}
     */
    template <class T>
    void log(int importance, const std::string& message, 
             const std::string& name, const T& val);

    /**
     * send a message to the log
     * @param importance    how loud the message should be
     * @param message      a simple bit of text to send in the message
     * @param prop         a property to include in the message.
     *
     * @bpdox{ignore}
     */
    template <class T>
    void log(int importance, const std::string& message, 
             const RecordProperty<T>& prop);

    /**
     * send a simple message to the log
     * @param importance    how loud the message should be
     * @param message      a simple bit of text to send in the message
     *
     * @bpdox{label:simple}
     */
    void log(int importance, const std::string& message);

    /**
     * send a simple, formatted message to the log
     * @param importance    how loud the message should be
     * @param message      a simple message in the form of a boost::format
     *                       instance
     *
     * @bpdox{ignore}
     */
    void log(int importance, const boost::format& message) {
        log(importance, message.str());
    }

    /**
     * send a simple, formatted message.  Use of this function tends to 
     * perform better than log(int, boost::format) as the formatting is 
     * only done if the message will actually get recorded.
     * @param importance    how loud the message should be
     * @param fmt          a printf-style format string
     * @param ...          the inputs to the formatting.
     */
    void format(int importance, const char *fmt, ...);

    /**
     * send a fully formed LogRecord to the log destinations
     */
    void send(const LogRecord& record);

    /**
     * add a destination to this log.  The destination stream will included
     * in all child Logs created from this log after a call to this function.
     * All previously created logs, including ancestor logs, will be 
     * unaffected.  The IndentedFormatter format will be used with this new 
     * destination.  
     * @param destination   the stream to send messages to
     * @param threshold     the importance threshold to use to filter messages
     *                         sent to the stream.
     * @bpdox{ignore}
     */
    void addDestination(std::ostream& destination, int threshold);

    /**
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
     *
     * @bpdox{ignore}
     */
    void addDestination(std::ostream &destination, int threshold, 
                        const boost::shared_ptr<LogFormatter> &formatter);

    /**
     * add a destination to this log.  The destination stream will included
     * in all child Logs created from this log after a call to this function.
     * All previously created logs, including ancestor logs, will be 
     * unaffected.
     */
    void addDestination(const boost::shared_ptr<LogDestination> &destination) {
        _destinations.push_back(destination);
    }

    /** 
     * return the current set of preamble properties
     */
    const dafBase::PropertySet& getPreamble() { return *_preamble; }

    /**
     * obtain the default root Log instance.
     */
    static Log& getDefaultLog();

    /**
     * create a new log and set it as the default Log
     * @param destinations   the list of LogDestinations to attach to this 
     *                         Log.
     * @param preamble       a list of data properties that should be included 
     *                         with every recorded message to the Log.  This
     *                         constructor will automatically add the properties
     *                         "LOG", giving the Log name and "DATE", giving
     *                         the timestamp the message was recorded.
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
    static void createDefaultLog(
        const std::list<boost::shared_ptr<LogDestination> >& destinations, 
        const dafBase::PropertySet& preamble,
        const std::string& name="", const int threshold=INFO);

    /**
     * shutdown and destroy the default log
     */
    static void closeDefaultLog();

    /**
     * print the entire tree of thresholds
     */
    void printThresholds(std::ostream& out) {
        _thresholds->printThresholds(out);
    }

    /**
     * reset all thresholds to the default set at the construction of the 
     * root log.  In general, use of this function is not recommended as 
     * it will affect all other users of Logs.  
     */
    void reset() { _thresholds->forgetAllNames(); }

protected:
    /**
     * the default Log
     */
    static Log *defaultLog;

    /**
     * set the default Log.  The pointer sent to this method should be 
     * a freshly minted pointer, "unowned" by anyone.  That is, the caller 
     * relinguishes responsibility for deleting the log by calling this 
     * function.
     */
    static void setDefaultLog(Log* deflog);

    static const std::string _sep;

    /**
     * format and send a message using a variable argument list.  This does 
     * not check the Log threshold; it assumes this has already been done.
     */
    void _send(int threshold, int importance, const char *fmt, va_list ap);

private:
    void completePreamble();

    int _threshold;
    boost::shared_ptr<bool> _defShowAll;
    boost::shared_ptr<bool> _myShowAll;
    std::string _name;

protected: 
    /**
     * the memory of child importance thresholds.
     */
    boost::shared_ptr<threshold::Memory> _thresholds;

    /**
     * the list of destinations to send messages to
     */
    std::list<boost::shared_ptr<LogDestination> > _destinations;

    /**
     * the list preamble data properties that are included with every 
     * log record.
     */
    dafBase::PropertySet::Ptr _preamble;
};

template <class T>
void Log::addPreambleProperty(const std::string& name, const T& val) {
    _preamble->add<T>(name, val);
}

template <class T>
void Log::setPreambleProperty(const std::string& name, const T& val) {
    _preamble->set<T>(name, val);
}
        
template <class T>
void Log::log(int importance, const std::string& message, 
              const std::string& name, const T& val) {

    int threshold = getThreshold();
    if (importance < threshold)
        return;
    LogRecord rec(threshold, importance, *_preamble, willShowAll());
    rec.addComment(message);
    rec.addProperty(name, val);
    send(rec);
}

template <class T>
void Log::log(int importance, const std::string& message, 
         const RecordProperty<T>& prop) 
{
    log(importance, message, prop.name, prop.value);
}


/**
 * @brief  A LogRecord attached to a particular Log that suppports stream 
 * stream semantics.  
 * 
 */
class LogRec : public LogRecord {
public:

    /**
     * Create a log record to be sent to a given log.  
     * @param log         the log this record should be written to
     * @param importance  the urgency of the record.  If this value is 
     *                     greater than or equal to the Log's importance 
     *                     threshold, the message will be recorded.
     */
    LogRec(Log& log, int importance) 
        : LogRecord(log.getThreshold(), importance, log.getPreamble()), 
          _sent(false), _log(&log)
    { }

    /**
     * create a copy
     */
    LogRec(const LogRec& that) 
        : LogRecord(that), _sent(false), _log(that._log) 
    { }

    /**
     * delete this record
     */
    virtual ~LogRec();

    /**
     * copy another record into this one
     */
    LogRec& operator=(const LogRec& that) {
        LogRecord::operator=(that);
        _log = that._log;
        return *this;
    }

    /**
     * Record manipulators.  These values may be sent to this LogRec via 
     * the output operator to request a particular manipulation of the message.
     */
    enum Manip { 
        /**
         * a manipulator indicating that the message is complete and should 
         * be sent to the Log.
         */
        endr 
    };

    /**
     * record a string comment into this message
     */
    LogRec& operator<<(const std::string& comment); /* {
        addComment(comment);
        return *this;
    }  */

    /**
     * record a string comment into this message
     */
    LogRec& operator<<(const boost::format& comment) {
        addComment(comment);
        return *this;
    }

    /**
     * record a data property into this message
     */
    template <class T>
    LogRec& operator<<(const RecordProperty<T>& prop) {
        addProperty(prop.name, prop.value);
        return *this;
    }

    /**
     * record a data property into this message
     */
    LogRec& operator<<(const dafBase::PropertySet& props) {
        addProperties(props);
        return *this;
    }

    /**
     * receive a message manipulator signal.  The most common use of this 
     * version is to indicate that the Record is now complete and should 
     * be flushed to the Log.
     * @param signal    the manipulator signal value (e.g. LogRec::endr).  
     */
    LogRec& operator<<(Manip signal) {
        if (_send) {
            switch (signal) {
            case endr:
                _log->send(*this);
                _sent = true;
                break;
            default:
                ;
            }
        }
        return *this;
    }

private:
    bool _sent;
    Log *_log;
};

/**
 * a shorthand version of the LogRec class
 */
typedef LogRec Rec;

}}}     // end lsst::pex::logging

#endif  // end LSST_PEX_LOG_H
