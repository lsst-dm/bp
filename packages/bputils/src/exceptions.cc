#include "lsst/bputils/exceptions.h"

namespace bp = boost::python;

namespace lsst { namespace bputils {

namespace {

bp::object makeNewException(char const * name, bp::object const & base) {
    bp::object type(bp::handle<>(bp::borrowed(&PyType_Type)));
    return type(name, bp::make_tuple(base), bp::dict());
}

} // anonymous

namespace detail {

void addExceptionProxy(char const * name, bp::object & wrapper) {
    bp::object bases = wrapper.attr("__bases__");
    if (bp::len(bases) != 1) {
        PyErr_SetString(
            PyExc_SystemError, 
            "Cannot register LSST exception: exception must have exactly one base class."
        );
        bp::throw_error_already_set();
    }
    bp::object baseProxy;
    try {
        baseProxy = bases[0].attr("__exception_proxy__");
    } catch (bp::error_already_set) {
        PyErr_Clear();
        baseProxy = bp::object(bp::handle<>(bp::borrowed(PyExc_Exception)));
    }
    bp::object derivedProxy = makeNewException(name, baseProxy);
    wrapper.attr("__exception_proxy__") = derivedProxy;
    bp::scope().attr(name) = derivedProxy;
}

} // namespace detail

}} // lsst::bputils

