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
#ifndef LSST_NDARRAY_fft_h_INCLUDED
#define LSST_NDARRAY_fft_h_INCLUDED

/**
 * @file lsst/ndarray/fft.h 
 *
 * @brief Main public header file for ndarray FFT library.
 *
 *  \note This file is not included by the main "lsst/ndarray.h" header file.
 */

#include "lsst/ndarray.h"
#include "lsst/ndarray/fft/FourierTransform.h"
#include "lsst/ndarray/fft/FourierOps.h"
#ifndef NDARRAY_FFT_MANUAL_INCLUDE
#include "lsst/ndarray/fft/FourierTransform.cc"
#endif

#endif // !LSST_NDARRAY_fft_h_INCLUDED
