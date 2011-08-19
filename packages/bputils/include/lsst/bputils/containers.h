// -*- lsst-c++ -*-
#ifndef LSST_BPUTILS_containers_h_INCLUDED
#define LSST_BPUTILS_containers_h_INCLUDED

#include "boost/python.hpp"
#include "indexing_suite/vector.hpp"
#include "indexing_suite/list.hpp"
#include "indexing_suite/set.hpp"
#include "indexing_suite/map.hpp"
#include "indexing_suite/deque.hpp"
#include "boost/python/extensions/container_from_python.hpp"

namespace lsst { namespace bputils {

namespace detail {

template <typename T> struct ContainerTraits;

template <typename U>
struct ContainerTraits< std::vector<U> > {
    typedef boost::python::indexing::vector_suite< std::vector<U> > Suite;
    typedef boost::python::indexing::vector_suite< std::vector<U> const > ConstSuite;
    typedef boost::python::extensions::container_from_python_sequence< std::vector<U> > FromPython;
};

template <typename U>
struct ContainerTraits< std::list<U> > {
    typedef boost::python::indexing::list_suite< std::list<U> > Suite;
    typedef boost::python::indexing::list_suite< std::list<U> const > ConstSuite;
    typedef boost::python::extensions::container_from_python_sequence< std::list<U> > FromPython;
};

template <typename U>
struct ContainerTraits< std::deque<U> > {
    typedef boost::python::indexing::deque_suite< std::deque<U> > Suite;
    typedef boost::python::indexing::deque_suite< std::deque<U> const > ConstSuite;
    typedef boost::python::extensions::container_from_python_sequence< std::deque<U> > FromPython;
};

template <typename U>
struct ContainerTraits< std::set<U> > {
    typedef boost::python::indexing::set_suite< std::set<U> > Suite;
    typedef boost::python::indexing::set_suite< std::set<U> const > ConstSuite;
    typedef boost::python::extensions::container_from_python_sequence< std::set<U> > FromPython;
};

template <typename U, typename V>
struct ContainerTraits< std::map<U,V> > {
    typedef boost::python::indexing::map_suite< std::map<U,V> > Suite;
    typedef boost::python::indexing::map_suite< std::map<U,V> const > ConstSuite;
    typedef boost::python::extensions::container_from_python_mapping< std::map<U,V> > FromPython;
};

} // namespace detail

template <typename Container>
class PyContainer {
public:

    typedef typename detail::ContainerTraits<Container>::Suite Suite;
    typedef typename detail::ContainerTraits<Container>::ConstSuite ConstSuite;
    typedef typename detail::ContainerTraits<Container>::FromPython FromPython;

    static void declare(char const * name) {
        boost::python::object cls = boost::python::extensions::lookup_type<Container>();
        if (cls != boost::python::object()) {
            boost::python::scope().attr(name) = cls;
        } else {
            FromPython();
            boost::python::class_< boost::python::extensions::const_aware< Container > >(name)
                .def(boost::python::init<Container const &>())
                .def(ConstSuite())
                .main_class().def(Suite())
                ;
        }
    }

};

}} // namespace lsst::bputils

#endif // !LSST_BPUTILS_containers_h_INCLUDED
