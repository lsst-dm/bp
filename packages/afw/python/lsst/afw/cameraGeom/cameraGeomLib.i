// -*- lsst-++ -*-

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
 
#define CAMERA_GEOM_LIB_I

%define cameraGeomLib_DOCSTRING
"
Python bindings for classes describing the the geometry of a mosaic camera
"
%enddef

%feature("autodoc", "1");
%module(package="lsst.afw.cameraGeom", docstring=cameraGeomLib_DOCSTRING) cameraGeomLib

%{
#include "lsst/afw/image/Image.h"
#include "lsst/afw/image/Mask.h"
#include "lsst/afw/cameraGeom.h"
%}

%include "lsst/p_lsstSwig.i"
%include  "lsst/afw/utils.i" 
#if defined(IMPORT_IMAGE_I)
%import  "lsst/afw/image/imageLib.i" 
#endif

%lsst_exceptions();

SWIG_SHARED_PTR_DERIVED(CameraPtr, lsst::afw::cameraGeom::Detector, lsst::afw::cameraGeom::Camera);

%template(AmpSet) std::vector<boost::shared_ptr<lsst::afw::cameraGeom::Amp> >;
%template(DetectorSet) std::vector<boost::shared_ptr<lsst::afw::cameraGeom::Detector> >;

%define Instantiate(PIXEL_TYPE...)
%template(prepareAmpData)
    lsst::afw::cameraGeom::Amp::prepareAmpData<lsst::afw::image::Image<PIXEL_TYPE> >;
%enddef

Instantiate(boost::uint16_t);
Instantiate(float);
Instantiate(double);
%template(prepareAmpData)
    lsst::afw::cameraGeom::Amp::prepareAmpData<lsst::afw::image::Mask<boost::uint16_t> >;

%definePythonIterator(lsst::afw::cameraGeom::Ccd);
%definePythonIterator(lsst::afw::cameraGeom::DetectorMosaic);

%pythoncode {
class ReadoutCorner(object):
    """A python object corresponding to Amp::ReadoutCorner"""
    def __init__(self, value):
        self.value = value

    def __repr__(self):
        return ["LLC", "LRC", "URC", "ULC"][self.value]
}
