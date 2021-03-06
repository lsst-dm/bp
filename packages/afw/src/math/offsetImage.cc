// -*- lsst-c++ -*-

/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
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
 
/**
 * @file
 *
 * Offset an Image (or Mask or MaskedImage) by a constant vector (dx, dy)
 */
#include <iterator>
#include "lsst/afw/math/offsetImage.h"
#include "lsst/afw/image/ImageUtils.h"

namespace afwImage = lsst::afw::image;

namespace lsst {
namespace afw {
namespace math {

/**
 * @brief Return an image offset by (dx, dy) using the specified algorithm
 *
 * @note in general, the output image will be offset by a fractional amount
 * and X0/Y0 will be set.  However, if the offset lies in (-1, 1) in both
 * row and column, we guarantee to perform the shift as requested, and
 * not modify X0/Y0.  This makes it possible for client code to use this
 * routine to e.g. center an image in a given pixel
 *
 * @throw lsst::pex::exceptions::InvalidParameterException if the algorithm's invalid
 */
template<typename ImageT>
typename ImageT::Ptr offsetImage(ImageT const& inImage,  ///< The %image to offset
                                 float dx,               ///< move the %image this far in the column direction
                                 float dy,               ///< move the %image this far in the row direction
                                 std::string const& algorithmName  ///< Type of resampling Kernel to use
                                ) {
    SeparableKernel::Ptr offsetKernel = makeWarpingKernel(algorithmName);

    if (offsetKernel->getWidth() > inImage.getWidth() || offsetKernel->getHeight() > inImage.getHeight()) {
        throw LSST_EXCEPT(pexExcept::LengthErrorException,
                          (boost::format("Image of size %dx%d is too small to offset using a %s kernel (minimum %dx%d)") %
                           inImage.getWidth() %  inImage.getHeight() % algorithmName %
                           offsetKernel->getWidth() % offsetKernel->getHeight()).str());
    }

    typename ImageT::Ptr outImage(new ImageT(inImage, true)); // output image, a deep copy

    std::pair<int, double> deltaX = afwImage::positionToIndex(dx, true); // true => return the std::pair
    std::pair<int, double> deltaY = afwImage::positionToIndex(dy, true);
    //
    // If the offset is in (-1, 1) use it as is, and don't allow an integral part
    //
    if (dx > -1 && dx < 1 && dy > -1 && dy < 1) {
        if(deltaX.first != 0) {
            deltaX.second += deltaX.first;
            deltaX.first = 0;
        }
        if (deltaY.first != 0) {
            deltaY.second += deltaY.first;
            deltaY.first = 0;
        }
    }
    //
    // We won't do the integral part of the shift, but we will set [XY]0 correctly (but only after
    // we've done the convolution as convolve also sets [XY]0)
    //
    // And now the fractional part.  N.b. the fraction parts
    //
    // We seem to have to pass -dx, -dy to setKernelParameters, for reasons RHL doesn't understand
    dx = -deltaX.second;
    dy = -deltaY.second;

    //
    // If the shift is -ve, the generated shift kernel (e.g. Lanczos5) is quite asymmetric, with the
    // largest coefficients to the left of centre.  We therefore move the centre of calculated shift kernel
    // one to the right to center up the largest coefficients
    //
    if (dx < 0) {
        offsetKernel->setCtrX(offsetKernel->getCtrX() + 1);
    }
    if (dy < 0) {
        offsetKernel->setCtrY(offsetKernel->getCtrY() + 1);
    }
    
    offsetKernel->setKernelParameters(std::make_pair(dx, dy));

    convolve(*outImage, inImage, *offsetKernel, true, true);
    outImage->setXY0(geom::Point2I(inImage.getX0() + deltaX.first, inImage.getY0() + deltaY.first));

    return outImage;
}

/************************************************************************************************************/
//
// Explicit instantiations
//
/// \cond
#define INSTANTIATE(TYPE) \
    template afwImage::Image<TYPE>::Ptr offsetImage(afwImage::Image<TYPE> const&, float, float, \
                                                    std::string const&); \
    template afwImage::MaskedImage<TYPE>::Ptr offsetImage(afwImage::MaskedImage<TYPE> const&, float, float, \
                                                          std::string const&);

INSTANTIATE(double)
INSTANTIATE(float)
/// \endcond

}}}
