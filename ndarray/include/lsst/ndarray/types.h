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
#ifndef LSST_NDARRAY_types_h_INCLUDED
#define LSST_NDARRAY_types_h_INCLUDED

/// @file lsst/ndarray/types.h @brief Numeric type traits.

#include <complex>
#include <limits>

#include <boost/type_traits/is_complex.hpp>

namespace lsst { namespace ndarray {

/// @addtogroup MainGroup
/// @{

/**
 *  @class NumericTraits
 *  @brief Numeric type traits
 *
 *  Defines Real and Complex versions of numeric types, along with
 *  priority values to aid in type promotion.
 */
template <typename T, typename U=typename boost::remove_const<T>::type,
          typename is_complex=typename boost::is_complex<U>::type,
          typename is_arithmetic=typename boost::is_arithmetic<U>::type> 
struct NumericTraits {};

/// \cond SPECIALIZATIONS
template <typename T, typename U>
struct NumericTraits<T,U,boost::false_type,boost::true_type> {
    typedef U Type;
    typedef boost::true_type IsReal;
    typedef U RealType;
    typedef std::complex<U> ComplexType;
    typedef U ParamType;

    static const int PRIORITY = sizeof(U)  + (sizeof(long long) * (!std::numeric_limits<U>::is_integer));
};

template <typename T, typename U>
struct NumericTraits<T,U,boost::true_type,boost::false_type> {
    typedef U type;
    typedef boost::false_type IsReal;
    typedef typename U::value_type RealType;
    typedef U ComplexType;
    typedef U const & ParamType;

    static const int PRIORITY = NumericTraits<RealType>::PRIORITY;
};
/// \endcond

/**
 *  @class Promote
 *  @brief Metafunction to compute numeric promotions.
 */
template <typename T1, typename T2,
          bool winner=(NumericTraits<T1>::PRIORITY > NumericTraits<T2>::PRIORITY),
    bool is_complex=(NumericTraits<T1>::IsReal::value && NumericTraits<T2>::IsReal::value)
    >
struct Promote {
};

/// \cond SPECIALIZATIONS

// Real, T2 has priority
template <typename T1, typename T2>
struct Promote<T1,T2,false,true> {
    typedef typename NumericTraits<T2>::Type Type;
};

// Real, T1 has priority
template <typename T1, typename T2>
struct Promote<T1,T2,true,true> {
    typedef typename NumericTraits<T1>::Type Type;
};

// Complex, T2 has priority
template <typename T1, typename T2>
struct Promote<T1,T2,false,false> {
    typedef typename NumericTraits<T2>::ComplexType Type;
};

// Complex, T1 has priority
template <typename T1, typename T2>
struct Promote<T1,T2,true,false> {
    typedef typename NumericTraits<T1>::ComplexType Type;
};

/// \endcond

namespace detail {

/**
 *  @internal @ingroup InternalGroup
 *  @brief Provides careful floating point operations for use in floating point comparisons.
 *
 *  Implementation is roughly modeled after the floating point comparisons in the Boost.Test library.
 *
 *  \sa ApproximatelyEqual
 */
template <typename T>
struct SafeFloatingPointOps {

    static inline T divide(T a, T b) {
        if (b < static_cast<T>(1) && a > b*std::numeric_limits<T>::max())
            return std::numeric_limits<T>::max();
        if (a == static_cast<T>(0) || (b > static_cast<T>(1) && a < b*std::numeric_limits<T>::min()))
            return static_cast<T>(0);
        return a / b;
    }

    static inline T abs(T a) {
        return (a < static_cast<T>(0)) ? -a : a;
    }

};

} // namespace detail

/**
 *  @ingroup MainGroup
 *  @brief Binary predicate for floating point equality comparison with tolerance.
 *
 *  Implementation is roughly modeled after the floating point comparisons in the Boost.Test library.
 */
template <typename T1, typename T2=T1>
struct ApproximatelyEqual {
    typedef T1 first_argument_type;
    typedef T2 second_argument_type;
    typedef bool result_type;

    typedef typename Promote<T1,T2>::Type Promoted;
    typedef detail::SafeFloatingPointOps<Promoted> Ops;

    result_type operator()(T1 a, T2 b) const {
        Promoted diff = Ops::abs(a - b);
        Promoted da = Ops::divide(diff,Ops::abs(a));
        Promoted db = Ops::divide(diff,Ops::abs(b));
        return db <= _tolerance && da <= _tolerance;
    }

    explicit ApproximatelyEqual(Promoted tolerance) : _tolerance(Ops::abs(tolerance)) {}

private:
    Promoted _tolerance;
};

/**
 *  @ingroup MainGroup
 *  @brief Binary predicate for complex floating point equality comparison with tolerance.
 *
 *  Implementation is roughly modeled after the floating point comparisons in the Boost.Test library.
 */
template <typename U1, typename U2>
struct ApproximatelyEqual< std::complex<U1>, std::complex<U2> > {
    typedef std::complex<U1> first_argument_type;
    typedef std::complex<U2> second_argument_type;
    typedef bool result_type;

    typedef typename Promote<U1,U2>::Type Promoted;

    result_type operator()(std::complex<U1> const & a, std::complex<U2> const & b) const {
        return _real(a.real(),b.real()) && _real(a.imag(),b.imag());
    }

    explicit ApproximatelyEqual(Promoted tolerance) : _real(tolerance) {}

private:
    ApproximatelyEqual<U1,U2> _real;
};

/// @}

}} // namespace lsst::ndarray

#endif // !LSST_NDARRAY_types_h_INCLUDED
