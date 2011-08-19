// Copyright 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_EXTENSIONS_STD_PAIR_HPP
#define BOOST_PYTHON_EXTENSIONS_STD_PAIR_HPP

/**
 *  @file std_pair.hpp
 * 
 *  A template-based converters that converts a two-element Python tuple into an std::pair and back.
 *  Including the file is all that is needed to enable the conversions.
 */

#include "boost/python.hpp"

namespace boost { namespace python {

template <typename T1, typename T2>
struct to_python_value< std::pair<T1,T2> const & > : public detail::builtin_to_python {
    inline PyObject * operator()(std::pair<T1,T2> const & x) const {
        object result = make_tuple(x.first, x.second);
        return incref(result.ptr());
    }
    inline PyTypeObject const * get_pytype() const {
        return &PyTuple_Type;
    }
};

template <typename T1, typename T2>
struct to_python_value< std::pair<T1,T2> & > : public to_python_value< std::pair<T1,T2> const & > {};

namespace converter {

template <typename T1, typename T2>
struct arg_to_python< std::pair<T1,T2> > : public handle<> {
    inline arg_to_python(std::pair<T1,T2> const & v) :
        handle<>(to_python_value<std::pair<T1,T2> const &>()(v)) {}
};

template <typename T1, typename T2>
struct arg_rvalue_from_python< std::pair<T1,T2> const & > {
    typedef std::pair<T1,T2> result_type;

    arg_rvalue_from_python(PyObject * p) : arg(python::detail::borrowed_reference(p)) {}

    bool convertible() const {
        return (len(arg) == 2) && extract<tuple>(arg).check();
    }

    result_type operator()() const {
        tuple t = extract<tuple>(arg);
        return result_type(extract<T1>(t[0]), extract<T2>(t[1]));
    }

    mutable object arg;
};

template <typename T1, typename T2>
struct extract_rvalue< std::pair<T1,T2> > : private noncopyable {
    typedef std::pair<T1,T2> result_type;

    extract_rvalue(PyObject * x) : m_converter(x) {}

    bool check() const { return m_converter.convertible(); }
    
    result_type operator()() const { return m_converter(); }

private:
    arg_rvalue_from_python< result_type const & > m_converter;
};


}}} // namespace boost::python::converter

#endif // !BOOST_PYTHON_EXTENSIONS_BOOST_FUSION_HPP
