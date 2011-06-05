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

    virtual bool isBlock() const = 0;

    virtual ~Macro() {}

protected:

    explicit Macro(std::string const & name) : _name(name) {}

    std::string _name;
    OptionList _options;
};

class SimpleMacro : public Macro {
public:

    explicit SimpleMacro(std::string const & name) : Macro(name) {}

    virtual bool isBlock() const { return false; }

};

class BlockMacro : public Macro {
public:

    explicit BlockMacro(std::string const & name) : Macro(name) {}

    virtual bool isBlock() const { return true; }

};

} // namespace bpdox

#endif // BPDOX_Macro_h_INCLUDED
