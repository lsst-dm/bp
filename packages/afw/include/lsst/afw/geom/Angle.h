#if !defined(ANGLE_H)
#define ANGLE_H

#include <limits>
#include <iostream>
#include <boost/math/constants/constants.hpp>

#if 1
#   include <boost/static_assert.hpp>
#   define static_assert(EXPR, MSG) BOOST_STATIC_ASSERT(EXPR) // in C++0x
#endif

namespace lsst { namespace afw { namespace geom {

/************************************************************************************************************/
/*
 * None of C99, C++98, and C++0x define M_PI, so we'll do it ourselves
 */
double const PI = boost::math::constants::pi<double>(); ///< The ratio of a circle's circumference to diameter

#if 0 && !defined(M_PI)                 // a good idea, but with ramifications
#   define M_PI ::lsst::afw::geom::PI
#endif

/************************************************************************************************************/

class Angle;
/**
 * \brief A class used to convert scalar POD types such as double to Angle
 *
 * For example, 
 * \code
 * Angle pi = 180*degrees;
 * \endcode
 * is equivalent to 
 * \code 
 * Angle pi(180, degrees);
 * \endcode
 */
class AngleUnit {
    friend class Angle;
    template<typename T> friend const Angle operator *(T lhs, AngleUnit const rhs);
public:
    explicit AngleUnit(double val) : _val(val) {}
private:
    double _val;

};

// swig likes this way of initialising the constant, so don't mess with it;
// N.b. swig 1.3 doesn't like PI/(60*180)
AngleUnit const radians =    AngleUnit(1.0); ///< constant with units of radians
AngleUnit const degrees =    AngleUnit(PI/180.0); // constant with units of degrees
AngleUnit const arcminutes = AngleUnit(PI/60/180.0); // constant with units of arcminutes
AngleUnit const arcseconds = AngleUnit(PI/180.0/3600); // constant with units of arcseconds

/************************************************************************************************************/
/**
 * A class representing an Angle
 *
 * Angles may be manipulated like doubles, and automatically converted to doubles, but they may not be
 * constructed from doubles without calling a constructor or multiplying by an AngleUnit
 */
class Angle {
    friend class AngleUnit;
public:
    /** Construct an Angle with the specified value (interpreted in the given units) */
    explicit Angle(double val, AngleUnit units=radians) : _val(val*units._val) {}
    /** Convert an Angle to a double in radians*/
    operator double() const { return _val; }

    /** Return an Angle's value as a double in the specified units (i.e. afwGeom::degrees) */
    double asAngularUnits(AngleUnit const& units) const {
        return _val/units._val;
    }
    /** Return an Angle's value as a double in radians */
    double asRadians() const { return asAngularUnits(radians); }
    /** Return an Angle's value as a double in degrees */
    double asDegrees() const { return asAngularUnits(degrees); }
    /** Return an Angle's value as a double in arcminutes */
    double asArcminutes() const { return asAngularUnits(arcminutes); }
    /** Return an Angle's value as a double in arcseconds */
    double asArcseconds() const { return asAngularUnits(arcseconds); }

private:
    double _val;
};

/************************************************************************************************************/
/*
 * Operators for Angles.
 *
 * N.b. We need both int and double versions to avoid ambiguous overloading due to implicit conversion of
 * Angle to double
 */
#define ANGLE_OP(OP)                                                \
    const Angle operator OP(Angle const a, Angle const d) {         \
        return Angle(static_cast<double>(a) OP d);                  \
    }

#define ANGLE_OP_TYPE(OP, TYPE)                             \
    const Angle operator OP(Angle const a, TYPE d) {        \
        return Angle(static_cast<double>(a) OP d);          \
    }                                                       \
                                                            \
    const Angle operator OP(TYPE d, Angle const a) {        \
        return Angle(d OP static_cast<double>(a));          \
    }

ANGLE_OP(+)
ANGLE_OP(-)
ANGLE_OP(*)
ANGLE_OP_TYPE(*, double)
ANGLE_OP_TYPE(*, int)

// Division is different.  Don't allow division by an Angle
const Angle operator /(Angle const a, int d) {
    return Angle(static_cast<double>(a)/d);
}

const Angle operator /(Angle const a, double d) {
    return Angle(static_cast<double>(a)/d);
}

template<typename T>
double operator /(T const lhs, Angle const rhs) {
    static_assert((sizeof(T) == 0), "You may not divide by an Angle");
    return 0.0;
}
            
#undef ANGLE_OP
#undef ANGLE_OP_TYPE

/************************************************************************************************************/
/**
 * \brief Allow a user to check if they have an angle (yes; they could do this themselves via trivial TMP)
 */
template<typename T>
bool isAngle(T) {
    return false;
};

bool isAngle(Angle const&) {
    return true;
};

/************************************************************************************************************/
/**
 * \brief Use AngleUnit to convert a POD (e.g. int, double) to an Angle; e.g. 180*afwGeom::degrees
 */
template<typename T>
const Angle operator *(T lhs,              ///< the value to convert
                       AngleUnit const rhs ///< the conversion coefficient
                      ) {
    static_assert(std::numeric_limits<T>::is_specialized,
                  "Only numeric types may be converted to Angles using degrees/radians!");
    return Angle(lhs*rhs._val);
}
/**
 * Output operator for an Angle
 */
std::ostream& operator<<(std::ostream &s, ///< The output stream
                         Angle const a    ///< The angle
                        )
{
    return s << static_cast<double>(a) << " rad";
}

}}}
#endif
