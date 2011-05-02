// Copyright 2010 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_TO_PYTHON_FILESYSTEM_HPP
#define BOOST_PYTHON_TO_PYTHON_FILESYSTEM_HPP

#include <boost/python.hpp>
#include <boost/filesystem/path.hpp>

namespace boost {
namespace python {

/**
 *  @brief A to-python converter that converts a boost::filesystem::path to a Python str.
 */
struct filesystem_path_to_python {
    typedef boost::python::copy_const_reference::apply<std::string const &>::type Converter;

    static PyObject * convert(boost::filesystem::path const & source) {
        Converter converter;
        return converter(source.string());
    }
    
    static PyTypeObject const * get_pytype() {
        Converter converter;
        return converter.get_pytype();
    }

    filesystem_path_to_python() { declare(); }

    static void declare() {
        boost::python::to_python_converter<boost::filesystem::path,filesystem_path_to_python,true>();
    }

};

} // namespace boost::python
} // namespace boost

#endif // !BOOST_PYTHON_TO_PYTHON_FILESYSTEM_HPP
