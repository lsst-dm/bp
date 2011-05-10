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
 
#include "lsst/pex/logging/LogRecord.h"
#include "lsst/daf/base/DateTime.h"
#include <iostream>

using lsst::pex::logging::LogRecord;
using lsst::daf::base::PropertySet;
using lsst::daf::base::DateTime;
using namespace std;

void assure(bool mustBeTrue, const string& failureMsg) {
    if (! mustBeTrue)
        throw runtime_error(failureMsg);
}

int main() {

    LogRecord lr1(-1, 10);
    cout << "first record is " << ((lr1.willRecord()) ? "loud" : "quiet")
         << " and has " << lr1.countParamNames() << " records." << endl;
    assure(lr1.willRecord(), "verbose record not recording.");

    // when a LogRecord is first created it is initialized with three 
    // properties: LEVEL, TIMESTAMP, and DATE
    assure(lr1.countParamNames()==3, 
           string("wrong initial data count (loud)."));
    cout << "  LEVEL: " << lr1.data().get<int>("LEVEL") << endl;
    cout << "  TIMESTAMP: " << lr1.data().get<DateTime>("TIMESTAMP").nsecs() << endl;
    cout << "  DATE: " << lr1.data().get<string>("DATE") << endl;

    LogRecord lr2(10, 5);
    cout << "second record is " << ((lr2.willRecord()) ? "loud" : "quiet")
         << " and has " << lr2.countParamNames() << " records." << endl;
    assure(! lr2.willRecord(), "quiet record is recording anyway.");
    assure(lr2.countParamNames()==0, string("wrong initial data count (quiet)."));
        
    const char * simple = "a simple comment";
    lr1.addComment(simple);
    lr2.addComment(simple);
    assure(lr1.countParamNames()==4, 
           string("wrong 1st updated data count (loud)."));
    assure(lr2.countParamNames()==0, 
           string("wrong 1st updated data count (quiet)."));
    
    const PropertySet& lis = lr1.getProperties();
    assure(lis.nameCount() > 0, "no properties in LogRecord");
    assure(lis.exists("TIMESTAMP"), "wrong label for TIMESTAMP");
    assure(lis.exists("COMMENT"), "wrong label for COMMENT");

    std::vector<string> comments = lis.getArray<string>("COMMENT");
    cout << "First record has " << comments.size() << " comments" << endl;
    cout << "COMMENT: " << comments[0] << endl;
    assure(comments[0] == simple, "Wrong comment value");
    assure(comments.size() == 1, "First record has wrong number of comments");
    assure(! lr2.data().exists("COMMENT"), "2nd quiet record has comments"); 

    lr1.addProperty("dpint", 2);
    lr2.addProperty("dpint", 2);
    lr1.addProperty("dpfloat", 2.5);
    lr2.addProperty("dpfloat", 2.5);
    lr1.addProperty("dplong", 5l);
    lr2.addProperty("dplong", 5l);

    cout << "The first record now has " << lr1.countParamValues() 
         << " properties" << endl;
    cout << "The second record now has " << lr2.countParamValues() 
         << " properties" << endl;
    assure(lr1.countParamValues()==7, 
           string("wrong 1st updated data count (loud)."));
    assure(lr2.countParamValues()==0, 
           string("wrong 1st updated data count (quiet)."));

    PropertySet preamble;
    preamble.set<int>("dpint", 2);
    preamble.set<float>("dpfloat", 2.5);
    preamble.set<long>("dplong", 5l);

    LogRecord lr3(1, 5, preamble);
    cout << "The third record starts with " << lr3.countParamNames() 
         << " properties."  << endl;
    assure(lr3.countParamNames()==6, "wrong initial count via preamble");

    // note that this is now set automatically during construction
    lr3.setDate();
    cout << "It now has " << lr3.countParamNames() 
         << " properties after adding DATE."  << endl;
    assure(lr3.countParamNames()==6, "wrong initial count via preamble");

    const PropertySet& list = lr3.getProperties();
    cout << "Properties: ";
    std::vector<string> names = list.paramNames();
    for(std::vector<string>::const_iterator i=names.begin(); 
        i != names.end(); i++) 
    {
        cout << *i << " ";
    }
    cout << endl;

    cout << "Third record's properties:" << endl;
    cout << "  LEVEL: " << lr3.data().get<int>("LEVEL") << endl;
    cout << "  TIMESTAMP: " << lr3.data().get<DateTime>("TIMESTAMP").nsecs() << endl;
    cout << "  DATE: " << lr3.data().get<string>("DATE") << endl;
    cout << "  dpint: " << lr3.data().get<int>("dpint") << endl;
    cout << "  dpfloat: " << lr3.data().get<float>("dpfloat") << endl;
    cout << "  dplong: " << lr3.data().get<long>("dplong") << endl;
}
