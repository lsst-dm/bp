// -*- lsst-c++ -*-
/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010, 2011 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
#ifndef LSST_NDARRAY_ArrayTraits_h_INCLUDED
#define LSST_NDARRAY_ArrayTraits_h_INCLUDED

/** 
 *  @file lsst/ndarray/ArrayTraits.h
 *
 *  @brief Traits for Array.
 */

#include "lsst/ndarray_fwd.h"
#include "lsst/ndarray/ExpressionTraits.h"
#include "lsst/ndarray/detail/Core.h"
#include <boost/mpl/int.hpp>
#include <boost/mpl/bool.hpp>

namespace lsst { namespace ndarray {

/**
 *  @brief Dimension-specialized traits shared by Array and ArrayRef.
 *
 *  @ingroup MainGroup
 */
template <typename T, int N, int C>
struct ArrayTraits {
    typedef T Element;
    typedef boost::mpl::int_<N> ND;
    typedef boost::mpl::int_<C> RMC;
    typedef detail::NestedIterator<T,N,C> Iterator;
    typedef ArrayRef<T,N-1,(N==C)?(N-1):C> Reference;
    typedef Array<T,N-1,(N==C)?(N-1):C> Value;
    typedef detail::Core<N> Core;
    typedef typename Core::ConstPtr CorePtr;

    static Reference makeReference(Element * data, CorePtr const & core) {
        return Reference(data, core);
    }
    static Iterator makeIterator(Element * data, CorePtr const & core, int stride) {
        return Iterator(Reference(data, core), stride);
    }
};

template <typename T>
struct ArrayTraits<T,1,0> {
    typedef T Element;
    typedef boost::mpl::int_<1> ND;
    typedef boost::mpl::int_<0> RMC;
    typedef detail::StridedIterator<Element> Iterator;
    typedef Element & Reference;
    typedef Element Value;
    typedef detail::Core<1> Core;
    typedef typename Core::ConstPtr CorePtr;

    static Reference makeReference(Element * data, CorePtr const & core) {
        return *data;
    }
    static Iterator makeIterator(Element * data, CorePtr const & core, int stride) {
        return Iterator(data, stride);
    }
};

template <typename T>
struct ArrayTraits<T,1,1> {
    typedef T Element;
    typedef boost::mpl::int_<1> ND;
    typedef boost::mpl::int_<1> RMC;
    typedef Element * Iterator;
    typedef Element & Reference;
    typedef Element Value;
    typedef detail::Core<1> Core;
    typedef typename Core::ConstPtr CorePtr;

    static Reference makeReference(Element * data, CorePtr const & core) {
        return *data;
    }
    static Iterator makeIterator(Element * data, CorePtr const & core, int stride) {
        return data;
    }
};

template <typename T, int N, int C>
struct ExpressionTraits< Array<T,N,C> > : public ArrayTraits<T,N,C> {
    typedef Array<T,N,C> Self;
    typedef boost::mpl::false_ IsScalar;
};

template <typename T, int N, int C>
struct ExpressionTraits< ArrayRef<T,N,C> > : public ArrayTraits<T,N,C> {
    typedef ArrayRef<T,N,C> Self;
    typedef boost::mpl::false_ IsScalar;
};

}} // namespace lsst::ndarray

#endif // !LSST_NDARRAY_ArrayTraits_h_INCLUDED
