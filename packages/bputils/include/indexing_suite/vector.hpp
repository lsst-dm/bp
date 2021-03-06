// Copyright (c) 2003 Raoul M. Gough
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)
//
// Header file vector.hpp
//
// Indexing algorithms support for std::vector instances
//
// History
// =======
// 2003/10/28   rmg     File creation from algo_selector.hpp
// 2008/12/08   Roman   Change indexing suite layout
//
// $Id$
//

#ifndef BOOST_PYTHON_INDEXING_VECTOR_HPP
#define BOOST_PYTHON_INDEXING_VECTOR_HPP

#include <indexing_suite/container_traits.hpp>
#include <indexing_suite/container_suite.hpp>
#include <indexing_suite/algorithms.hpp>
#include <vector>

namespace boost { namespace python {

// Overloads to make std::vector<bool> specialization work.

template <>
struct to_python_value< std::vector<bool>::reference const & > : public detail::builtin_to_python {
    inline PyObject * operator()(std::vector<bool>::reference const & x) const {
        return PyBool_FromLong(bool(x));
    }
    inline PyTypeObject const * get_pytype() const {
        return &PyBool_Type;
    }
};

namespace converter {

template <>
struct arg_to_python< std::vector<bool>::reference > : public handle<> {
    inline arg_to_python(std::vector<bool>::reference v) :
        handle<>(to_python_value<std::vector<bool>::reference const &>()(v)) {}
};

} // namespace converter



namespace indexing {
#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
  namespace detail {
    ///////////////////////////////////////////////////////////////////////
    // algorithms support for std::vector instances
    ///////////////////////////////////////////////////////////////////////

    template <class T, class Allocator>
    class algorithms_selector<std::vector<T, Allocator> >
    {
      typedef std::vector<T, Allocator> Container;

      typedef random_access_sequence_traits<Container>       mutable_traits;
      typedef random_access_sequence_traits<Container const> const_traits;

    public:
      typedef default_algorithms<mutable_traits> mutable_algorithms;
      typedef default_algorithms<const_traits>   const_algorithms;
    };
  }
#endif

  template<
    class Container,
    method_set_type MethodMask = all_methods,
    class Traits = random_access_sequence_traits<Container>
  >
  struct vector_suite
    : container_suite<Container, MethodMask, default_algorithms<Traits> >
  {
  };

} } }

#endif // BOOST_PYTHON_INDEXING_VECTOR_HPP
