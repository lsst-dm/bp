// -*- c++ -*-
#ifndef BPDOX_Macro_h_INCLUDED
#define BPDOX_Macro_h_INCLUDED

#include "boost/python.hpp"
#include <string>
#include <list>

namespace bpdox {

namespace bp = boost::python;

struct Option {
    enum TypeEnum { REF, LIST, STRING, CODE, BOOL };

    Option(std::string const & name_, TypeEnum type_, bp::object const default__,
           std::string const & doc_) :
        name(name_), type(type_), default_(default__), doc(doc_)
    {}

    std::string name;
    TypeEnum type;
    bp::object default_;
    std::string doc;
};

typedef std::list<Option> OptionList;
    
class Macro {
public:

    void addOption(
        std::string const & name, Option::TypeEnum type, bp::object const & default_,
        std::string const & doc
    ) {
        _options.push_back(Option(name, type, default_, doc));
    }

    OptionList::iterator findOption(std::string const & name);

    void reorderOptions(bp::object const & names);

    std::string const & getName() const { return _name; }

    bp::dict getDefaults() const;

    OptionList const & getOptions() const { return _options; }

    // Generate the output text for the macro and return true if the macro is a block macro.
    virtual bool apply(
        std::string & output, int indent, bp::dict const & options, bp::object const & state
    ) const = 0;

    virtual void finish(
        std::string & output, int indent, bp::dict const & options, bp::object const & state
    ) const {}

    virtual ~Macro() {}

protected:

    void call(
        PyObject * self, char const * method, std::string & output, int indent, 
        bp::dict const & options, bp::object const & state
    ) const;

    explicit Macro(std::string const & name) : _name(name) {}

    std::string _name;
    OptionList _options;
};

class SimpleMacro : public Macro {
public:

    explicit SimpleMacro(PyObject * self, std::string const & name) :
        Macro(name), _self(self) {}

    virtual bool apply(
        std::string & output, int indent, bp::dict const & options, bp::object const & state
    ) const {
        call(_self, "apply", output, indent, options, state);
        return false;
    }

private:
    PyObject * _self;
};

class BlockMacro : public Macro {
public:

    explicit BlockMacro(PyObject * self, std::string const & name) :
        Macro(name), _self(self) {}

    virtual bool apply(
        std::string & output, int indent, bp::dict const & options, bp::object const & state
    ) const {
        call(_self, "apply", output, indent, options, state);
        return true;
    }

    virtual void finish(
        std::string & output, int indent, bp::dict const & options, bp::object const & state
    ) const {
        call(_self, "finish", output, indent, options, state);
    }

private:
    PyObject * _self;    
};

} // namespace bpdox

#endif // BPDOX_Macro_h_INCLUDED
