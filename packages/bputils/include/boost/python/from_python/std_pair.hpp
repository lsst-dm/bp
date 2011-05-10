// Copyright 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_FROM_PYTHON_STD_PAIR_HPP
#define BOOST_PYTHON_FROM_PYTHON_STD_PAIR_HPP

/**
 *  @brief A template-based from-python converter that converts a two-element Python tuple
 *         into an std::pair.
 */

#include "boost/python.hpp"

namespace boost {
namespace python {
namespace converter {

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

#endif // !BOOST_PYTHON_FROM_PYTHON_BOOST_FUSION_HPP
