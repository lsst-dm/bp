// -*- lsst-c++ -*-
#ifndef LSST_BPUTILS_exceptions_h_INCLUDED
#define LSST_BPUTILS_exceptions_h_INCLUDED

#include <boost/python.hpp>

namespace lsst { namespace bputils {

namespace detail {

template <typename T>
struct ExceptionConverter {
    ExceptionConverter() {
        boost::python::converter::registry::insert(&extract, boost::python::type_id<T>(), &get_pytype);
    }
private:
    static void * extract(PyObject * op) {
        boost::python::object proxy(boost::python::handle<>(boost::python::borrowed(op)));
        T & r = boost::python::extract<T &>(proxy.attr("args")[0]);
        return &r;
    }
    static PyTypeObject const * get_pytype() {
        boost::python::converter::registration const & r =
            boost::python::converter::registry::lookup(boost::python::type_id<T>());
        return (PyTypeObject*)PyObject_GetAttrString((PyObject*)r.m_class_object, "__exception_proxy__");
    }
};

void addExceptionProxy(char const * name, boost::python::object & wrapper);

} // namespace detail

template <typename T, typename Base>
boost::python::class_< T, boost::python::bases<Base> >
declareException(char const * name) {
    std::string cppName("LsstCpp");
    cppName += name;
    boost::python::class_< T, boost::python::bases<Base> > wrapper(cppName.c_str(), boost::python::no_init);
    detail::addExceptionProxy(name, wrapper);
    detail::ExceptionConverter<T>();
    boost::python::register_ptr_to_python< boost::shared_ptr<T> >();
    return wrapper;
}

}} // namespace lsst::bputils

#endif // !LSST_BPUTILS_exceptions_h_INCLUDED
