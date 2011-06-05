#include "boost/python.hpp"
#include "bpdox/Macro.h"
#include "bpdox/Processor.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(_bpdox) {

    bp::enum_< bpdox::Option::TypeEnum >("OptionType")
        .value("REF", bpdox::Option::REF)
        .value("LIST", bpdox::Option::LIST)
        .value("STRING", bpdox::Option::STRING)
        .value("CODE", bpdox::Option::CODE)
        .value("BOOL", bpdox::Option::BOOL)
        ;

    bp::class_< bpdox::Macro,boost::noncopyable >("Macro", bp::no_init)
        .def(
            "addOption", &bpdox::Macro::addOption, 
            (bp::arg("name"), bp::arg("type"), bp::arg("default"), bp::arg("doc"))
        )
        .add_property(
            "name",
            bp::make_function(&bpdox::Macro::getName, bp::return_value_policy< bp::copy_const_reference >())
        )
        .def("reorderOptions", &bpdox::Macro::reorderOptions)
        ;

    bp::class_< bpdox::SimpleMacro, bp::bases<bpdox::Macro>, boost::noncopyable >(
        "SimpleMacro", bp::init<std::string const &>()
    );

    bp::class_< bpdox::BlockMacro, bp::bases<bpdox::Macro>, boost::noncopyable >(
        "BlockMacro", bp::init<std::string const &>()
    );

    bp::class_< bpdox::Processor, boost::noncopyable >("ProcessorBase", bp::init<>())
        .def("register", &bpdox::Processor::register_)
        .def("_process", &bpdox::Processor::process, bp::arg("input"))
        ;

    bp::register_ptr_to_python< boost::shared_ptr<bpdox::Macro> >();
    bp::register_ptr_to_python< boost::shared_ptr<bpdox::SimpleMacro> >();
    bp::register_ptr_to_python< boost::shared_ptr<bpdox::BlockMacro> >();
}

