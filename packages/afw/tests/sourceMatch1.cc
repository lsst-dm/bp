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
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SourceMatch

#include <cmath>

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include "lsst/afw/detection/Source.h"
#include "lsst/afw/detection/SourceMatch.h"
#include "lsst/afw/math/Random.h"
#include "lsst/afw/coord/Utils.h"


namespace det = lsst::afw::detection;
namespace math = lsst::afw::math;
namespace coord = lsst::afw::coord;

namespace {

double const PI = 3.14159265358979323846;

math::Random & rng() {
    static math::Random * generator = 0;
    if (generator == 0) {
        generator = new math::Random(math::Random::MT19937);
    }
    return *generator;
}

// Randomly generate a set of sources that are uniformly distributed across
// the unit sphere (ra/dec space) and the unit box (x,y space).
void makeSources(det::SourceSet &set, int n) {
    for (int i = 0; i < n; ++i) {
        det::Source::Ptr src(new det::Source);
        src->setSourceId(i);
        src->setXAstrom(rng().uniform());
        src->setYAstrom(rng().uniform());
        double z = rng().flat(-1.0, 1.0);
        src->setRa(rng().flat(0.0, 2.*M_PI));
        src->setDec(std::asin(z));
        set.push_back(src);
    }
}

struct CmpSourceMatch {
    bool operator()(det::SourceMatch const &m1, det::SourceMatch const &m2) {
        if (m1.first->getSourceId() == m2.first->getSourceId()) {
            return m1.second->getSourceId() < m2.second->getSourceId();
        }
        return m1.first->getSourceId() < m2.first->getSourceId(); 
    }
};

struct DistRaDec {
    double operator()(det::Source::Ptr const &s1, det::Source::Ptr const &s2) const {
        // halversine distance formula
        double sinDeltaRa = std::sin(0.5*(s2->getRa() - s1->getRa()));
        double sinDeltaDec = std::sin(0.5*(s2->getDec() - s1->getDec()));
        double cosDec1CosDec2 = std::cos(s1->getDec())*std::cos(s2->getDec());
        double a = sinDeltaDec*sinDeltaDec + cosDec1CosDec2*sinDeltaRa*sinDeltaRa;
        double b = std::sqrt(a);
        double c = b > 1 ? 1 : b;
        return 3600.0*(180.0/PI)*2.0*std::asin(c);
    }
};

struct DistXy {
    double operator()(det::Source::Ptr const &s1, det::Source::Ptr const &s2) const {
        double dx = s2->getXAstrom() - s1->getXAstrom();
        double dy = s2->getYAstrom() - s1->getYAstrom();
        return std::sqrt(dx*dx + dy*dy);
    }
};

template <typename DistFunctorT>
std::vector<det::SourceMatch> bruteMatch(det::SourceSet const &set,
                                         double radius,
                                         DistFunctorT const &distFun) {
    std::vector<det::SourceMatch> matches;
    for (det::SourceSet::const_iterator i1(set.begin()), e(set.end()); i1 != e; ++i1) {
        for (det::SourceSet::const_iterator i2(i1); i2 != e; ++i2) {
            if (i1 == i2) {
                continue;
            }
            double d = distFun(*i1, *i2);
            if (d <= radius) {
               matches.push_back(det::SourceMatch(*i1, *i2, d));
               matches.push_back(det::SourceMatch(*i2, *i1, d));
            }
        }
    }
    return matches;
}

template <typename DistFunctorT>
std::vector<det::SourceMatch> bruteMatch(det::SourceSet const &set1,
                                         det::SourceSet const &set2,
                                         double radius,
                                         DistFunctorT const &distFun) {
    if (&set1 == &set2) {
        return bruteMatch(set1, radius, distFun);
    }
    std::vector<det::SourceMatch> matches;
    for (det::SourceSet::const_iterator i1(set1.begin()), e1(set1.end()); i1 != e1; ++i1) {
        for (det::SourceSet::const_iterator i2(set2.begin()), e2(set2.end()); i2 != e2; ++i2) {
            double d = distFun(*i1, *i2);
            if (d <= radius) {
                matches.push_back(det::SourceMatch(*i1, *i2, d));
            }
        }
    }
    return matches;
}

// The distance computation for the brute-force and actual match algorithm is different,
// so we cannot naively test whether both match lists are identical. Instead,  check
// that any tuple in one result set but not the other has a match distance very close
// to the match radius.
void compareMatches(std::vector<det::SourceMatch> &matches,
                    std::vector<det::SourceMatch> &refMatches,
                    double radius) {
    double const tolerance = 1e-6; // 1 micro arcsecond
    CmpSourceMatch lessThan;

    std::sort(matches.begin(), matches.end(), lessThan);
    std::sort(refMatches.begin(), refMatches.end(), lessThan);
    std::vector<det::SourceMatch>::const_iterator i(matches.begin());
    std::vector<det::SourceMatch>::const_iterator j(refMatches.begin());
    std::vector<det::SourceMatch>::const_iterator const iend(matches.end());
    std::vector<det::SourceMatch>::const_iterator const jend(refMatches.end());

    while (i < iend && j < jend) {
        if (lessThan(*i, *j)) {
            BOOST_CHECK(std::fabs(i->distance - radius) <= tolerance);
            ++i;
        } else if (lessThan(*j, *i)) {
            BOOST_CHECK(std::fabs(j->distance - radius) <= tolerance);
            ++j;
        } else {
            BOOST_CHECK(std::fabs(i->distance - j->distance) <= tolerance);
            ++i;
            ++j;
        }
    }
    for (; i < iend; ++i) {
        BOOST_CHECK(std::fabs(i->distance - radius) <= tolerance);
    }
    for (; j < jend; ++j) {
        BOOST_CHECK(std::fabs(j->distance - radius) <= tolerance);
    }
}

} // namespace <anonymous>


