#include "lsst/bputils.h"
#include "boost/python/extensions/filesystem.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace bp = boost::python;
namespace bpx = boost::python::extensions;

namespace {

struct Py_ostream {

    static void write(std::ostream & os, std::string const & s) {
        os.write(s.c_str(), s.size());
    }

    static void declare() {
        bp::class_<std::ostream,boost::noncopyable> wrapper("ostream", bp::no_init);
        wrapper
            .def("write", &write)
            .def("flush", &std::ostream::flush, bpx::return_none<>())
            ;
        bp::register_ptr_to_python< boost::shared_ptr<std::ostream> >();
    }

};

struct Py_ofstream {

    static boost::shared_ptr<std::ofstream> make_ofstream(char const * filename, std::string const & mode) {
        std::ios_base::openmode flags(std::ios_base::out);
        if (mode == "a") {
            flags |= std::ios_base::app;
        } else if (mode == "w") {
            flags |= std::ios_base::trunc;
        } else if (mode == "ab") {
            flags |= std::ios_base::app;
            flags |= std::ios_base::binary;
        } else if (mode == "wb") {
            flags |= std::ios_base::trunc;
            flags |= std::ios_base::binary;
        } else {
            PyErr_SetString(PyExc_ValueError, "ostreame mode must be one of 'a', 'w', 'ab' or 'wb'");
            bp::throw_error_already_set();
        }
        return boost::shared_ptr<std::ofstream>(new std::ofstream(filename, flags));
    }

    static void declare() {
        bp::class_<std::ofstream,bp::bases<std::ostream>,boost::noncopyable> 
            wrapper("ofstream", bp::no_init);
        wrapper
            .def(
                "__init__", 
                bp::make_constructor(
                    &make_ofstream, bp::default_call_policies(), 
                    (bp::arg("name"), bp::arg("mode")="w")
                ),
                "Construct an ofstream from a filename and mode (one of 'w', 'a', 'wb', and 'ab')."
            )
            .def("close", &std::ofstream::close, bpx::return_none<>())
            ;
        bp::register_ptr_to_python< boost::shared_ptr<std::ofstream> >();
    }

};

struct Py_ostringstream {

    static void declare() {
        bp::class_<std::ostringstream,bp::bases<std::ostream>,boost::noncopyable> 
            wrapper("ostringstream", bp::init<>());
        wrapper
            .def("str", (std::string (std::ostringstream::*)() const)&std::ostringstream::str)
            .def("str", (void (std::ostringstream::*)(std::string const &))&std::ostringstream::str)
            ;
        bp::register_ptr_to_python< boost::shared_ptr<std::ostringstream> >();
    }

};

std::ostream & _get_cout() { return std::cout; }

std::ostream & _get_cerr() { return std::cerr; }

} // anonymous

BOOST_PYTHON_MODULE(_bputils) {
    bpx::filesystem_path_to_python();
    bpx::filesystem_path_from_python_str();
    Py_ostream::declare();
    Py_ofstream::declare();
    Py_ostringstream::declare();
    bp::def("_get_cout", &_get_cout, bp::return_value_policy<bp::reference_existing_object>());
    bp::def("_get_cerr", &_get_cerr, bp::return_value_policy<bp::reference_existing_object>());
}
