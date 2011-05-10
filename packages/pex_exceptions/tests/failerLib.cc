#include "lsst/bputils/exceptions.h"
#include "Failer.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(failer) {
    
    bp::import("lsst.pex.exceptions");

    lsst::bputils::declareException<failer::MyException,lsst::pex::exceptions::Exception>("MyException");
    
    bp::class_<failer::Failer>("Failer")
        .def("fail", &failer::Failer::fail)
        .def("fail2", &failer::Failer::fail2)
        ;

}
