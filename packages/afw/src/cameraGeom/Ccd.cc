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
 * \file
 */
#include <algorithm>
#include "lsst/afw/cameraGeom/Ccd.h"

namespace afwGeom = lsst::afw::geom;
namespace afwImage = lsst::afw::image;
namespace cameraGeom = lsst::afw::cameraGeom;

/************************************************************************************************************/
/**
 * Add an Amp to the set known to be part of this Ccd
 *
 *  The \c pos value is the 0-indexed position of the Amp on the CCD; e.g. (4, 1)
 * for the top right Amp on a CCD with serials across the top and bottom, and each serial split 5 ways
 */
void cameraGeom::Ccd::addAmp(afwGeom::Point2I pos,        ///< position of Amp in the Ccd
                             cameraGeom::Amp const& amp_c ///< The amplifier to add to the Ccd's manifest
                            )
{
    cameraGeom::Amp::Ptr amp(new Amp(amp_c)); // the Amp with absolute coordinates
    //
    // Correct Amp's coordinate system to be absolute within CCD
    //
    {
        afwGeom::Box2I ampPixels = amp->getAllPixels();
        amp->shift(
            pos.getX()*ampPixels.getWidth(), 
            pos.getY()*ampPixels.getHeight()
        );
    }
    getAllPixels().include(amp->getAllPixels());

    //
    // Now deal with the geometry after we trim everything except the dataSec
    //
    amp->setTrimmedGeom();
    getAllTrimmedPixels().include(amp->getDataSec(true));
    // insert new Amp, keeping the Amps sorted
    _amps.insert(std::lower_bound(_amps.begin(), _amps.end(), amp, cameraGeom::detail::sortPtr<Amp>()), amp);
    amp->setParent(getThisPtr());

    afwGeom::Extent2I dim = getAllPixels(true).getDimensions() - afwGeom::Extent2I(1);
    setCenterPixel(afwGeom::Point2D(dim[0]*0.5, dim[1]*0.5));
}

/**
 * Return the offset from the Detector centre, in mm, given a pixel position wrt Detector's centre
 * \sa getPositionFromPixel
 */
afwGeom::Point2D cameraGeom::Ccd::getPositionFromIndex(
        afwGeom::Point2D const& pix     ///< Pixel coordinates wrt centre of Ccd
                                     ) const
{
    return getPositionFromIndex(pix, isTrimmed());
}

/**
 * Return the offset from the chip centre, in mm, given a pixel position wrt the chip centre
 * \sa getPositionFromPixel
 */
afwGeom::Point2D cameraGeom::Ccd::getPositionFromIndex(
        afwGeom::Point2D const& pix,    ///< Pixel coordinates wrt Ccd's centre
        bool const isTrimmed            ///< Is this detector trimmed?
                                                      ) const
{
    if (isTrimmed) {
        return cameraGeom::Detector::getPositionFromIndex(pix, isTrimmed);
    }

    double pixelSize = getPixelSize();

    afwGeom::Point2D const & centerPixel= getCenterPixel();
    afwGeom::PointI pos(pix[0] + centerPixel[0], pix[1] + centerPixel[1]);
    cameraGeom::Amp::ConstPtr amp = findAmp(pos);
    afwGeom::Extent2I off(amp->getDataSec(false).getMin() - amp->getDataSec(true).getMin());
    return afwGeom::Point2D((pix[0]-off[0])*pixelSize, (pix[1]-off[1])*pixelSize);
}    

namespace {
    struct findById {
        findById(cameraGeom::Id id) : _id(id) {}
        bool operator()(cameraGeom::Amp::Ptr amp) const {
            return _id == amp->getId();
        }
    private:
        cameraGeom::Id _id;
    };

    struct findByPixel {
        findByPixel(
            afwGeom::Point2I point, 
            bool isTrimmed
        ) :  _point(point),
            _isTrimmed(isTrimmed)
        { }

        bool operator()(cameraGeom::Amp::Ptr amp) const {
            return amp->getAllPixels(_isTrimmed).contains(_point);
        }
    private:
        afwGeom::Point2I _point;
        bool _isTrimmed;
    };
}

