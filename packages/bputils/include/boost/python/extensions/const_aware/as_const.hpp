// Copyright 2010, 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_EXTENSIONS_CONST_AWARE_AS_CONST_HPP
#define BOOST_PYTHON_EXTENSIONS_CONST_AWARE_AS_CONST_HPP

#include <boost/python.hpp>
#include <boost/python/extensions/const_aware/proxy_class.hpp>

namespace boost { namespace python { namespace extensions {

/**
 *  Call policy that returns a const proxy to the passed result object.
 *  If the result type doesn't have a __const_proxy__ member,
 *  the original result is returned.
 */
template <typename Base = default_call_policies>
struct as_const : Base {
    static PyObject * postcall(PyObject *, PyObject * result) {
        return const_aware_detail::construct_proxy_class(result);
    }
};

/**
 *  An analogue of return_internal_reference that returns const references and pointers
 *  using const proxies.
 */
template <long unsigned int owner_arg=1, typename Base = default_call_policies>
struct return_internal : public return_internal_reference<owner_arg, Base> {
private:

    template <typename R>
    struct converter_impl : public reference_existing_object::apply<R>::type {
        PyObject * operator()(R r) const {
            PyObject * m = this->reference_existing_object::apply<R>::type::operator()(r);
            if (is_const<typename remove_pointer<typename remove_reference<R>::type>::type>::value) {
                return const_aware_detail::construct_proxy_class(m);
            } else {
                return m;
            }
        }
    };

public:    

    struct result_converter {
        template <typename R> struct apply {
            typedef converter_impl<R> type;
        };
    };

};

}}} // namespace boost::python::extensions

#endif // !BOOST_PYTHON_EXTENSIONS_CONST_AWARE_AS_CONST_HPP
