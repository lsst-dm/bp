#ifndef BOOST_PYTHON_EXTENSIONS_NUMPY_SCALARS_HPP_INCLUDED
#define BOOST_PYTHON_EXTENSIONS_NUMPY_SCALARS_HPP_INCLUDED

/**
 *  @file boost/python/extensions/numpy/scalars.hpp
 *  @brief Object managers for array scalars (currently only numpy.void is implemented).
 */

#include <boost/python.hpp>
#include <boost/python/extensions/numpy/numpy_object_mgr_traits.hpp>
#include <boost/python/extensions/numpy/dtype.hpp>

namespace boost { namespace python { namespace extensions { namespace numpy {

/**
 *  @brief A boost.python "object manager" (subclass of object) for numpy.void.
 *
 *  @todo This could have a lot more functionality.
 */
class void_ : public object {
    static python::detail::new_reference convert(object_cref arg, bool align);
public:

    /**
     *  @brief Construct a new array scalar with the given size and void dtype.
     *
     *  Data is initialized to zero.  One can create a standalone scalar object
     *  with a certain dtype "dt" with:
     *  @code
     *  void_ scalar = void_(dt.get_itemsize()).view(dt);
     *  @endcode
     */
    explicit void_(Py_ssize_t size);

    BOOST_PYTHON_FORWARD_OBJECT_CONSTRUCTORS(void_, object);

    /// @brief Return a view of the scalar with the given dtype.
    void_ view(dtype const & dt) const;

    /// @brief Copy the scalar (deep for all non-object fields).
    void_ copy() const;

};

}} // namespace extensions::numpy

namespace converter {
NUMPY_OBJECT_MANAGER_TRAITS(python::extensions::numpy::void_);
}}} // namespace boost::python::converter


#endif // !BOOST_PYTHON_EXTENSIONS_NUMPY_SCALARS_HPP_INCLUDED
