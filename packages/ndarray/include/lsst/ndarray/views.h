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
#ifndef LSST_NDARRAY_views_h_INCLUDED
#define LSST_NDARRAY_views_h_INCLUDED

/** 
 *  \file lsst/ndarray/views.h @brief Construction of arbitrary views into arrays.
 */

#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/pop_back.hpp>
#include <boost/fusion/include/front.hpp>
#include <boost/fusion/include/back.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/mpl/count.hpp>
#include <boost/mpl/size.hpp>

namespace lsst { namespace ndarray {
namespace detail {

/** 
 *  @internal @ingroup InternalGroup
 *  @brief A temporary object used in constructing a Core object in a view operation.
 */
template <typename T, int M, int N>
struct CoreTransformer {
    T * _data;
    typename Core<M>::ConstPtr _input;
    typename Core<N>::Ptr _output;
    
    CoreTransformer(
        T * data,
        typename Core<M>::ConstPtr const & input,
        typename Core<N>::Ptr const & output
    ) : _data(data), _input(input), _output(output) {}
    
    template <int M1, int N1>
    CoreTransformer(CoreTransformer<T,M1,N1> const & other) : 
        _data(other._data), _input(other._input), _output(other._output) {}
};

/** 
 *  @internal @ingroup InternalGroup
 *  @brief Simple structure defining a noncontiguous array section.
 */
struct SliceDim {
    int start;
    int stop;
    int step;
    SliceDim(int start_, int stop_, int step_) : start(start_), stop(stop_), step(step_) {}

    int computeSize() const { return (step>1) ? (stop-start+1)/step : stop-start; }

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Compute the dimensions of an Array<T,N,C> after a slice at index I is applied.
     */
    template <int N, int C, int I> struct Dimensions {
        typedef boost::mpl::int_<N> ND;
        typedef boost::mpl::int_<((C<I) ? C : (I-1))> RMC;
    };

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Metafunction for the result type of transform().
     */
    template <typename T, int M, int N> struct TransformResult {
        typedef CoreTransformer<T,M-1,N-1> Type;
    };

    /**
     *  @brief Apply a slice index.
     */
    template <typename T, int M, int N>
    CoreTransformer<T,M-1,N-1> transform(CoreTransformer<T,M,N> & t) const {
        LSST_NDARRAY_ASSERT(step > 0);
        LSST_NDARRAY_ASSERT(start <= stop);
        LSST_NDARRAY_ASSERT(start >= 0);
        LSST_NDARRAY_ASSERT(stop <= t._input->getSize());
        t._data += start * t._input->getStride();
        t._output->setSize(computeSize());
        t._output->setStride(t._input->getStride() * step);
        return t;
    }
};

/**
 *  @internal @ingroup InternalGroup
 *  @brief Simple structure defining a contiguous array section. 
 */
struct RangeDim {
    int start;
    int stop;
    RangeDim(int start_, int stop_) : start(start_), stop(stop_) {}

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Compute the dimensions of an Array<T,N,C> after a range at index I is applied.
     */
    template <int N, int C, int I> struct Dimensions {
        typedef boost::mpl::int_<N> ND;
        typedef boost::mpl::int_<((C<I) ? C : I)> RMC;
    };

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Metafunction for the result type of transform().
     */
    template <typename T, int M, int N, typename Tag> struct TransformResult {
        typedef CoreTransformer<T,M-1,N-1> Type;
    };

