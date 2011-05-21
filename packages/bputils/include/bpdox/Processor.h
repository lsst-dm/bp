// -*- c++ -*-
#ifndef BPDOX_Processor_h_INCLUDED
#define BPDOX_Processor_h_INCLUDED

#include "bpdox/Macro.h"
#include <map>

namespace bpdox {

class Processor {
    typedef std::map< std::string, boost::shared_ptr<Macro> > MacroMap;
public:

    explicit Processor(bp::object const & state) : _macros(), _state(state) {}

    void register_(boost::shared_ptr<Macro> const & macro) {
        _macros.insert(std::make_pair(macro->getName(), macro));
    }

    std::string process(bp::str const & input) const;

    boost::shared_ptr<Macro> findMacro(std::string const & name) const;

    bp::object const & getState() const { return _state; }

private:
    MacroMap _macros;
    bp::object _state;
};

} // namespace bpdox

#endif // BPDOX_Processor_h_INCLUDED
