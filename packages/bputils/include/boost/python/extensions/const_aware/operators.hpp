// Copyright 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_EXTENSIONS_CONST_AWARE_OPERATORS_HPP
#define BOOST_PYTHON_EXTENSIONS_CONST_AWARE_OPERATORS_HPP

#include "boost/python.hpp"

namespace boost { namespace python { namespace extensions { namespace const_aware_detail {

template <detail::operator_id id, typename L, typename R>
struct operator_info {

    // Copied from Boost.Python proper, where it's private so we can't get at it.
    typedef typename mpl::eval_if<
        is_same<L,self_t>
        , mpl::if_<
              is_same<R,self_t>
              , detail::binary_op<id>
              , detail::binary_op_l<
                    id
                    , BOOST_DEDUCED_TYPENAME detail::unwrap_other<R>::type
                    >
              >
        , mpl::if_<
              is_same<L,detail::not_specified>
              , detail::unary_op<id>
              , detail::binary_op_r<
                    id
                    , BOOST_DEDUCED_TYPENAME detail::unwrap_other<L>::type
                    >
              >
    >::type generator;

    static bool is_inplace() {
        return (id >= static_cast<int>(detail::op_iadd) && id <= static_cast<int>(detail::op_ior));
    }
        
};

}}}} // namespace boost::python::extensions::const_aware_detail

#endif // !BOOST_PYTHON_EXTENSIONS_CONST_AWARE_OPERATORS_HPP
