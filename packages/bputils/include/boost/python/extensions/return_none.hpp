// Copyright 2010 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_PYTHON_EXTENSIONS_RETURN_NONE_HPP
#define BOOST_PYTHON_EXTENSIONS_RETURN_NONE_HPP

#include <boost/python.hpp>

namespace boost { namespace python { namespace extensions {

/**
 *  @brief A model of CallPolicies that returns None and disregards the C++ return value.
 */
template <typename Base = default_call_policies>
struct return_none : public Base {

    template <typename T>
    struct converter {

        bool convertible() const { return true; }

        PyObject * operator()(T const & r) const {
            Py_RETURN_NONE;
        }

        static PyTypeObject const * get_pytype() {
            return Py_None->ob_type;
        }
    };

    PyObject * postcall(PyObject *, PyObject * r) {
        return r;
    }

    struct result_converter {
        template <typename T>
        struct apply {
            typedef converter<T> type;
        };
    };

    template <typename Signature>
    struct extract_return_type {
        typedef void type;
    };

};

}}}

#endif // !BOOST_PYTHON_EXTENSIONS_RETURN_NONE_HPP
