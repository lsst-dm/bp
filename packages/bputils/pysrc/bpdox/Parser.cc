#include "bpdox/Parser.h"
#include "boost/format.hpp"
#include <cctype>
#include <algorithm>

namespace bp = boost::python;

namespace bpdox {

template <typename Format>
void throwSyntaxError(Format const & format) {
    std::string message = format.str();
    PyErr_SetString(PyExc_SyntaxError, message.c_str());
    bp::throw_error_already_set();
}

void Parser::parse() {
    while (_current != _end) {
        switch (*_current) {
        case '"':
            _output.push_back(*_current);
            _output += readString();
            _output.push_back('"');
            break;
        case MARKER:
            parseMacro();
            break;
        case '/':
            parseComment();
            break;
        case '\n':
            _output.push_back(*_current);
            ++_lineNumber;
            ++_current;
            _indent = 0;
            break;
        case '}':
            --_braces;
            if (!_active.empty() && _braces == _active.back().braces) {
                _data.append(_output);
                _output.clear();
                _data.append(
                    bp::make_tuple(
                        _active.back().macro, 
                        "finish", 
                        std::string(_active.back().indent, ' '),
                        _active.back().lineNumber,
                        _active.back().options
                    )
                );
                _active.pop_back();
                ++_current;
                ++_indent;
            } else if (_braces < 0) {
                throwSyntaxError(boost::format("Unmatched '}' on line %d.") % _lineNumber);
            } else {
                _output.push_back(*_current);
                ++_current;
                ++_indent;
            }
            break;
        case '{':
            ++_braces;
        default:
            _output.push_back(*_current);
            ++_current;
            ++_indent;
        }
    }
    _data.append(_output);
    if (_braces > 0) {
        throwSyntaxError(boost::format("Unmatched '{' at end of file."));
    }
}

void Parser::parseMacro() {
    assert(*_current == MARKER);
    if (++_current == _end) {
        throwSyntaxError(
            boost::format("File _ended while processing macro line %d.")
            % _lineNumber
        );
    }
    if (*_current == MARKER) { // two consecutive markers means _output one and move on.
        _output.push_back(MARKER);
        ++_current;
        return;
    }
    int macroIndent = _indent;
    int macroLineNumber = _lineNumber;
    std::string macroName = readWord();
    boost::shared_ptr<Macro> macro = _processor.findMacro(macroName);
    if (!macro) {
        _output.push_back(MARKER);
        _output.append(macroName);
        return;
    }
    consumeSpace();
    bp::dict options;
    if (_current != _end && *_current == '(') {
        options = processArgs(*macro);
    } else {
        options = macro->getDefaults();
    }
    _data.append(_output);
    _output.clear();
    _data.append(
        bp::make_tuple(
            macro, 
            "apply", 
            std::string(macroIndent, ' '),
            macroLineNumber,
            options
        )
    );
    bool isBlock = macro->isBlock();
    if (isBlock) {
        consumeSpace();
        if (_current == _end || *_current != '{') {
            throwSyntaxError(
                boost::format("Expected '{' after '@%s' call on line %d.")
                % macroName % _lineNumber
            );
        }
        ActiveBlock block = { _braces, macroIndent, macroLineNumber, options, macro };
        _active.push_back(block);
        ++_braces;
        ++_current;
    }
}

void Parser::parseComment() {
    assert(*_current == '/');
    ++_current;
    if (_current == _end) return;
    if (*_current == '/') {
        char const * i = std::find(_current + 1, _end, '\n');
        _output.append(_current - 1, i);
        _current = i;
        return;
    } else if (*_current == '*') {
        char const * i = std::find(_current + 1, _end, '*');
        while (_current != _end) {
            if (++i == _end) {
                throwSyntaxError(
                    boost::format("Unterminated comment at end of file on line %d.") % _lineNumber
                );
            }
            if (*i == '/') {
                _output.append(_current - 1, i + 1);
                _current = i + 1;
                return;
            }
            i = std::find(i, _end, '*');
        }
    } else {
        _output.push_back('/');
    }
}

bp::dict Parser::processArgs(Macro & macro) {
    assert(*_current == '(');
    ++_current;
    bp::dict result = macro.getDefaults();
    OptionList const & options = macro.getOptions();
    OptionList::const_iterator positional = options.begin();
    while (positional != options.end()) {
        consumeSpace(true);
        if (*_current == ')') return result;
        if (*_current == '{') {
            if (positional->type != Option::CODE) {
                throwSyntaxError(
                    boost::format("Code values not supported for argument '%s' on line %d.")
                    % positional->name % _lineNumber
                );
            }
            result[positional->name] = readCode();
            if (finishArg(')')) return result;
            ++positional;
        } else if (*_current == '[') {
            if (positional->type != Option::LIST) {
                throwSyntaxError(
                    boost::format("List values not supported for argument '%s' on line %d.")
                    % positional->name % _lineNumber
                );                    
            }
            result[positional->name] = readList();
            if (finishArg(')')) return result;
            ++positional;
        } else if (*_current == '"') {
            if (positional->type != Option::STRING) {
                throwSyntaxError(
                    boost::format("String values not supported for argument '%s' on line %d.")
                    % positional->name % _lineNumber
                );                                        
            }
            result[positional->name] = readString();
            if (finishArg(')')) return result;
            ++positional;
        } else {
            // There's no special delimiter, so we have to see whether this is a positional
            // argument or a keyword argument.
            char const * i = _current;
            bool inBrackets = false;
            while (true) {
                if (*i == '=') {
                    // This is actually a keyword argument. Bail out of both loops so we can
                    // process this argument in the next one.
                    positional = options.end();
                    break;
                } else if (*i == ',' || *i == ')') {
                    if (!inBrackets) {
                        // We found a comma outside of any square brackets before we encounted
                        // a '='; it's a positional argument.
                        if (positional->type == Option::REF) {
                            result[positional->name] = readRef();
                        } else if (positional->type == Option::BOOL) {
                            result[positional->name] = readBool();
                        } else {
                            throwSyntaxError(
                                boost::format("Error parsing argument on line %d.") % _lineNumber
                            );
                        }
                        if (finishArg(')')) return result;
                        ++positional;
                    }
                } else if (*i == '[') {
                    if (inBrackets) {
                        throwSyntaxError(
                            boost::format("Invalid nested brackets on line %d.") % _lineNumber
                        );
                    } else {
                        inBrackets = true;
                    }
                } else if (*i == ']') {
                    if (inBrackets) {
                        inBrackets = false;
                    } else {
                        throwSyntaxError(boost::format("Unmatched ']' on line %d.") % _lineNumber);
                    }
                }
                if (++i == _end) {
                    throwSyntaxError(
                        boost::format("Unexpected EOF while processing macro arguments on line %d.")
                        % _lineNumber
                    );
                }
            }
        }
    }
    // Done with positional arguments, either because we finished them all
    // or because we encountered a keyword argument.
    while (true) {
        consumeSpace(true);
        if (*_current == ')') return result;
        std::string kwd = readWord();
        consumeSpace(true);
        if (*_current != '=') {
            throwSyntaxError(
                boost::format("Invalid character '%c' in keyword argument on line %d.") 
                % (*_current) % _lineNumber
            );
        }
        ++_current;
        consumeSpace(true);
        OptionList::const_iterator option = macro.findOption(kwd);
        if (option == options.end()) {
            throwSyntaxError(
                boost::format("Unknown keyword argument '%s' on line %d.") % kwd % _lineNumber
            );
        }
        switch (option->type) {
        case Option::REF:
            result[kwd] = readRef();
            break;
        case Option::LIST:
            result[kwd] = readList();
            break;
        case Option::STRING:
            result[kwd] = readString();
            break;
        case Option::CODE:
            result[kwd] = readCode();
            break;
        case Option::BOOL:
            result[kwd] = readBool();
            break;
        }
        if (finishArg(')')) return result;
    }
}

void Parser::consumeSpace(bool throwAtEnd) {
    while (_current != _end) {
        if (*_current == '\n') {
            ++_lineNumber;
            _indent = 0;
        } else if (!std::isspace(*_current)) {
            return;
        }
        ++_current;
    }
    if (throwAtEnd) {
        throwSyntaxError(boost::format("Unexpected EOF on line %d.") % _lineNumber);
    }
}

bool Parser::finishArg(char const close) {
    consumeSpace(true);
    if (*_current == ',') {
        ++_current;
        return false;
    } else if (*_current == close) {
        ++_current;
        return true;
    }
    throwSyntaxError(boost::format("Error processing macro arguments on line %d.") % _lineNumber);
    return false;  // just to suppress compiler warnings
}

std::string Parser::readWord() {
    char const * const begin = _current;
    while (_current != _end) {
        if (!std::isalnum(*_current) && *_current != '_') {
            return std::string(begin, _current);
        }
        ++_current;
    }
    throwSyntaxError(
        boost::format("Unexpected EOF on line %d.") % _lineNumber             
    );
    return std::string(); // just to suppress compiler warnings
}

std::string Parser::readString() {
    if (*_current != '"') {
        throwSyntaxError(boost::format("String value must begin with '\"' on line %d.") % _lineNumber);
    }
    char const * const begin = ++_current;
    while (_current != _end) {
        _current = std::find(_current, _end, '"');
        if (_current[-1] != '\\') {
            std::string result(begin, _current);
            ++_current;
            if (result.find('\n') != std::string::npos) {
                throwSyntaxError(
                    boost::format("Unexpected newline in string literal on line %d.")
                    % _lineNumber
                );
            }
            return result;
        }
        ++_current;
    }
    throwSyntaxError(
        boost::format("File ended while processing string literal on line %d.") % _lineNumber
    );
    return std::string(); // just to suppress compiler warnings
}

std::string Parser::readCode() {
    if (*_current != '{') {
        throwSyntaxError(boost::format("Code value must begin with '{' on line %d.") % _lineNumber);
    }
    char const * begin = ++_current;
    int count = 1;
    std::string result;
    while (_current != _end) {
        switch (*_current) {
        case '{':
            ++count;
            ++_current;
            break;
        case '}':
            if (--count == 0) {
                result.append(begin, _current);
                ++_current;
                return result;
            }
            ++_current;
            break;
        case '"':
            result.append(begin, _current);
            result.push_back('"');
            result += readString();
            result.push_back('"');
            begin = _current;
            break;
        case '\n':
            ++_lineNumber;
        default:
            ++_current;
        }
    }
    throwSyntaxError(
        boost::format("File ended while processing code value on line %d.") % _lineNumber
    );
    return std::string(); // just to suppress compiler warnings
}

bool Parser::readBool() {
    std::string s = readWord();
    if (s == "true" || s == "True" || s == "1") {
        return true;
    } else if (s == "false" || s == "False" || s == "0") {
        return false;
    }
    throwSyntaxError(boost::format("Invalid boolean value '%s' on line %d.") % s % _lineNumber);
    return false; // just to suppress compiler warnings
}

bp::tuple Parser::readRef() {
    std::string name = readWord();
    while (_current != _end) {
        if (*_current == ':') {
            ++_current;
            if (_current == _end) {
                throwSyntaxError(
                    boost::format("File ended while processing ref value on line %d.") % _lineNumber
                );
            }
            if (*_current != ':') {
                throwSyntaxError(
                    boost::format("Invalid scope delimiter ':' on line %d.") % _lineNumber
                );
            }
            name += "::";
            ++_current;
            name += readWord();
        } else if (*_current == '[') {
            ++_current;
            bp::list labels;
            while (true) {
                consumeSpace(true);
                labels.append(readWord());
                if (finishArg(']')) return bp::make_tuple(name, labels);
            }
        } else {
            consumeSpace(true);
            if (*_current == ',' || *_current == ')' || *_current == ']') {
                return bp::make_tuple(name, bp::object());
            } else {
                throwSyntaxError(
                    boost::format("Error processing list argument on line %d.") % _lineNumber
                );                
            }
        }
    }
    throwSyntaxError(
        boost::format("File ended while processing ref value on line %d.") % _lineNumber
    );
    return bp::tuple(); // just to suppress compiler warnings
}

bp::list Parser::readList() {
    bp::list result;
    if (*_current != '[') {
        throwSyntaxError(boost::format("List value must begin with '[' on line %d.") % _lineNumber);
    }
    ++_current;
    while (true) {
        consumeSpace(true);
        if (*_current == ']') return result;
        result.append(readRef());
        if (finishArg(']')) return result;
    }
    return result;
}

} // namespace bpdox
