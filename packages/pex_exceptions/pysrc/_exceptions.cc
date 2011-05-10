// -*- lsst-c++ -*-
#include "lsst/bputils.h"
#include "lsst/pex/exceptions.h"

namespace bp = boost::python;

namespace lsst { namespace pex { namespace exceptions {

namespace {

void translateException(Exception const & err) {
    try {
        boost::shared_ptr<Exception> errPtr(err.clone());
        bp::object errWrapped(errPtr);
        bp::object proxyType = errWrapped.attr("__exception_proxy__");
        bp::object proxy = proxyType(errWrapped);
        Py_INCREF(proxyType.ptr());
        Py_INCREF(proxy.ptr());
        PyErr_SetObject(proxyType.ptr(), proxy.ptr());
    } catch (bp::error_already_set & exc) {
        throw exc;
    }
}

void declareBaseException() {
    bp::class_<Tracepoint>("Tracepoint", bp::no_init)
        .def_readonly("_file", &Tracepoint::_file)
        .def_readonly("_line", &Tracepoint::_line)
        .def_readonly("_func", &Tracepoint::_func)
        .def_readonly("_msg", &Tracepoint::_msg)
        ;
    bp::class_<Exception> wrapper("LsstCppException", bp::no_init);
    wrapper
        .def(str(bp::self))
        .def("what", &Exception::what)
        .def("getType", &Exception::getType, "Return the full C++ type of the exception.")
        .def("clone", &Exception::clone,
             bp::return_value_policy<bp::manage_new_object>(), 
             "Deep-copy the exception.")
        .def("getTraceback", &Exception::getTraceback, bp::return_value_policy<bp::copy_to_tuple>())
        ;
    bputils::detail::addExceptionProxy("Exception", wrapper);
    bputils::detail::ExceptionConverter<Exception>();
    bp::register_ptr_to_python< boost::shared_ptr<Exception> >();
    bp::register_exception_translator<Exception>(&translateException);
}

void declareRuntimeExceptions() {
    bputils::declareException<LogicErrorException,Exception>("LogicErrorException");
    bputils::declareException<DomainErrorException,LogicErrorException>("DomainErrorException");
    bputils::declareException<InvalidParameterException,LogicErrorException>("InvalidParameterException");
    bputils::declareException<LengthErrorException,LogicErrorException>("LengthErrorException");
    bputils::declareException<OutOfRangeException,LogicErrorException>("OutOfRangeException");
    bputils::declareException<RuntimeErrorException,Exception>("RuntimeErrorException");
    bputils::declareException<RangeErrorException,RuntimeErrorException>("RangeErrorException");
    bputils::declareException<OverflowErrorException,RuntimeErrorException>("OverflowErrorException");
    bputils::declareException<UnderflowErrorException,RuntimeErrorException>("UnderflowErrorException");
    bputils::declareException<MemoryException,RuntimeErrorException>("MemoryException");
    bputils::declareException<IoErrorException,RuntimeErrorException>("IoErrorException");
    bputils::declareException<TimeoutException,RuntimeErrorException>("TimeoutException");
    bputils::declareException<NotFoundException,Exception>("NotFoundException");
}

} // anonymous

}}} // namespace lsst::pex::exceptions

BOOST_PYTHON_MODULE(_exceptions) {
    lsst::bputils::registerConversions();
    lsst::pex::exceptions::declareBaseException();
    lsst::pex::exceptions::declareRuntimeExceptions();
}
