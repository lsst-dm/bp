#include "bpdox/Processor.h"
#include "bpdox/Parser.h"

namespace bpdox {

std::string Processor::process(bp::str const & input) const {
    std::string result;
    char * buffer;
    Py_ssize_t length;
    if (PyString_AsStringAndSize(input.ptr(), &buffer, &length) < 0) {
        bp::throw_error_already_set();
    }
    Parser parser(buffer, buffer + length, result, *this);
    parser.parse();
    return result;
}

boost::shared_ptr<Macro> Processor::findMacro(std::string const & name) const {
    MacroMap::const_iterator i = _macros.find(name);
    if (i == _macros.end()) 
        return boost::shared_ptr<Macro>();
    return i->second;
}

} // namespace bpdox
