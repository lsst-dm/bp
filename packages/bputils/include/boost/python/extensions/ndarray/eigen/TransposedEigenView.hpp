#ifndef BOOST_PYTHON_EXTENSIONS_NDARRAY_TRANSPOSEDEIGENVIEW_HPP_INCLUDED
#define BOOST_PYTHON_EXTENSIONS_NDARRAY_TRANSPOSEDEIGENVIEW_HPP_INCLUDED

#include "boost/python/extensions/ndarray/Array.hpp"
#include "lsst/ndarray/eigen.h"

namespace boost { namespace python {

template <typename T, int N, int C>
struct to_python_value< lsst::ndarray::TransposedEigenView<T,N,C> const & > : public detail::builtin_to_python {
    inline PyObject * operator()(lsst::ndarray::TransposedEigenView<T,N,C> const & x) const {
        to_python_value< lsst::ndarray::Array<T,N,C> const &> array_to_python;
        try {
            extensions::numpy::ndarray array(python::detail::new_reference(array_to_python(x.getArray())));
            extensions::numpy::matrix matrix(array, array.get_dtype(), false);
            matrix = matrix.transpose();
            Py_INCREF(matrix.ptr());
            return matrix.ptr();
        } catch (error_already_set & err) {
            handle_exception();
            return NULL;
        }
    }
    inline PyTypeObject const * get_pytype() const {
        return converter::object_manager_traits<extensions::numpy::matrix>::get_pytype();
    }
};

template <typename T, int N, int C>
struct to_python_value< lsst::ndarray::TransposedEigenView<T,N,C> & > : public detail::builtin_to_python {
    inline PyObject * operator()(lsst::ndarray::TransposedEigenView<T,N,C> & x) const {
        return to_python_value< lsst::ndarray::TransposedEigenView<T,N,C> const & >()(x);
    }
    inline PyTypeObject const * get_pytype() const {
        return converter::object_manager_traits<extensions::numpy::matrix>::get_pytype();
    }
};

namespace converter {

template <typename T, int N, int C>
struct arg_to_python< lsst::ndarray::TransposedEigenView<T,N,C> > : public handle<> {
    inline arg_to_python(lsst::ndarray::TransposedEigenView<T,N,C> const & v) :
        handle<>(to_python_value<lsst::ndarray::TransposedEigenView<T,N,C> const &>()()) {}
};

}}} // namespace boost::python::converter

#endif // !BOOST_PYTHON_EXTENSIONS_NDARRAY_TRANSPOSEDEIGENVIEW_HPP_INCLUDED