    /// @brief Apply a range index.
    template <typename T, int M, int N>
    CoreTransformer<T,M-1,N-1> transform(CoreTransformer<T,M,N> & t) const {
        LSST_NDARRAY_ASSERT(start <= stop);
        LSST_NDARRAY_ASSERT(start >= 0);
        LSST_NDARRAY_ASSERT(stop <= t._input->getSize());
        t._data += start * t._input->getStride();
        t._output->setSize(stop - start);
        t._output->setStride(t._input->getStride());
        return t;
    }
};

/**
 *  @internal @ingroup internalGroup
 *  @brief Empty structure marking a view of an entire dimension.
 */
struct FullDim {

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Compute the dimensions of an Array<T,N,C> after a full dimension index at I is applied.
     */
    template <int N, int C, int I> struct Dimensions {
        typedef boost::mpl::int_<N> ND;
        typedef boost::mpl::int_<C> RMC;
    };

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Metafunction for the result type of transform().
     */
    template <typename T, int M, int N> struct TransformResult {
        typedef CoreTransformer<T,M-1,N-1> Type;
    };

    /// @brief Apply a full dimension index.
    template <typename T, int M, int N>
    CoreTransformer<T,M-1,N-1> transform(CoreTransformer<T,M,N> & t) const {
        t._output->setSize(t._input->getSize());
        t._output->setStride(t._input->getStride());
        return t;
    }
};

/**
 *  @internal @ingroup InternalGroup
 *  @brief Structure marking a single element of a dimension.
 */
struct ScalarDim {
    int n;
    explicit ScalarDim(int n_) : n(n_) {}

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Compute the dimensions of an Array<T,N,C> after a scalar index at I is applied.
     */
    template <int N, int C, int I> struct Dimensions {
        typedef boost::mpl::int_<(N-1)> ND;
        typedef boost::mpl::int_<((C<(I-1)) ? C : (I-1))> RMC;
    };

    /**
     *  @internal @ingroup InternalGroup
     *  @brief Metafunction for the result type of transform().
     */
    template <typename T, int M, int N> struct TransformResult {
        typedef CoreTransformer<T,M-1,N> Type;
    };

    /// @brief Apply a scalar dimension index.
    template <typename T, int M, int N>
    CoreTransformer<T,M-1,N> transform(CoreTransformer<T,M,N> & t) const {
        LSST_NDARRAY_ASSERT(n >= 0);
        LSST_NDARRAY_ASSERT(n < t._input->getSize());
        t._data += n * t._input->getStride();
        return t;
    }
};

/** 
 *  @internal @ingroup InternalGroup  
 *  @brief A recursively-defined traits class for computing the dimensions and contiguousness of views.
 */
template <
    int N, int C, typename Seq_, 
    typename First_ = typename boost::fusion::result_of::value_at< Seq_, boost::mpl::int_<0> >::type, 
    int D = boost::fusion::result_of::size<Seq_>::type::value
    > 
struct ViewTraits;

/// \cond SPECIALIZATIONS
template <int N, int C, typename Seq_>
struct ViewTraits<N,C,Seq_,FullDim,1> {
    typedef boost::mpl::int_<N> ND;
    typedef boost::mpl::int_<C> RMC;
    typedef boost::mpl::int_<N> I;
};

template <int N, int C, typename Seq_>
struct ViewTraits<N,C,Seq_,RangeDim,1> {
    typedef boost::mpl::int_<N> ND;
    typedef boost::mpl::int_<C> RMC;
    typedef boost::mpl::int_<N> I;
};

template <int N, int C, typename Seq_>
struct ViewTraits<N,C,Seq_,SliceDim,1> {
    typedef boost::mpl::int_<N> ND;
    typedef boost::mpl::int_<((C==N) ? (N-1) : C)> RMC;
    typedef boost::mpl::int_<N> I;
};

template <int N, int C, typename Seq_>
struct ViewTraits<N,C,Seq_,ScalarDim,1> {
    typedef boost::mpl::int_<(N-1)> ND;
    typedef boost::mpl::int_<((C==N) ? (N-1) : C)> RMC;
    typedef boost::mpl::int_<N> I;
};

template <int N, int C, typename Seq_, typename First_, int D>
struct ViewTraits {

    typedef ViewTraits<
        N,C,
        typename boost::fusion::result_of::pop_back<Seq_>::type
        > PreviousTraits;

