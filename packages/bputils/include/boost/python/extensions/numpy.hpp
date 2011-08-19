#ifndef BOOST_PYTHON_EXTENSIONS_NUMPY_HPP_INCLUDED
#define BOOST_PYTHON_EXTENSIONS_NUMPY_HPP_INCLUDED

/**
 *  @file boost/python/extensions/numpy.hpp
 *  @brief Main public header file for boost.python.numpy.
 */

#include <boost/python/extensions/numpy/dtype.hpp>
#include <boost/python/extensions/numpy/ndarray.hpp>
#include <boost/python/extensions/numpy/scalars.hpp>
#include <boost/python/extensions/numpy/matrix.hpp>
#include <boost/python/extensions/numpy/ufunc.hpp>
#include <boost/python/extensions/numpy/invoke_matching.hpp>

namespace boost { namespace python { namespace extensions { namespace numpy {

/**
 *  @brief Initialize the Numpy C-API
 *
 *  This must be called before using anything in boost.python.numpy;
 *  It should probably be the first line inside BOOST_PYTHON_MODULE.
 *
 *  @internal This just calls the Numpy C-API functions "import_array()"
 *            and "import_ufunc()".
 */
void initialize();

}}}} // namespace boost::python::extension::numpy

#endif // !BOOST_PYTHON_EXTENSIONS_NUMPY_HPP_INCLUDED
