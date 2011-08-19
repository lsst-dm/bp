// -*- lsst-c++ -*-
#ifndef LSST_BPUTILS_pickle_INCLUDED
#define LSST_BPUTILS_pickle_INCLUDED

#include "boost/python.hpp"

#include "boost/serialization/serialization.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include <sstream>

namespace lsst { namespace bputils {

template <typename T>
struct BoostPickleInterface {
    static std::string getstate(T const & self) {
        std::stringstream ss;
        boost::archive::binary_oarchive ar(ss);
        ar << self;
        return ss.str();
    }
    static void setstate(T & self, std::string const & state) {
        std::stringstream ss(state);
        boost::archive::binary_iarchive ar(ss);
        ar >> self;
    }
    static boost::python::tuple getinitargs(T const & self) {
        return boost::python::make_tuple();
    }
    template <typename Wrapper>
    static void apply(Wrapper & wrapper) {
        wrapper.def("__getstate__", &getstate);
        wrapper.def("__setstate__", &setstate);
        wrapper.def("__getinitargs__", &getinitargs);
        wrapper.enable_pickling();
    }
};

}}

#endif // !LSST_BPUTILS_pickle_INCLUDED