    typedef typename boost::remove_reference<
        typename boost::fusion::result_of::back<Seq_>::type
        >::type Current;

    typedef boost::mpl::int_<(PreviousTraits::I::value-1)> I;
    typedef typename Current::template Dimensions<
        PreviousTraits::ND::value,
        PreviousTraits::RMC::value,
        I::value
        > Updated;
    
    typedef typename Updated::ND ND;
    typedef typename Updated::RMC RMC;
};
/// \endcond

/**
 *  @internal @ingroup InternalGroup
 *  @brief Metafunction that pads a View with extra FullDim indices to make size<Seq_>::type::value == N.
 */
template <int N, typename Seq_, bool IsNormalized=(boost::mpl::template size<Seq_>::type::value==N)>
struct ViewNormalizer {

    typedef typename boost::fusion::result_of::as_vector<
        typename boost::fusion::result_of::push_back<Seq_,FullDim>::type
        >::type Next;

    typedef typename ViewNormalizer<N,Next>::Output Output;

    static Output apply(Seq_ const & input) {
        return ViewNormalizer<N,Next>::apply(
            boost::fusion::push_back(input,FullDim())
        );
    }
};
/// \cond SPECIALIZATIONS
template <int N, typename Seq_>
struct ViewNormalizer<N,Seq_,true> {
    typedef Seq_ Output;
    static Output apply(Seq_ const & input) { return input; }
};
/// \endcond

/**
 *  @internal @ingroup InternalGroup
 *  @brief Static function object that constructs a view Array.
 */
template <typename Array_, typename Seq_>
struct ViewBuilder {
    typedef ExpressionTraits<Array_> Traits;
    typedef typename Traits::Element Element;
    typedef typename Traits::ND InputND;
    typedef typename Traits::RMC InputRMC;
    typedef typename Traits::Core InputCore;
    typedef ViewTraits<InputND::value,InputRMC::value,Seq_> OutputTraits;
    typedef typename OutputTraits::ND OutputND;
    typedef typename OutputTraits::ND OutputRMC;
    typedef ArrayRef<Element,OutputND::value,OutputRMC::value> OutputArray;
    typedef Core<OutputND::value> OutputCore;

    static OutputArray apply(Array_ const & array, Seq_ const & seq) {
        CoreTransformer<Element,InputND::value,OutputND::value> initial(
            array.getData(), 
            ArrayAccess< Array_>::getCore(array),
            OutputCore::create(array.getManager())
        );
        std::pair<Element*,typename OutputCore::Ptr> final = process(seq, initial);
        return ArrayAccess< OutputArray >::construct(final.first, final.second);
    }

    template <int M, int N>
    static std::pair<Element*,typename OutputCore::Ptr>
    process(Seq_ const & seq, CoreTransformer<Element,M,N> t) {
        return process(seq, boost::fusion::at_c<(InputND::value-M)>(seq).transform(t));
    }

    static std::pair<Element*,typename OutputCore::Ptr>
    process(Seq_ const & seq, CoreTransformer<Element,0,0> t) {
        return std::make_pair(t._data,boost::static_pointer_cast<OutputCore>(t._output));
    }
    
};

/**
 *  @internal @ingroup InternalGroup
 *  @brief Wrapper function for ViewBuilder that removes the need to specify its template parameters.
 */
template <typename Array_, typename SeqIn>
typename ViewBuilder<
    Array_, 
    typename ViewNormalizer< ExpressionTraits<Array_>::ND::value, SeqIn >::Output
    >::OutputArray
buildView(Array_ const & array, SeqIn const & seqIn) {
    typedef ViewNormalizer<ExpressionTraits<Array_>::ND::value,SeqIn> Normalizer;
    typedef typename Normalizer::Output SeqOut;
    SeqOut seqOut = Normalizer::apply(seqIn);
    return ViewBuilder<Array_,SeqOut>::apply(array,seqOut);
};

} // namespace detail

/** 
 *  @brief A template meta-sequence that defines an arbitrary view into an unspecified array. 
 *
 *  @ingroup MainGroup
 *
 *  A View is constructed from a call to the global view() function
 *  and subsequent chained calls to operator().
 */
template <typename Seq_ = boost::fusion::vector<> >
struct View {
    typedef Seq_ Sequence; ///< A boost::fusion sequence type
    Seq_ _seq; ///< A boost::fusion sequence of FullDim, RangeDim, SliceDim, and ScalarDim objects.

