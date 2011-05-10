/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
 
#ifndef LSST_PEX_EXCEPTIONS_EXCEPTION_H
#define LSST_PEX_EXCEPTIONS_EXCEPTION_H

#include <exception>
#include <ostream>
#include <string>
#include <vector>

#include "boost/current_function.hpp"

namespace lsst {
namespace pex {
namespace exceptions {

/** For internal use; gathers the file, line, and function for a tracepoint.
  */
#define LSST_EXCEPT_HERE __FILE__, __LINE__, BOOST_CURRENT_FUNCTION

/** Create an exception with a given type and message and optionally other
  * arguments (dependent on the type).
  * \param[in] type C++ type of the exception to be thrown.
  */
#define LSST_EXCEPT(type, ...) type(LSST_EXCEPT_HERE, __VA_ARGS__)

/** Add the current location and a message to an existing exception before
  * rethrowing it.
  */
#define LSST_EXCEPT_ADD(e, m) e.addMessage(LSST_EXCEPT_HERE, m)

/** The initial arguments required for new exception subclasses.
  */
#define LSST_EARGS_TYPED \
    char const* ex_file, int ex_line, char const* ex_func, \
    std::string const& ex_message

/** The initial arguments to the base class constructor for new subclasses.
  */
#define LSST_EARGS_UNTYPED ex_file, ex_line, ex_func, ex_message

/** Macro used to define new types of exceptions without additional data.
  * \param[in] t Type of the exception.
  * \param[in] b Base class of the exception.
  * \param[in] c C++ class of the exception (fully specified).
  */
#define LSST_EXCEPTION_TYPE(t, b, c) \
    class t : public b { \
    public: \
        t(LSST_EARGS_TYPED) : b(LSST_EARGS_UNTYPED) { }; \
        virtual char const* getType(void) const throw() { return #c " *"; }; \
        virtual lsst::pex::exceptions::Exception* clone(void) const { \
            return new t(*this); \
        }; \
    };

struct Tracepoint {
    Tracepoint(void);
    Tracepoint(char const* file, int line, char const* func,
               std::string const& message);
    char const* _file; // Compiled strings only; does not need deletion
    int _line;
    char const* _func; // Compiled strings only; does not need deletion
    std::string _msg;
};
typedef std::vector<Tracepoint> Traceback;

class Exception : public std::exception {
public:

    // Constructors
    // Should use LSST_EARGS_TYPED, but that confuses doxygen.
    Exception(char const* file, int line, char const* func,
              std::string const& message);
    virtual ~Exception(void) throw();

    // Modifiers
    void addMessage(char const* file, int line, char const* func,
                    std::string const& message);

    // Accessors
    Traceback const& getTraceback(void) const throw();
    virtual std::ostream& addToStream(std::ostream& stream) const;
    virtual char const* what(void) const throw();
    virtual char const* getType(void) const throw();
    virtual Exception* clone(void) const;

private:
    Traceback _traceback;
};

std::ostream& operator<<(std::ostream& stream, Exception const& e);

}}} // namespace lsst::pex::exceptions

#endif
