#include "boost/python.hpp"
#include "boost/python/extensions/const_aware.hpp"
#include "boost/python/extensions/lookup_type.hpp"
#include "boost/python/extensions/container_from_python.hpp"
#include "boost/python/extensions/const_reference_defaults.hpp"
#include "boost/shared_ptr.hpp"
#include "indexing_suite/vector.hpp"

namespace bp = boost::python;
namespace bpx = boost::python::extensions;

class Example {
public:

    static double static_value;
    static float const static_const_value;

    static bool compare_static_value(double value) {
        return value == static_value;
    }

    std::size_t get_address() const { return std::size_t(this); }

    bool non_const_method() { return true; }
    bool const_method() const { return true; }

    int value;

    int get_value() const { return value; }
    void set_value(int v) { value = v; }
    
    Example() : value(0)
    {}

    Example(int v) : value(v)
    {}

    Example operator+(Example const & other) const { return Example(value + other.value); }

    Example operator+(int other) const { return Example(value + other); }

    inline friend Example operator+(int a, Example const & b) { return Example(a + b.value); }

    Example & operator+=(Example const & other) {
        value += other.value;
        return *this;
    }

    Example & operator+=(int other) {
        value += other;
        return *this;
    }

};

static void freeFunctionC(Example const & self) {}

static void freeFunctionNC(Example & self) {}

double Example::static_value = 2.0;
float const Example::static_const_value = 3.0;

struct ConstAwareOwner {

    double value_member;
    double const const_value_member;
    
    Example example_member;
    Example const const_example_member;

    boost::shared_ptr<Example> example_ptr_member;
    boost::shared_ptr<Example const> example_const_ptr_member;

    boost::shared_ptr<Example> const const_example_ptr_member;
    boost::shared_ptr<Example const> const const_example_const_ptr_member;

    explicit ConstAwareOwner() : 
        value_member(2.0), const_value_member(3.0), 
        example_member(), const_example_member(),
        example_ptr_member(new Example()),
        example_const_ptr_member(example_ptr_member),
        const_example_ptr_member(example_ptr_member),
        const_example_const_ptr_member(example_ptr_member)
    {}

};

class Owner {
public:
    typedef Example Ex;

    Example by_value() const { return *example_ptr_member; }
    Example const by_const_value() const { return *example_ptr_member; }
    Example & by_reference() { return *example_ptr_member; }
    Example const & by_const_reference() const { return *example_ptr_member; }
    boost::shared_ptr<Example> by_shared_ptr() { return example_ptr_member; }
    boost::shared_ptr<Example const> by_const_shared_ptr() const { return example_ptr_member; }

    static double static_value;
    static float const static_const_value;

    static bool compare_static_value(double value) {
        return value == static_value;
    }

    bool accept_by_value(Example x) { return true; }
    bool accept_by_const_value(Example const x) { return true; }
    bool accept_by_reference(Example & x) { return true; }
    bool accept_by_const_reference(Example const & x) { return true; }
    bool accept_by_shared_ptr(boost::shared_ptr<Example> const & x) { return true; }
    bool accept_by_const_shared_ptr(boost::shared_ptr<Example const> const & x) { return true; }

    double value_member;
    double const const_value_member;
    
    Example example_member;
    Example const const_example_member;

    boost::shared_ptr<Example> example_ptr_member;
    boost::shared_ptr<Example const> example_const_ptr_member;

    boost::shared_ptr<Example> const const_example_ptr_member;
    boost::shared_ptr<Example const> const const_example_const_ptr_member;

    explicit Owner() : 
        value_member(2.0), const_value_member(3.0), 
        example_member(), const_example_member(),
        example_ptr_member(new Example()),
        example_const_ptr_member(example_ptr_member),
        const_example_ptr_member(example_ptr_member),
        const_example_const_ptr_member(example_ptr_member)
    {}


};

double Owner::static_value = 2.0;
float const Owner::static_const_value = 3.0;

