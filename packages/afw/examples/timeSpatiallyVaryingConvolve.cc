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
 
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>

#include "lsst/afw/math/FunctionLibrary.h"
#include "lsst/afw/image/Image.h"
#include "lsst/afw/image/MaskedImage.h"
#include "lsst/afw/math/Kernel.h"
#include "lsst/afw/math/KernelFunctions.h"

namespace afwImage = lsst::afw::image;
namespace afwMath = lsst::afw::math;

/**
 * Time convolution with a spatially varying kernel
 *
 * The kernel is a Gaussian that varies as follows:
 * xSigma varies linearly from minSigma to maxSigma as image col goes from 0 to max
 * ySigma varies linearly from minSigma to maxSigma as image row goes from 0 to max
 */
int main(int argc, char **argv) {
    typedef float imageType;
    typedef double kernelType;
    double minSigma = 0.1;
    double maxSigma = 3.0;
    const unsigned int DefNIter = 10;
    const unsigned int MinKernelSize = 5;
    const unsigned int MaxKernelSize = 15;
    const unsigned int DeltaKernelSize = 5;

    std::string mimg;
    if (argc < 2) {
        std::string afwdata = getenv("AFWDATA_DIR");
        if (afwdata.empty()) {
            std::cout << "Usage: timeSpatiallyVaryingConvolve fitsFile [nIter]" << std::endl;
            std::cout << "fitsFile excludes the \"_img.fits\" suffix" << std::endl;
            std::cout << "nIter (default " << DefNIter <<
            ") is the number of iterations per kernel size" << std::endl;
            std::cout << "Kernel size ranges from " << MinKernelSize << " to " << MaxKernelSize
            << " in steps of " << DeltaKernelSize << " pixels on a side" << std::endl;
            std::cerr << "I can take a default file from AFWDATA_DIR, but it's not defined." << std::endl;
            std::cerr << "Is afwdata set up?\n" << std::endl;
            exit(EXIT_FAILURE);
        } else {
            mimg = afwdata + "/small_MI";
            std::cerr << "Using " << mimg << std::endl;
        }
    } else {
        mimg = std::string(argv[1]);
    }
    
    unsigned int nIter = DefNIter;
    if (argc > 2) {
        std::istringstream(argv[2]) >> nIter;
    }
    
    // read in fits file
    afwImage::MaskedImage<imageType> mImage(mimg);
    
    std::cout << "Image is " << mImage.getWidth() << " by " << mImage.getHeight() << std::endl;
    
    afwImage::MaskedImage<imageType> resMImage(mImage.getDimensions());
    
    for (unsigned int kSize = MinKernelSize; kSize <= MaxKernelSize; kSize += DeltaKernelSize) {
        // construct kernel
        afwMath::GaussianFunction2<kernelType> gaussFunc(1, 1, 0);
        unsigned int polyOrder = 1;
        afwMath::PolynomialFunction2<double> polyFunc(polyOrder);
        afwMath::AnalyticKernel gaussSpVarKernel(kSize, kSize, gaussFunc, polyFunc);
    
        // get copy of spatial parameters (all zeros), set and feed back to the kernel
        std::vector<std::vector<double> > polyParams = gaussSpVarKernel.getSpatialParameters();
        polyParams[0][0] = minSigma;
        polyParams[0][1] = (maxSigma - minSigma) / static_cast<double>(mImage.getWidth());
        polyParams[0][2] = 0.0;
        polyParams[1][0] = minSigma;
        polyParams[1][1] = 0.0;
        polyParams[1][2] = (maxSigma - minSigma) / static_cast<double>(mImage.getHeight());
        gaussSpVarKernel.setSpatialParameters(polyParams);
        
        clock_t startTime = clock();
        for (unsigned int iter = 0; iter < nIter; ++iter) {
            // convolve
            afwMath::convolve(resMImage, mImage, gaussSpVarKernel, true);
        }
        double secPerIter = (clock() - startTime) / static_cast<double> (nIter * CLOCKS_PER_SEC);
        std::cout << secPerIter << " sec/convolution for a " << kSize << " by " << kSize <<
            " kernel" << std::endl;
    }
}
