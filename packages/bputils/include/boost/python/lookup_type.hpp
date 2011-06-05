// Copyright 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_LOOKUP_TYPE_HPP
#define BOOST_PYTHON_LOOKUP_TYPE_HPP

#include "boost/python.hpp"

namespace boost { namespace python {

template <typename T>
object lookup_type() {
    return object(
        handle<>(borrowed((PyObject*)to_python_value<T const &>().get_pytype()))
    );
}

}} // namespace boost::python

#endif // !BOOST_PYTHON_LOOKUP_TYPE_HPP
