#define BOOST_PYTHON_EXTENSIONS_NUMPY_INTERNAL
#include <boost/python/extensions/numpy/internal.hpp>

namespace boost { namespace python {
namespace converter {
NUMPY_OBJECT_MANAGER_TRAITS_IMPL(PyVoidArrType_Type, python::extensions::numpy::void_)
} // namespace boost::python::converter

namespace extensions { namespace numpy {

void_::void_(Py_ssize_t size) : 
    object(
        python::detail::new_reference(
            PyObject_CallFunction((PyObject*)&PyVoidArrType_Type, const_cast<char*>("i"), size)
        )
    )
{}

void_ void_::view(dtype const & dt) const {
    return void_(
        python::detail::new_reference(
            PyObject_CallMethod(this->ptr(), const_cast<char*>("view"), const_cast<char*>("O"), dt.ptr())
        )
    );
}

void_ void_::copy() const {
    return void_(
        python::detail::new_reference(
            PyObject_CallMethod(this->ptr(), const_cast<char*>("copy"), const_cast<char*>(""))
        )
    );
}

}}}} // namespace boost::python::extensions::numpy