BOOST_AUTO_TEST_CASE(matchRaDec) { /* parasoft-suppress  LsstDm-3-2a LsstDm-3-4a LsstDm-4-6 LsstDm-5-25 "Boost non-Std" */
    int const N = 500;    // # of points to generate
    double const M = 8.0; // avg. # of matches
    double const radius = std::acos(1.0 - 2.0*M/N)*(180.0/PI)*3600.0;

    det::SourceSet set1, set2;
    makeSources(set1, N);
    makeSources(set2, N);
    std::vector<det::SourceMatch> matches = det::matchRaDec(set1, set2, radius, false);
    std::vector<det::SourceMatch> refMatches = bruteMatch(set1, set2, radius, DistRaDec()); 
    compareMatches(matches, refMatches, radius);
}

BOOST_AUTO_TEST_CASE(matchSelfRaDec) { /* parasoft-suppress  LsstDm-3-2a LsstDm-3-4a LsstDm-4-6 LsstDm-5-25 "Boost non-Std" */
    int const N = 500;    // # of points to generate
    double const M = 8.0; // avg. # of matches
    double const radius = std::acos(1.0 - 2.0*M/N)*(180.0/PI)*3600.0;

    det::SourceSet set;
    makeSources(set, N);
    std::vector<det::SourceMatch> matches = det::matchRaDec(set, radius, true);
    std::vector<det::SourceMatch> refMatches = bruteMatch(set, radius, DistRaDec());
    compareMatches(matches, refMatches, radius);
}

BOOST_AUTO_TEST_CASE(matchXy) { /* parasoft-suppress  LsstDm-3-2a LsstDm-3-4a LsstDm-4-6 LsstDm-5-25 "Boost non-Std" */
    int const N = 500;    // # of points to generate
    double const M = 8.0; // avg. # of matches
    double const radius = std::sqrt(M/(PI*static_cast<double>(N)));

    det::SourceSet set1, set2;
    makeSources(set1, N);
    makeSources(set2, N);
    std::vector<det::SourceMatch> matches = det::matchXy(set1, set2, radius, false);
    std::vector<det::SourceMatch> refMatches = bruteMatch(set1, set2, radius, DistXy());
    compareMatches(matches, refMatches, radius);
}