    explicit View(Seq_ seq) : _seq(seq) {}

    /// @brief The View that results from chaining an full dimension index <b><tt>()</tt></b> to this.
    typedef View<
        typename boost::fusion::result_of::as_vector<
            typename boost::fusion::result_of::push_back<Seq_,detail::FullDim>::type
            >::type
        > Full;
    /// @brief The View that results from chaining a range <b><tt>(start,stop)</tt></b> to this.
    typedef View<
        typename boost::fusion::result_of::as_vector<
            typename boost::fusion::result_of::push_back<Seq_,detail::RangeDim>::type
            >::type
        > Range;
    /// @brief The View that results from chaining a slice <b><tt>(start,stop,step)</tt></b> to this.
    typedef View<
        typename boost::fusion::result_of::as_vector<
            typename boost::fusion::result_of::push_back<Seq_,detail::SliceDim>::type
            >::type
        > Slice;
    /// @brief The View that results from chaining a scalar <b><tt>(n)</tt></b> to this.
    typedef View<
        typename boost::fusion::result_of::as_vector<
            typename boost::fusion::result_of::push_back<Seq_,detail::ScalarDim>::type
            >::type
        > Scalar;

    /// @brief Chain the full next dimension to this.
    Full operator()() const { return Full(boost::fusion::push_back(_seq,detail::FullDim())); }
    
    /// @brief Chain a contiguous range of the next dimension to this.
    Range operator()(int start, int stop) const {
        return Range(boost::fusion::push_back(_seq,detail::RangeDim(start,stop)));
    }

    /// @brief Chain a noncontiguous slice of the next dimension to this.
    Slice operator()(int start, int stop, int step) const {
        return Slice(boost::fusion::push_back(_seq,detail::SliceDim(start,stop,step)));
    }

    /// @brief Chain a single element of the next dimension to this.
    Scalar operator()(int n) const {
        return Scalar(boost::fusion::push_back(_seq,detail::ScalarDim(n)));
    }
};

/// @addtogroup MainGroup
/// @{

/** @brief Start a view definition that includes the entire first dimension. */
inline View< boost::fusion::vector<detail::FullDim> > view() {
    return View< boost::fusion::vector<detail::FullDim> >(
        boost::fusion::vector<detail::FullDim>(detail::FullDim())
    );
}

/** @brief Start a view definition that selects a contiguous range in the first dimension. */
inline View< boost::fusion::vector<detail::RangeDim> > view(int start, int stop) {
    return View< boost::fusion::vector<detail::RangeDim> >(
        boost::fusion::vector<detail::RangeDim>(detail::RangeDim(start,stop))
    );
}

/** @brief Start a view definition that selects a noncontiguous slice of the first dimension. */
inline View< boost::fusion::vector<detail::SliceDim> > view(int start, int stop, int step) {
    return View< boost::fusion::vector<detail::SliceDim> >(
        boost::fusion::vector<detail::SliceDim>(detail::SliceDim(start,stop,step))
    );
}

/** @brief Start a view definition that selects single element from the first dimension. */
inline View< boost::fusion::vector<detail::ScalarDim> > view(int n) {
    return View< boost::fusion::vector<detail::ScalarDim> >(
        boost::fusion::vector<detail::ScalarDim>(detail::ScalarDim(n))
    );
}

/// @}

}} // namespace lsst::ndarray

#endif // !LSST_NDARRAY_views_h_INCLUDED
