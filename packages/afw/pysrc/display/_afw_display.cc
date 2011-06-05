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
 * \file
 * \brief Write a FITS image to a file descriptor; useful for talking to DS9
 *
 * This version knows about LSST data structures
 */
#include "boost/python.hpp"
#include "simpleFits.h"

namespace bp = boost::python;

namespace lsst { namespace afw { namespace display { namespace {

template <typename ImageT>
void declareTemplates() {
    bp::def(
        "writeBasicFits", 
        (void (*)(int, ImageT const &, image::Wcs const *, char const *))
        &writeBasicFits<ImageT>,
        (bp::arg("fd"), bp::arg("data"), bp::arg("wcs")=0, bp::arg("title")=0)
    );
    bp::def(
        "writeBasicFits", 
        (void (*)(std::string const &, ImageT const &, image::Wcs const *, char const *))
        &writeBasicFits< ImageT >,
        (bp::arg("filename"), bp::arg("data"), bp::arg("wcs")=0, bp::arg("title")=0)
    );
}

}}}}

BOOST_PYTHON_MODULE(_afw_display) {
    bp::import("lsst.afw.image");
    lsst::afw::display::declareTemplates< lsst::afw::image::Image<boost::uint16_t> >();
    lsst::afw::display::declareTemplates< lsst::afw::image::Image<int> >();
    lsst::afw::display::declareTemplates< lsst::afw::image::Image<float> >();
    lsst::afw::display::declareTemplates< lsst::afw::image::Image<double> >();
    lsst::afw::display::declareTemplates< lsst::afw::image::Mask<boost::uint16_t> >();
}
