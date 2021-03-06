// -*- LSST-C++ -*-

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
 
#if !defined(LSST_AFW_MATH_INTERPOLATE_H)
#define LSST_AFW_MATH_INTERPOLATE_H
/**
 * @file Interpolate.h
 * @brief Wrap GSL to interpolate values for a set of x,y vector<>s
 * @ingroup afw
 * @author Steve Bickerton
 */
#include <limits>
#include <map>
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"
#include "boost/shared_ptr.hpp"

#include "lsst/pex/exceptions.h"

namespace lsst {
namespace afw {
namespace math {

class Interpolate {
public:

    enum Style {
        CONSTANT = 0,
        LINEAR = 1,
        NATURAL_SPLINE = 2,
        CUBIC_SPLINE = 3,
        CUBIC_SPLINE_PERIODIC = 4,
        AKIMA_SPLINE = 5,
        AKIMA_SPLINE_PERIODIC = 6,
        NUM_STYLES
    };
    
    Interpolate(std::vector<double> const &x, std::vector<double> const &y,
                ::gsl_interp_type const *gslInterpType = ::gsl_interp_akima);
    Interpolate(std::vector<double> const &x, std::vector<double> const &y,
                Interpolate::Style const style);
    Interpolate(std::vector<double> const &x, std::vector<double> const &y,
                std::string style);
    
    void initialize(std::vector<double> const &x, std::vector<double> const &y,
                    ::gsl_interp_type const *gslInterpType);

    virtual ~Interpolate();
    double interpolate(double const x);
    
private:
    std::vector<double> const &_x;
    std::vector<double> const &_y;
    ::gsl_interp_accel *_acc;
    ::gsl_interp *_interp;
};


    
/**
 * @brief Conversion function to switch an Interpolate::Style to a gsl_interp_type.
 */
::gsl_interp_type const *styleToGslInterpType(Interpolate::Style const style);
    
/**
 * @brief Conversion function to switch a string to an Interpolate::Style.
 */
Interpolate::Style stringToInterpStyle(std::string const style);

/**
 * @brief Conversion function to switch a string to a gsl_interp_type.
 */
::gsl_interp_type const *stringToGslInterpType(std::string const style);
    
/**
 * @brief Get the highest order Interpolation::Style available for 'n' points.
 */
    Interpolate::Style lookupMaxInterpStyle(int const n);
    
/**
 * @brief Get the minimum number of points needed to use the requested interpolation style
 */
int lookupMinInterpPoints(Interpolate::Style const style);
    
/**
 * @brief Get the minimum number of points needed to use the requested interpolation style
 * Overload of lookupMinInterpPoints() which takes a string
 */
int lookupMinInterpPoints(std::string const style);
        
}}}
                     
#endif // LSST_AFW_MATH_INTERPOLATE_H
