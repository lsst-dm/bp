// -*- LSST-C++ -*-
/*!
 * Represent a Psf as a circularly symmetrical double Gaussian
 *
 * \file
 *
 * \ingroup afw
 */
#include <cmath>
#include "lsst/pex/exceptions.h"
#include "lsst/afw/detection/detail/dgPsf.h"
#include "lsst/afw/detection/LocalPsf.h"
#include "lsst/afw/image/ImageUtils.h"

namespace afwMath = lsst::afw::math;

namespace lsst {
namespace afw {
namespace detection {

/************************************************************************************************************/
/**
 * Constructor for a dgPsf
 */
dgPsf::dgPsf(int width,                         ///< Number of columns in realisations of Psf
             int height,                        ///< Number of rows in realisations of Psf
             double sigma1,                     ///< Width of inner Gaussian
             double sigma2,                     ///< Width of outer Gaussian
             double b                   ///< Central amplitude of outer Gaussian (inner amplitude == 1)
            ) :
    KernelPsf(), _sigma1(sigma1), _sigma2(sigma2), _b(b)
{
    if (b == 0.0 && sigma2 == 0.0) {
        sigma2 = 1.0;                  // avoid 0/0 at centre of Psf
    }

    if (sigma1 <= 0 || sigma2 <= 0) {
        throw LSST_EXCEPT(lsst::pex::exceptions::DomainErrorException,
                          (boost::format("sigma may not be 0: %g, %g") % sigma1 % sigma2).str());
    }
    
    if (width > 0) {
        afwMath::DoubleGaussianFunction2<double> dg(sigma1, sigma2, b);
        setKernel(afwMath::Kernel::Ptr(new afwMath::AnalyticKernel(width, height, dg)));
    }
}

PTR(LocalPsf) dgPsf::doGetLocalPsf(
    lsst::afw::geom::Point2D const & center,
    lsst::afw::image::Color const &
) const {
    afwMath::shapelets::MultiShapeletFunction multiShapelet;
    multiShapelet.getElements().push_back(
        afwMath::shapelets::ShapeletFunction(0, afwMath::shapelets::HERMITE, _sigma1, center)
    );
    multiShapelet.getElements().push_back(
        afwMath::shapelets::ShapeletFunction(0, afwMath::shapelets::HERMITE, _sigma2, center)
    );
    multiShapelet.getElements().front().getCoefficients()[0] = 1.0;
    multiShapelet.getElements().back().getCoefficients()[0] = _b;
    return ShapeletLocalPsf::Ptr(new ShapeletLocalPsf(center, multiShapelet));
}

//
// We need to make an instance here so as to register it
//
// \cond
namespace {
    volatile bool isInstance =
        Psf::registerMe<dgPsf, boost::tuple<int, int, double, double, double> >("DoubleGaussian");
}

// \endcond
}}}
