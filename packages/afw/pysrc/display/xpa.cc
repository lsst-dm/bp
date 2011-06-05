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
 
/*
 * This would be a very simple module, except that I didn't want to
 * deal with (char **) in SWIG; so I wrote C wrappers to pass a single
 * (char *) instead.
 */
#include "boost/python.hpp"
#include "boost/noncopyable.hpp"
extern "C" {
#include "xpa.h"
}
#include "lsst/pex/exceptions/Runtime.h"
#include <cstring>

namespace bp = boost::python;

namespace {

class myXPA : private boost::noncopyable {
public:
    static XPA get(bool reset=false) {
        static myXPA *singleton = NULL;

        if (reset && singleton != NULL) {
            delete singleton;
            singleton = NULL;
        }

        if (singleton == NULL) {
            singleton = new myXPA("w");
        }

        return singleton->_xpa;
    }
private:
    myXPA(char const *mode) {
        _xpa = XPAOpen((char *)mode);

        if (_xpa == NULL) {
            throw LSST_EXCEPT(lsst::pex::exceptions::IoErrorException, "Unable to open XPA");
        }
    }
        
    ~myXPA() {
        XPAClose(_xpa);
    }

    static XPA _xpa;                // the real XPA connection
};

XPA myXPA::_xpa = NULL;


/*
 * A binding for XPAGet that talks to only one server, but doesn't have to talk (char **) with SWIG
 */
const char *
XPAGet1(XPA xpa,
	char *xtemplate,
	char *paramlist,
	char *mode)
{
    char *buf = NULL;			/* desired response */
    int len = 0;			/* length of buf; ignored */
    char *error = NULL;			/* returned error if any*/

    if (xpa == NULL) {
        xpa = myXPA::get();
    }

    int n = XPAGet(xpa, xtemplate, paramlist, mode,
		   &buf, &len, NULL, &error, 1);

    if(n == 0) {
	return(NULL);
    }
    if(error != NULL) {
	return(error);
    }

    return(buf);
}

/*****************************************************************************/

const char *
XPASet1(XPA xpa,
	char *xtemplate,
	char *paramlist,
	char *mode,
	char *buf,			// desired extra data
	int len)			// length of buf (or -1)
{
    if(len < 0) {
	len = strlen(buf);		// length of buf
    }
    char *error = NULL;			// returned error if any

    if (xpa == NULL) {
        xpa = myXPA::get();
    }

    int n = XPASet(xpa, xtemplate, paramlist, mode,
		   buf, len, NULL, &error, 1);

    if(n == 0) {
	return(NULL);
    }
    if(error != NULL) {
	return(error);
    }

    return "";
}


/*****************************************************************************/

const char *
XPASetFd1(XPA xpa,
	  char *xtemplate,
	  char *paramlist,
	  char *mode,
	  int fd)			/* file descriptor for xpa to read */
{
    char *error = NULL;			/* returned error if any*/

    if (xpa == NULL) {
        xpa = myXPA::get();
    }

    int n = XPASetFd(xpa, xtemplate, paramlist, mode,
		     fd, NULL, &error, 1);

    if(n == 0) {
	return(NULL);
    }
    if(error != NULL) {
	return(error);
    }

    return NULL;
}

void reset() {
    myXPA::get(true);
}

} // <anonymous>

BOOST_PYTHON_MODULE(xpa) {
    bp::class_< XPARec, boost::noncopyable >("XPA", bp::no_init);
    bp::def("get", &XPAGet1);
    bp::def("set", &XPASet1);
    bp::def("setFd1", &XPASetFd1);
    bp::def("reset", &reset);
}
