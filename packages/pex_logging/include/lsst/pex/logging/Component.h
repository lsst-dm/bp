// -*- lsst-c++ -*-

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
 
/** \file Component.h
  *
  * \ingroup pex
  *
  * \brief Defines the (deprecated) Component class
  *
  * \author Robert Lupton, Princeton University
  */

#if !defined(LSST_PEX_UTILS_COMPONENT)            //!< multiple inclusion guard macro
#define LSST_PEX_UTILS_COMPONENT 1

#include <map>
#include <boost/tokenizer.hpp>

namespace lsst {
namespace pex {
namespace logging {


/** \brief Create a component in the verbosity tree (deprecated).
 *
 * \deprecated threshold::Memory class now provides this functionality.
 *
 * A name is a string of the form aaa.bbb.ccc, and may itself contain further
 * subcomponents. The Component structure doesn't in fact contain its full name,
 * but only the first part.
 *
 * The reason for this is inheritance --- verbosity is inherited, but may be
 * overriden.  For example, if "foo" is at level 2 then "foo.goo.hoo" is taken
 * to be at level 2 unless set specifically -- but this inheritance is dynamic
 * (so changing "foo" to 3 changes "foo.goo.hoo" too).  However, I may also set
 * "foo.goo.hoo" explicitly, in which case "foo"'s value is irrelevant --- but
 * "foo.goo" continues to inherit it.
 */
class Component {
public:
    enum { INHERIT_VERBOSITY = -9999};  //!< use parent's verbosity

    Component(const std::string &name = ".", int verbosity=INHERIT_VERBOSITY);
    ~Component();

    void add(const std::string &name, int verbosity,
             const std::string &separator);

    int  getVerbosity(const std::string &name, const std::string &separator);
    int  highestVerbosity(int highest=0);
    void printVerbosity(std::ostream &fp, int depth = 0);
    void setVerbosity(int verbosity) { _verbosity = verbosity; }

private:
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    typedef std::map<const std::string, Component *> comp_map; // subcomponents

    std::string* _name;         // last part of name of this component
    int          _verbosity;    // verbosity for this component
    comp_map    &_subcomp;      // next level of subcomponents

    void add(tokenizer::iterator token, const tokenizer::iterator end,
             int verbosity);
    int getVerbosity(tokenizer::iterator token, const tokenizer::iterator end,
                     int defaultVerbosity);
};

} // namespace logging
} // namespace pex
} // namespace lsst
#endif

