#ifndef BOOST_PYTHON_EXTENSIONS_NDARRAY_TABLES_LAYOUT_HPP_INCLUDED
#define BOOST_PYTHON_EXTENSIONS_NDARRAY_TABLES_LAYOUT_HPP_INCLUDED

#include <boost/python/extensions/numpy.hpp>
#include <boost/python/extensions/ndarray.hpp>
#include "lsst/ndarray/tables.h"

namespace lsst { namespace ndarray { namespace tables {

struct LayoutToNumpyType {

    template <typename T>
    void operator()(Field<T,0> const & field) const {
        dict[field.name] = boost::python::make_tuple(
            boost::python::extensions::numpy::dtype::get_builtin<T>(),
            field.offset
        );
    }

    template <typename T, int N>
    void operator()(Field<T,N> const & field) const {
        dict[field.name] = boost::python::make_tuple(
            boost::python::make_tuple(
                boost::python::extensions::numpy::dtype::get_builtin<T>(),
                boost::python::object(field.shape)
            ),
            field.offset
        );
    }
    
    mutable boost::python::dict dict;
};

struct NumpyTypeToLayout {

    template <typename T>
    void operator()(Field<T,0> & field) const {
        field.name = boost::python::extract<std::string>(pyNames[n]);
        boost::python::tuple s = boost::python::extract<boost::python::tuple>(pyFields[field.name]);
        field.offset = boost::python::extract<int>(s[1]);
        boost::python::extensions::numpy::dtype pyElement 
            = boost::python::extract<boost::python::extensions::numpy::dtype>(s[0]);
        if (pyElement != boost::python::extensions::numpy::dtype::get_builtin<T>()) {
            PyErr_Format(PyExc_TypeError, "Type mismatch encountered in field %s.", field.name.c_str());
            boost::python::throw_error_already_set();
        }
        ++n;
    }

    template <typename T, int N>
    void operator()(Field<T,N> & field) const {
        field.name = boost::python::extract<std::string>(pyNames[n]);
        boost::python::tuple s1 = boost::python::extract<boost::python::tuple>(pyFields[field.name]);
        boost::python::tuple s2 = boost::python::extract<boost::python::tuple>(s1[0].attr("subdtype"));
        field.offset = boost::python::extract<int>(s1[1]);
        field.shape = boost::python::extract< Vector<int,N> >(s2[1]);
        boost::python::extensions::numpy::dtype pyElement 
            = boost::python::extract<boost::python::extensions::numpy::dtype>(s2[0]);
        if (pyElement != boost::python::extensions::numpy::dtype::get_builtin<T>()) {
            PyErr_Format(PyExc_TypeError, "Type mismatch encountered in field %s.", field.name.c_str());
            boost::python::throw_error_already_set();
        }
        ++n;
    }

    NumpyTypeToLayout(boost::python::extensions::numpy::dtype const & input) :
        n(0), pyNames(input.attr("names")), pyFields(input.attr("fields"))
    {}

    mutable int n;
    boost::python::object pyNames;
    boost::python::object pyFields;
};

template <typename T>
boost::python::extensions::numpy::dtype makeNumpyType(Layout<T> const & layout) {
    LayoutToNumpyType function;
    boost::fusion::for_each(layout.getSequence(), function);
    return boost::python::extensions::numpy::dtype(function.dict, false);
}

template <typename T>
void fillLayout(boost::python::extensions::numpy::dtype const & input, Layout<T> & layout) {
    NumpyTypeToLayout function(input);
    boost::fusion::for_each(layout.getSequence(), function);
}

}}} // namespace lsst::ndarray::tables


namespace boost { namespace python {

template <typename T>
struct to_python_value< lsst::ndarray::tables::Layout<T> const & > : public detail::builtin_to_python {
    inline PyObject * operator()(lsst::ndarray::tables::Layout<T> const & x) const {
        lsst::ndarray::tables::Layout<T> normal_layout(x);
        normal_layout.normalize();
        extensions::numpy::dtype dtype = lsst::ndarray::tables::makeNumpyType(normal_layout);
        Py_INCREF(dtype.ptr());
        return dtype.ptr();
    }
    inline PyTypeObject const * get_pytype() const {
        return converter::object_manager_traits<extensions::numpy::dtype>::get_pytype();
    }
};

template <typename T>
struct to_python_value< lsst::ndarray::tables::Layout<T> & > : public detail::builtin_to_python {
    inline PyObject * operator()(lsst::ndarray::tables::Layout<T> & x) const {
        return to_python_value< lsst::ndarray::tables::Layout<T> const & >()(x);
    }
    inline PyTypeObject const * get_pytype() const {
        return converter::object_manager_traits<extensions::numpy::dtype>::get_pytype();
    }
};

namespace converter {

template <typename T>
struct arg_to_python< lsst::ndarray::tables::Layout<T> > : public handle<> {
    inline arg_to_python(lsst::ndarray::tables::Layout<T> const & v) :
        handle<>(to_python_value<lsst::ndarray::tables::Layout<T> const &>()(v)) {}
};

template <typename T>
struct arg_rvalue_from_python< lsst::ndarray::tables::Layout<T> const & > {
    typedef lsst::ndarray::tables::Layout<T> result_type;

    arg_rvalue_from_python(PyObject * p) : arg(python::detail::borrowed_reference(p)) {}

    bool convertible() const {
        try {
            extensions::numpy::dtype dtype = extract<extensions::numpy::dtype>(arg);
            if (dtype.attr("fields") == object()) return false;
        } catch (error_already_set) {
            handle_exception();
            PyErr_Clear();
            return false;
        }
        return true;
    }

    result_type operator()() const {
        extensions::numpy::dtype dtype = extract<extensions::numpy::dtype>(arg);
        lsst::ndarray::tables::Layout<T> layout;
        lsst::ndarray::tables::fillLayout(dtype, layout);
        return layout;
    }

    mutable object arg;
};

template <typename T>
struct extract_rvalue< lsst::ndarray::tables::Layout<T> > : private noncopyable {
    typedef lsst::ndarray::tables::Layout<T> result_type;

    extract_rvalue(PyObject * x) : m_converter(x) {}

    bool check() const { return m_converter.convertible(); }
    
    result_type operator()() const { return m_converter(); }

private:
    arg_rvalue_from_python< result_type const & > m_converter;
};

} // namespace converter

}} // namespace boost::python

#endif // !BOOST_PYTHON_EXTENSIONS_NDARRAY_TABLES_LAYOUT_HPP_INCLUDED
