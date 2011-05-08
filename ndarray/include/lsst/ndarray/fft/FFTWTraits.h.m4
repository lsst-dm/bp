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
changecom(`###')dnl
define(`FFTW_TRAITS',
`
    template <> struct FFTWTraits<$1> {
        BOOST_STATIC_ASSERT((!boost::is_const<$1>::value));
        typedef $2_plan Plan;
        typedef FourierTraits<$1>::ElementX ElementX;
        typedef FourierTraits<$1>::ElementK ElementK;
        typedef boost::shared_ptr<ElementX> OwnerX;
        typedef boost::shared_ptr<ElementK> OwnerK;
        static inline Plan forward(int rank, const int *n, int howmany,	
                                   ElementX *in, const int *inembed, int istride, int idist,
                                   ElementK *out, const int *onembed, int ostride, int odist,
                                   unsigned flags) {			
            return $2_plan_many_dft_r2c(rank, n, howmany,
                                        in, inembed, istride, idist,
                                        reinterpret_cast<$2_complex*>(out),
                                        onembed, ostride, odist,
                                        flags);			
        }
        static inline Plan inverse(int rank, const int *n, int howmany,
                                   ElementK *in, const int *inembed, int istride, int idist,
                                   ElementX *out, const int *onembed, int ostride, int odist,
                                   unsigned flags) {			
            return $2_plan_many_dft_c2r(rank, n, howmany,
                                        reinterpret_cast<$2_complex*>(in),
                                        inembed, istride, idist,
                                        out, onembed, ostride, odist,
                                        flags);			
        }
        static inline void destroy(Plan p) { $2_destroy_plan(p); }
        static inline void execute(Plan p) { $2_execute(p); }	
        static inline OwnerX allocateX(int n) {
            return OwnerX(
                reinterpret_cast<ElementX*>(
                    $2_malloc(sizeof(ElementX)*n)
                ),
                $2_free
            );
        }
        static inline OwnerK allocateK(int n) {
            return OwnerK(
                reinterpret_cast<ElementK*>(
                    $2_malloc(sizeof(ElementK)*n)
                ),
                $2_free
            );
        }
    };
    template <> struct FFTWTraits< std::complex<$1> > {
        typedef $2_plan Plan;
        typedef FourierTraits< std::complex<$1> >::ElementX ElementX;
        typedef FourierTraits< std::complex<$1> >::ElementK ElementK;
        typedef boost::shared_ptr<ElementX> OwnerX;
        typedef boost::shared_ptr<ElementK> OwnerK;
        static inline Plan forward(int rank, const int *n, int howmany,	
                                   ElementX *in, const int *inembed, int istride, int idist,
                                   ElementK *out, const int *onembed, int ostride, int odist,
                                   unsigned flags) {			
            return $2_plan_many_dft(rank, n, howmany,
                                    reinterpret_cast<$2_complex*>(in),
                                    inembed, istride, idist,
                                    reinterpret_cast<$2_complex*>(out),
                                    onembed, ostride, odist,
                                    FFTW_FORWARD, flags);
        }
        static inline Plan inverse(int rank, const int *n, int howmany,
                                   ElementK *in, const int *inembed, int istride, int idist,
                                   ElementX *out, const int *onembed, int ostride, int odist,
                                   unsigned flags) {			
            return $2_plan_many_dft(rank, n, howmany,
                                    reinterpret_cast<$2_complex*>(in),
                                    inembed, istride, idist,
                                    reinterpret_cast<$2_complex*>(out),
                                    onembed, ostride, odist,
                                    FFTW_BACKWARD,flags);
        }
        static inline void destroy(Plan p) { $2_destroy_plan(p); }
        static inline void execute(Plan p) { $2_execute(p); }	
        static inline OwnerX allocateX(int n) {
            return OwnerX(
                reinterpret_cast<ElementX*>(
                    $2_malloc(sizeof(ElementX)*n)
                ),
                $2_free
            );
        }
        static inline OwnerK allocateK(int n) {
            return OwnerK(
                reinterpret_cast<ElementK*>(
                    $2_malloc(sizeof(ElementK)*n)
                ),
                $2_free
            );
        }
    }')dnl
#ifndef LSST_NDARRAY_FFT_FFTWTraits_h_INCLUDED
#define LSST_NDARRAY_FFT_FFTWTraits_h_INCLUDED

/** 
 *  @file lsst/ndarray/fft/FFTWTraits.h
 *
 *  \brief Traits classes that wrap FFTW in a template-friendly interface.
 */

#include <complex>
#include <fftw3.h>
#include "lsst/ndarray/fft/FourierTraits.h"

namespace lsst { namespace ndarray {
/// \cond INTERNAL
namespace detail {

/**
 *  \internal \ingroup FFTInternalGroup
 *  \brief A traits class that maps C++ template types to FFTW types and wraps FFTW function calls.
 */
template <typename T> struct FFTWTraits { BOOST_STATIC_ASSERT(sizeof(T) < 0); };

/// \cond SPECIALIZATIONS
FFTW_TRAITS(float,fftwf);
FFTW_TRAITS(double,fftw);
#ifndef NDARRAY_FFT_NO_LONG_DOUBLE
FFTW_TRAITS(long double, fftwl);
#endif
/// \endcond

} // namespace detail
/// \endcond
}} // namespace lsst::ndarray

#endif // !LSST_NDARRAY_FFT_FFTWTraits_h_INCLUDED