static void export_module() {

    bp::class_< bpx::const_aware<Example> >("Example")
        .def(bp::init<Example const &>())
        .add_property("address", &Example::get_address)
        .def("non_const_method", &Example::non_const_method)
        .def("const_method", &Example::const_method)
        .add_property("value_prop", &Example::get_value, &Example::set_value)
        .def_readonly("value_ro", &Example::value)
        .def_readwrite("value_rw", &Example::value)
        .enable_shared_ptr()
        .enable_pickling()
        .def(bpx::data_member("static_value", &Example::static_value))
        .def(bpx::data_member("static_const_value", &Example::static_const_value))
        .def("compare_static_value", &Example::compare_static_value)
        .staticmethod("compare_static_value")
        .def(bp::init<int>((bp::arg("value")=0)))
        .def("freeFunctionC", &freeFunctionC)
        .def("freeFunctionNC", &freeFunctionNC)
        .def(bp::self + bp::self)
        .def(bp::self + int())
        .def(int() + bp::self)
        .def(bp::self += bp::self)
        .def(bp::self += int())
        ;

    bp::class_< bpx::const_aware<ConstAwareOwner> >("ConstAwareOwner")
        .def(bpx::data_member("value_member", &ConstAwareOwner::value_member))
        .def(bpx::data_member("const_value_member", &ConstAwareOwner::const_value_member))
        .def(bpx::data_member("example_member", &ConstAwareOwner::example_member))
        .def(bpx::data_member("const_example_member", &ConstAwareOwner::const_example_member))
        .def(bpx::data_member("example_ptr_member", &ConstAwareOwner::example_ptr_member))
        .def(bpx::data_member("example_const_ptr_member", &ConstAwareOwner::example_const_ptr_member))
        .def(bpx::data_member("const_example_ptr_member", &ConstAwareOwner::const_example_ptr_member))
        .def(bpx::data_member("const_example_const_ptr_member", &ConstAwareOwner::const_example_const_ptr_member))
        ;

    bp::class_<Owner>("Owner")
        .add_static_property("Ex", &bpx::lookup_type<Example>)
        .def("by_value", &Owner::by_value)
        .def("by_const_value", &Owner::by_const_value, bpx::as_const<>())
        .def("by_reference", &Owner::by_reference, bpx::return_internal<>())
        .def("by_const_reference", &Owner::by_const_reference, bpx::return_internal<>())
        .def("by_shared_ptr", &Owner::by_shared_ptr)
        .def("by_const_shared_ptr", &Owner::by_const_shared_ptr)
        .def("accept_by_value", &Owner::accept_by_value)
        .def("accept_by_const_value", &Owner::accept_by_const_value)
        .def("accept_by_reference", &Owner::accept_by_reference)
        .def("accept_by_const_reference", &Owner::accept_by_const_reference)
        .def("accept_by_shared_ptr", &Owner::accept_by_shared_ptr)
        .def("accept_by_const_shared_ptr", &Owner::accept_by_const_shared_ptr)
        .def(bpx::data_member("static_value", &Owner::static_value))
        .def(bpx::data_member("static_const_value", &Owner::static_const_value))
        .def("compare_static_value", &Owner::compare_static_value)
        .staticmethod("compare_static_value")
        .def(bpx::data_member("value_member", &Owner::value_member))
        .def(bpx::data_member("const_value_member", &Owner::const_value_member))
        .def(bpx::data_member("example_member", &Owner::example_member))
        .def(bpx::data_member("const_example_member", &Owner::const_example_member))
        .def(bpx::data_member("example_ptr_member", &Owner::example_ptr_member))
        .def(bpx::data_member("example_const_ptr_member", &Owner::example_const_ptr_member))
        .def(bpx::data_member("const_example_ptr_member", &Owner::const_example_ptr_member))
        .def(bpx::data_member("const_example_const_ptr_member", &Owner::const_example_const_ptr_member))
        ;

    bp::class_< bpx::const_aware< std::vector<int> > >("IntVector")
        .def(bp::init< std::vector<int> const & >())
        .def(bp::indexing::vector_suite< std::vector<int> const >())
        .main_class().def(bp::indexing::vector_suite< std::vector<int> >())
        ;
    bpx::container_from_python_sequence< std::vector<int> >();

}

BOOST_PYTHON_MODULE(const_aware_mod) {
    export_module();
}
