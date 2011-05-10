#include "lsst/bputils/conversions.h"
#include "boost/python/to_python/filesystem.hpp"
#include "boost/python/from_python/filesystem.hpp"
#include "boost/python/from_python/container.hpp"

#include <list>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <string>

#include "boost/mpl/vector.hpp"

namespace bp = boost::python;

namespace lsst { namespace bputils {

namespace {

typedef boost::mpl::vector<
    bool,
    char,
    signed char,
    unsigned char,
    unsigned short,
    signed short,
    unsigned int,
    signed int,
    signed long,
    unsigned long,
    float,
    double,
    long double,
    std::complex<float>,
    std::complex<double>,
    std::string
    > TypeSequence;

struct add_pointer_meta {

    template <typename T>
    struct apply {
        typedef typename boost::add_pointer<T>::type type;
    };

};

template <typename T>
struct RegisterMap {

    template <typename U>
    void operator()(U *) const {
        bp::container_from_python_mapping< std::map<T,U> >();
    }

};

struct RegisterContainers {

    template <typename T>
    void operator()(T *) const {
        bp::container_from_python_sequence< std::list<T> >();
        bp::container_from_python_sequence< std::vector<T> >();
        bp::container_from_python_sequence< std::deque<T> >();
        bp::container_from_python_sequence< std::set<T> >();
        boost::mpl::for_each< TypeSequence, add_pointer_meta >(RegisterMap<T>());
    }

    template <typename T>
    void operator()(std::complex<T> *) const {
        bp::container_from_python_sequence< std::list<T> >();
        bp::container_from_python_sequence< std::vector<T> >();
        bp::container_from_python_sequence< std::deque<T> >();
    }

    void operator()(bool *) const {
        bp::container_from_python_sequence< std::list<bool> >();
        bp::container_from_python_sequence< std::vector<bool> >();
        bp::container_from_python_sequence< std::deque<bool> >();
    }

};

template <typename T>
void registerContainers() {
    boost::mpl::for_each< TypeSequence, add_pointer_meta >(RegisterContainers());
};

} // anonymous

void registerConversions() {
    static bool alreadyRegistered = false;
    if (alreadyRegistered) return;
    bp::filesystem_path_to_python();
    bp::filesystem_path_from_python_str();
    registerContainers<TypeSequence>();
    alreadyRegistered = true;
}

}} // lsst::bputils