BOOST_AUTO_TEST_CASE(matchSelfXy) { /* parasoft-suppress  LsstDm-3-2a LsstDm-3-4a LsstDm-4-6 LsstDm-5-25 "Boost non-Std" */
    int const N = 500;    // # of points to generate
    double const M = 8.0; // avg. # of matches
    double const radius = std::sqrt(M/(PI*static_cast<double>(N)));

    det::SourceSet set;
    makeSources(set, N);
    std::vector<det::SourceMatch> matches = det::matchXy(set, radius, true);
    std::vector<det::SourceMatch> refMatches = bruteMatch(set, radius, DistXy());
    compareMatches(matches, refMatches, radius);
}


static void normalizeRaDec(det::SourceSet ss) {
    for (size_t i=0; i<ss.size(); i++) {
        double r,d;
        r = ss[i]->getRa();
        d = ss[i]->getDec();
        // wrap Dec over the (north) pole
        if (d > M_PI_2) {
            d = M_PI - d;
            r = r + M_PI;
        }
        while (r < 0)
            r += 2.*M_PI;
        while (r >= 2.*M_PI)
            r -= 2.*M_PI;
        ss[i]->setRa(r);
        ss[i]->setDec(d);
    }
}


BOOST_AUTO_TEST_CASE(matchNearPole) {
    det::SourceSet set1;
    det::SourceSet set2;

    // for each source, add a true match right on top, plus one within range
    // and one outside range in each direction.

    double rad = 0.1 * coord::degToRad;
    int id1 = 0;
    int id2 = 1000000;
    for (double  j=0.1; j<1; j+=0.1) {
        for (int i=0; i<360; i+=45) {
            double ra = i * coord::degToRad;
            double dec = (90 - j) * coord::degToRad;
            double ddec1 = rad;
            double dra1 = rad / cos(dec);
            double ddec2 = 2. * rad;
            double dra2 = 2. * rad / cos(dec);

            det::Source::Ptr src1(new det::Source);
            src1->setSourceId(id1);
            id1++;
            src1->setRa(ra);
            src1->setDec(dec);
            set1.push_back(src1);

            // right on top
            det::Source::Ptr src2(new det::Source);
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra);
            src2->setDec(dec);
            set2.push_back(src2);

            // +Dec 1
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra);
            src2->setDec(dec + ddec1);
            set2.push_back(src2);

            // +Dec 2
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra);
            src2->setDec(dec + ddec2);
            set2.push_back(src2);

            // -Dec 1
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra);
            src2->setDec(dec - ddec1);
            set2.push_back(src2);

            // -Dec 2
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra);
            src2->setDec(dec - ddec2);
            set2.push_back(src2);

            // +RA 1
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra + dra1);
            src2->setDec(dec);
            set2.push_back(src2);
            // +RA 2
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra + dra2);
            src2->setDec(dec);
            set2.push_back(src2);

            // -RA 1
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra - dra1);
            src2->setDec(dec);
            set2.push_back(src2);
            // -RA 2
            src2 = det::Source::Ptr(new det::Source());
            src2->setSourceId(id2);
            id2++;
            src2->setRa(ra - dra2);
            src2->setDec(dec);
            set2.push_back(src2);
        }
    }

    normalizeRaDec(set1);
    normalizeRaDec(set2);

    std::vector<det::SourceMatch> matches = det::matchRaDec(set1, set2, rad, false);
    std::vector<det::SourceMatch> refMatches = bruteMatch(set1, set2, rad, DistRaDec()); 
    compareMatches(matches, refMatches, rad);

}


