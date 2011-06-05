#include "bpdox/Processor.h"
#include "bpdox/Parser.h"

namespace bpdox {

bp::list Processor::process(bp::str const & input) const {
    char * buffer;
    Py_ssize_t length;
    if (PyString_AsStringAndSize(input.ptr(), &buffer, &length) < 0) {
        bp::throw_error_already_set();
    }
    bp::list data;
    Parser parser(buffer, buffer + length, *this, data);
    parser.parse();
    return data;
}

boost::shared_ptr<Macro> Processor::findMacro(std::string const & name) const {
    MacroMap::const_iterator i = _macros.find(name);
    if (i == _macros.end()) 
        return boost::shared_ptr<Macro>();
    return i->second;
}

} // namespace bpdox
