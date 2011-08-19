// Copyright 2010 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_EXTENSIONS_CONST_REFERENCE_DEFAULTS_HPP
#define BOOST_PYTHON_EXTENSIONS_CONST_REFERENCE_DEFAULTS_HPP

/**
 *  @file const_reference_derfaults.hpp
 *
 *  Specializations of default_result_converter that allow certain const
 *  references (numeric types, strings, shared_ptrs) to be returned to
 *  Python by-value without generating a compiler error due to lack of
 *  call policies.
 *
 *  Including the file is sufficient to enable the functionality.
 */

#include <boost/python.hpp>

#define SPECIALIZE_DEFAULT_RESULT_CONVERTER(TYPE)                       \
    template <>                                                         \
    struct default_result_converter::apply<TYPE const &> {       \
        typedef boost::python::to_python_value<TYPE const &> type; \
    };

namespace boost { namespace python {

SPECIALIZE_DEFAULT_RESULT_CONVERTER(std::string);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(bool);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(signed char);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(char);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(unsigned char);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(signed short);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(unsigned short);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(signed int);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(unsigned int);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(signed long);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(unsigned long);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(signed long long);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(unsigned long long);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(float);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(double);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(long double);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(std::complex<float>);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(std::complex<double>);
SPECIALIZE_DEFAULT_RESULT_CONVERTER(std::complex<long double>);

template <typename T>
struct default_result_converter::apply< shared_ptr<T> const & > {
    typedef boost::python::to_python_value< shared_ptr<T> const & > type;
};

template <typename T>
struct default_result_converter::apply< shared_ptr<T const> const & > {
    typedef boost::python::to_python_value< shared_ptr<T const> const & > type;
};

}} // namespace boost::python

#endif // !BOOST_PYTHON_EXTENSIONS_CONST_REFERENCE_DEFAULTS_HPP
