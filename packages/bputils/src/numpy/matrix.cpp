#define BOOST_PYTHON_EXTENSIONS_NUMPY_INTERNAL
#include <boost/python/extensions/numpy/internal.hpp>
#include <boost/python/extensions/numpy/matrix.hpp>

namespace boost { namespace python { namespace extensions { namespace numpy { namespace detail {
inline object get_matrix_type() {
    object module = import("numpy");
    return module.attr("matrix");
}
}}} // namespace extentions::numpy::detail

namespace converter {

PyTypeObject const * object_manager_traits<extensions::numpy::matrix>::get_pytype() {
    return reinterpret_cast<PyTypeObject*>(extensions::numpy::detail::get_matrix_type().ptr());
}

} // namespace boost::python::converter

namespace extensions { namespace numpy {

object matrix::construct(object const & obj, dtype const & dt, bool copy) {
    return numpy::detail::get_matrix_type()(obj, dt, copy);
}

object matrix::construct(object const & obj, bool copy) {
    return numpy::detail::get_matrix_type()(obj, object(), copy);
}

matrix matrix::view(dtype const & dt) const {
    return matrix(
        python::detail::new_reference(
            PyObject_CallMethod(this->ptr(), const_cast<char*>("view"), const_cast<char*>("O"), dt.ptr())
        )
    );
}

matrix matrix::copy() const {
    return matrix(
        python::detail::new_reference(
            PyObject_CallMethod(this->ptr(), const_cast<char*>("copy"), const_cast<char*>(""))
        )
    );
}

matrix matrix::transpose() const {
    return matrix(extract<matrix>(ndarray::transpose()));
}

}}}} // namespace boost::python::extensions::numpy
