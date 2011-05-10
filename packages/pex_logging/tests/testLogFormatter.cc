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
 * @brief  This will test some LogRecord basics and LogFormatter.
 */

#include "lsst/pex/logging/LogFormatter.h"
#include "lsst/pex/logging/LogRecord.h"
#include <iostream>
#include <sstream>
#include <boost/regex.hpp>

using lsst::pex::logging::LogRecord;
using lsst::pex::logging::LogFormatter;
using lsst::pex::logging::BriefFormatter;
using lsst::pex::logging::NetLoggerFormatter;
using lsst::daf::base::PropertySet;
using namespace std;

#define Assert(b, m) tattle(b, m, __LINE__)

void tattle(bool mustBeTrue, const string& failureMsg, int line) {
    if (! mustBeTrue) {
        ostringstream msg;
        msg << __FILE__ << ':' << line << ":\n" << failureMsg << ends;
        throw runtime_error(msg.str());
    }
}

using boost::regex;
using boost::regex_search;

int main() {

    auto_ptr<ostringstream> cap;
    string msg;
    regex datestr("DATE: \\d{4}\\-\\d{2}\\-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d+");

    PropertySet preamble;
    preamble.add("HOST", string("localhost.localdomain"));
    preamble.add("IP", string("111.111.111.111"));
    preamble.add("LOG", string("tester"));

    LogRecord lr1(1, 5, preamble);
    LogRecord lr2(5, 1, preamble);
    lr1.setDate();
    lr2.setDate();

    lr1.addComment("This is a test");
    lr2.addComment("This is a test");

    cout << "data count: " << lr1.countParamNames() << endl;

    Assert(lr1.countParamNames() == 7, "Missing comment or preamble property");
    Assert(lr2.countParamNames() == 0, "Comment or preamble property not ignored");

    LogFormatter *brief = new BriefFormatter();
    LogFormatter *notsobrief = new BriefFormatter(true);
    LogFormatter *nl = new NetLoggerFormatter();

    cap.reset(new ostringstream());
    brief->write(cap.get(), lr1);
    msg = cap->str();
    cout << "[" << msg << "]" << endl;
    Assert(msg == "tester: This is a test\n", "Brief formatting failed");
    cout << "-------------" << endl;

    cap.reset(new ostringstream());
    notsobrief->write(cap.get(), lr1);
    msg = cap->str();
    cout << msg;
    Assert(msg.find("tester: This is a test\n") == 0, 
           "Not-so-brief formatting miswrote comment");
    Assert(msg.find("  HOST: localhost.localdomain\n") != string::npos,
           "Not-so-brief formatting miswrote HOST");
    Assert(msg.find("  IP: 111.111.111.111\n") != string::npos,
           "Not-so-brief formatting miswrote IP");
    Assert(regex_search(msg, datestr), "Not-so-brief formatting miswrote Date");
    cout << "-------------" << endl;

    cap.reset(new ostringstream());
    nl->write(cap.get(), lr1);
    msg = cap->str();
    cout << msg;
    Assert(msg.find("s HOST: localhost.localdomain\n") != string::npos,
           "Netlogger formatting miswrote HOST");
    Assert(msg.find("s IP: 111.111.111.111\n") != string::npos,
           "Netlogger formatting miswrote IP");
    Assert(msg.find("s LOG: tester\n") != string::npos,
           "Netlogger formatting miswrote LOG");
    Assert(msg.find("s COMMENT: This is a test\n") != string::npos,
           "Netlogger formatting miswrote COMMENT");
    Assert(msg.find("t DATE: ") != string::npos,
           "Netlogger formatting miswrote DATE");
    Assert(regex_search(msg, datestr), "Netlogger formatting miswrote Date");
    cout << "=============" << endl;

    cout << "The next logging calls should just produce lines:" << endl;
    cap.reset(new ostringstream());
    brief->write(&cout, lr2);
    msg = cap->str();
    cout << msg;
    Assert(msg.size() == 0, "Brief silent operation failed");
    cout << "-------------" << endl;

    cap.reset(new ostringstream());
    notsobrief->write(&cout, lr2);
    msg = cap->str();
    cout << msg;
    Assert(msg.size() == 0, "Brief silent operation failed");
    cout << "-------------" << endl;

    cap.reset(new ostringstream());
    nl->write(&cout, lr2);
    msg = cap->str();
    cout << msg;
    Assert(msg.size() == 0, "Brief silent operation failed");
    cout << "=============" << endl;

    LogRecord lr3(1, 5);
    lr3.addComment("another test");
    brief->write(&cout, lr3); 

    LogRecord lr4(1, 5);
    lr4.addProperty("LOG", string("tester"));
    brief->write(&cout, lr4); 

    delete notsobrief;
    delete brief;
    delete nl;

}
