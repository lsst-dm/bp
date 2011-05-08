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
#ifndef LSST_NDARRAY_ArrayRef_h_INCLUDED
#define LSST_NDARRAY_ArrayRef_h_INCLUDED

/** 
 *  @file lsst/ndarray/ArrayRef.h
 *
 *  @brief Definitions for ArrayRef.
 */

#include "lsst/ndarray_fwd.h"
#include "lsst/ndarray/ArrayTraits.h"
#include "lsst/ndarray/ArrayBase.h"
#include "lsst/ndarray/detail/ArrayAccess.h"
#include "lsst/ndarray/Vector.h"
#include "lsst/ndarray/detail/Core.h"
#include "lsst/ndarray/views.h"

namespace lsst { namespace ndarray {

/**
 *  @brief A proxy class for Array with deep assignment operators.
 */
template <typename T, int N, int C>
class ArrayRef : public ArrayBase< ArrayRef<T,N,C> > {
    typedef ArrayBase<ArrayRef> Super;
    typedef typename Super::Core Core;
    typedef typename Super::CorePtr CorePtr;
public:
    typedef typename Super::Iterator Iterator;

    /**
     *  @brief Non-converting copy constructor.
     */
    ArrayRef(ArrayRef const & other) : Super(other._data, other._core) {}

    /**
     *  @brief Converting copy constructor. 
     *
     *  Implicit conversion is allowed for non-const to const and for 
     *  more guaranteed RMC to less guaranteed RMC (see @ref contiguousness).
     */
    template <typename T_, int C_>
    explicit ArrayRef(
        Array<T_,N,C_> const & other
#ifndef DOXYGEN
        , typename boost::enable_if_c<((C_>=C) && boost::is_convertible<T_*,T*>::value),void*>::type=0
#endif
    ) : Super(other._data, other._core) {}

    /**
     *  @brief Converting copy constructor. 
     *
     *  Implicit conversion is allowed for non-const to const and for 
     *  more guaranteed RMC to less guaranteed RMC (see @ref contiguousness).
     */
    template <typename T_, int C_>
    ArrayRef(
        ArrayRef<T_,N,C_> const & other
#ifndef DOXYGEN
        , typename boost::enable_if_c<((C_>=C) && boost::is_convertible<T_*,T*>::value),void*>::type=0
#endif
    ) : Super(other._data, other._core) {}

    /**
     *  @name Assignment and Augmented Assignment Operators
     *
     *  ArrayRef assignment is deep, and requires that
     *  the ArrayRef being assigned to has the same shape as
     *  the input array expression.  Scalar assignment sets
     *  all elements of the ArrayRef to a single value.
     */
    /// @{

    /// \brief = assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator =(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        std::copy(expr.begin(),expr.end(),this->begin());
        return *this;
    }

    /// \brief = assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator =(Scalar const & scalar) const {
        std::fill(this->begin(),this->end(),scalar);
        return *this;
    }

    /// \brief += assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator +=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) += (*j);
        return *this;
    }

    /// \brief += assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator +=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) += scalar;
        return *this;
    }

    /// \brief -= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator -=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) -= (*j);
        return *this;
    }

    /// \brief -= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator -=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) -= scalar;
        return *this;
    }

    /// \brief *= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator *=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) *= (*j);
        return *this;
    }

    /// \brief *= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator *=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) *= scalar;
        return *this;
    }

    /// \brief /= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator /=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) /= (*j);
        return *this;
    }

    /// \brief /= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator /=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) /= scalar;
        return *this;
    }

    /// \brief %= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator %=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) %= (*j);
        return *this;
    }

    /// \brief %= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator %=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) %= scalar;
        return *this;
    }

    /// \brief ^= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator ^=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) ^= (*j);
        return *this;
    }

    /// \brief ^= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator ^=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) ^= scalar;
        return *this;
    }

    /// \brief &= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator &=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) &= (*j);
        return *this;
    }

    /// \brief &= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator &=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) &= scalar;
        return *this;
    }

    /// \brief |= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator |=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) |= (*j);
        return *this;
    }

    /// \brief |= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator |=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) |= scalar;
        return *this;
    }

    /// \brief <<= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator <<=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) <<= (*j);
        return *this;
    }

    /// \brief <<= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator <<=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) <<= scalar;
        return *this;
    }

    /// \brief >>= assignment of arrays and array expressions.
    template <typename Other>
    ArrayRef const &
    operator >>=(ExpressionBase<Other> const & expr) const {
        LSST_NDARRAY_ASSERT(expr.getShape() 
                         == this->getShape().template first<ExpressionBase<Other>::ND::value>());
        Iterator const i_end = this->end();
        typename Other::Iterator j = expr.begin();
        for (Iterator i = this->begin(); i != i_end; ++i, ++j) (*i) >>= (*j);
        return *this;
    }

    /// \brief >>= assignment of scalars.
    template <typename Scalar>
#ifndef DOXYGEN
    typename boost::enable_if<boost::is_convertible<Scalar,T>, ArrayRef const &>::type
#else
    ArrayRef const &
#endif
    operator >>=(Scalar const & scalar) const {
        Iterator const i_end = this->end();
        for (Iterator i = this->begin(); i != i_end; ++i) (*i) >>= scalar;
        return *this;
    }
    ///@}

private:
    template <typename T_, int N_, int C_> friend class Array;
    template <typename T_, int N_, int C_> friend class ArrayRef;
    template <typename T_, int N_, int C_> friend struct ArrayTraits;
    template <typename Derived> friend class ArrayBase;
    template <typename Array_> friend class detail::ArrayAccess;

    /// @internal @brief Construct an ArrayRef from a pointer and Core.
    ArrayRef(T * data, CorePtr const & core) : Super(data, core) {}

};

}} // namespace lsst::ndarray

#endif // !LSST_NDARRAY_ArrayRef_h_INCLUDED
