#include <boost/python/extensions/numpy.hpp>

namespace bp = boost::python;
namespace bpx = boost::python::extensions;

struct UnaryCallable {

    typedef double argument_type;
    typedef double result_type;

    double operator()(double r) const { return r * 2; }

};

struct BinaryCallable {

    typedef double first_argument_type;
    typedef double second_argument_type;
    typedef double result_type;

    double operator()(double a, double b) const { return a * 2 + b * 3; }

};

BOOST_PYTHON_MODULE(ufunc_mod) {
    bpx::numpy::initialize();
    bp::class_< UnaryCallable, boost::shared_ptr<UnaryCallable> >("UnaryCallable")
        .def("__call__", bpx::numpy::unary_ufunc<UnaryCallable>::make());
    bp::class_< BinaryCallable, boost::shared_ptr<BinaryCallable> >("BinaryCallable")
        .def("__call__", bpx::numpy::binary_ufunc<BinaryCallable>::make());

}
