// -*- c++ -*-
#ifndef BPDOX_Processor_h_INCLUDED
#define BPDOX_Processor_h_INCLUDED

#include "bpdox/Macro.h"
#include <map>

namespace bpdox {

class Processor {
    typedef std::map< std::string, boost::shared_ptr<Macro> > MacroMap;
public:

    explicit Processor() : _macros() {}

    void register_(boost::shared_ptr<Macro> const & macro) {
        _macros.insert(std::make_pair(macro->getName(), macro));
    }

    bp::list process(bp::str const & input) const;

    boost::shared_ptr<Macro> findMacro(std::string const & name) const;

private:
    MacroMap _macros;
};

} // namespace bpdox

#endif // BPDOX_Processor_h_INCLUDED
