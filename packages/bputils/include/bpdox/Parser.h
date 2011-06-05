// -*- c++ -*-
#ifndef BPDOX_Parser_h_INCLUDED
#define BPDOX_Parser_h_INCLUDED

#include "bpdox/Processor.h"

namespace bpdox {

class Parser {
public:

    static char const MARKER = '@';

    Parser(char const * current, char const * end, Processor const & processor, bp::list data) :
        _current(current), _end(end), _lineNumber(1), _indent(0), _braces(0),
        _output(), _processor(processor), _data(data)
    {}

    void parse();

private:

    struct ActiveBlock {
        int braces;
        int indent;
        int lineNumber;
        bp::dict options;
        boost::shared_ptr<Macro> macro;
    };

    void parseMacro();
    void parseComment();
    bp::dict processArgs(Macro & macro);
    void consumeSpace(bool throwAtEnd=false);
    bool finishArg(char const close);
    std::string readString();
    std::string readWord();
    std::string readCode();
    bool readBool();
    bp::tuple readRef();
    bp::list readList();

    char const * _current;
    char const * const _end;
    int _lineNumber;
    int _indent;
    int _braces;
    std::string _output;
    Processor const & _processor;
    std::list<ActiveBlock> _active;
    bp::list _data;
};

} // namespace bpdox

#endif // !BPDOX_Parser_h_INCLUDED