/// Set the trimmed status of this Ccd
void cameraGeom::Ccd::setTrimmed(bool isTrimmed ///< True iff the bias/overclock have been removed
                                ) {
    cameraGeom::Detector::setTrimmed(isTrimmed);
    // And the Amps too
    for (iterator ptr = begin(); ptr != end(); ++ptr) {
        (*ptr)->setTrimmed(isTrimmed);
    }
}

/**
 * Find an Amp given an Id
 */
cameraGeom::Amp::Ptr cameraGeom::Ccd::findAmp(cameraGeom::Id const id ///< The desired Id
                                             ) const {
    AmpSet::const_iterator result = std::find_if(_amps.begin(), _amps.end(), findById(id));
    if (result == _amps.end()) {
        throw LSST_EXCEPT(lsst::pex::exceptions::OutOfRangeException,
                          (boost::format("Unable to find Amp with serial %||") % id).str());
    }
    return *result;
}

/**
 * Find an Amp given a position
 */
cameraGeom::Amp::Ptr cameraGeom::Ccd::findAmp(afwGeom::Point2I const& pixel ///< The desired pixel
                                             ) const {
    return findAmp(pixel, isTrimmed());
}

/**
 * Find an Amp given a position and a request for trimmed or untrimmed coordinates
 */
cameraGeom::Amp::Ptr cameraGeom::Ccd::findAmp(afwGeom::Point2I const& pixel, ///< The desired pixel 
                                        bool const isTrimmed                 ///< Is Ccd trimmed?
                                             ) const {
    AmpSet::const_iterator result = std::find_if(_amps.begin(), _amps.end(), findByPixel(pixel, isTrimmed));
    if (result == _amps.end()) {
        throw LSST_EXCEPT(lsst::pex::exceptions::OutOfRangeException,
                          (boost::format("Unable to find Amp containing pixel (%d, %d)") %
                           pixel.getX() % pixel.getY()).str());
    }
    return *result;
}

#include "boost/bind.hpp"

/// Offset a Ccd by the specified amount
void cameraGeom::Ccd::shift(int dx,     ///< How much to offset in x (pixels)
                            int dy      ///< How much to offset in y (pixels)
                        ) {
    Detector::shift(dx, dy);
    
    std::for_each(_amps.begin(), _amps.end(), boost::bind(&Amp::shift, _1, boost::ref(dx), boost::ref(dx)));
}

/************************************************************************************************************/
///
/// Set the Ccd's Orientation
///
/// We also have to fix the amps, of course
///
void cameraGeom::Ccd::setOrientation(
        cameraGeom::Orientation const& orientation // the detector's new Orientation
                                    )
{
#if 0    
    int const n90 = orientation.getNQuarter() - getOrientation().getNQuarter(); // before setting orientation
    afwGeom::Extent2I const dimensions = getAllPixels(false).getDimensions();
#endif
    cameraGeom::Detector::setOrientation(orientation);
#if 0    
    std::for_each(_amps.begin(), _amps.end(),
                  boost::bind(&Amp::rotateBy90, _1, boost::ref(dimensions), boost::ref(n90)));
#endif
}

/************************************************************************************************************/

static void clipDefectsToAmplifier(
        cameraGeom::Amp::Ptr amp,                             // the Amp in question
        std::vector<afwImage::DefectBase::Ptr> const& defects // Defects in this detector
                                  )
{
    amp->getDefects().clear();

    for (std::vector<afwImage::DefectBase::Ptr>::const_iterator ptr = defects.begin(), end = defects.end();
         ptr != end; ++ptr) {
        afwImage::DefectBase::Ptr defect = *ptr;

        afwGeom::Box2I bbox = defect->getBBox();
        bbox.clip(amp->getAllPixels(false));

        if (!bbox.isEmpty()) {
            afwImage::DefectBase::Ptr ndet(new afwImage::DefectBase(bbox));
            amp->getDefects().push_back(ndet);
        }
    }
}

/// Set the Detector's Defect list
void cameraGeom::Ccd::setDefects(
        std::vector<lsst::afw::image::DefectBase::Ptr> const& defects ///< Defects in this detector
                                ) {
    cameraGeom::Detector::setDefects(defects);
    // And the Amps too
    for (iterator ptr = begin(); ptr != end(); ++ptr) {
        clipDefectsToAmplifier(*ptr, defects);
    }
}

