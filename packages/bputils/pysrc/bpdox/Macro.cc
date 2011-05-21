#include "bpdox/Macro.h"

namespace bpdox {

OptionList::iterator Macro::findOption(std::string const & name) {
    for (OptionList::iterator i = _options.begin(); i != _options.end(); ++i) {
        if (i->name == name) return i;
    }
    return _options.end();
}

void Macro::reorderOptions(bp::object const & names) {
    int const n = bp::len(names);
    OptionList ordered;
    for (int i = 0; i < n; ++i) {
        OptionList::iterator j = findOption(bp::extract<std::string>(names[i]));
        if (j == _options.end()) {
            PyErr_SetString(PyExc_IndexError, "Name in reorder list not found.");
            bp::throw_error_already_set();
        }
        ordered.splice(ordered.end(), _options, j);
    }
    ordered.splice(ordered.end(), _options);
}

bp::dict Macro::getDefaults() const {
    bp::dict result;
    for (OptionList::const_iterator i = _options.begin(); i != _options.end(); ++i) {
        result[i->name] = i->default_;
    }
    return result;
}

void Macro::call(
    PyObject * self, char const * method, std::string & output, 
    int indent, bp::dict const & options, bp::object const & state
) const {
    bp::handle<> result(PyObject_CallMethod(self, (char*)method, (char*)"dOO", 
                                            indent, options.ptr(), state.ptr()));
    char * buffer;
    Py_ssize_t length;
    if (PyString_AsStringAndSize(result.get(), &buffer, &length) < 0) {
        bp::throw_error_already_set();
    }
    output.append(buffer, length);
}

} // namespace bpdox
