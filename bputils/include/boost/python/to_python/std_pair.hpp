// Copyright 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_TO_PYTHON_STD_PAIR_HPP
#define BOOST_PYTHON_TO_PYTHON_STD_PAIR_HPP

/**
 *  @brief A template-based to-python converter for std::pair that yields a two-element Python tuple.
 */

#include "boost/python.hpp"

namespace boost {
namespace python {

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

} // namespace converter

} // namespace python
} // namespace boost

#endif // !BOOST_PYTHON_TO_PYTHON_STD_PAIR_HPP
