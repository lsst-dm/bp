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
 
/**
 * @file Memory.h
 * @brief definition of the Memory class
 * @author Ray Plante
 */
#ifndef LSST_PEX_LOGGING_THRESHOLD_MEMORY_H
#define LSST_PEX_LOGGING_THRESHOLD_MEMORY_H

#include <string>
#include <map>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>

#include "lsst/pex/logging/threshold/enum.h"

namespace lsst {
namespace pex {
namespace logging {
namespace threshold {

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

/**
 * @brief A hierarchical tree structure for holding mappings of names to 
 * threshold values.
 *
 * This contain contains a default threshold value and 0 or more child
 * Family instances, each with a name.  One can get or set the default 
 * threshold value for the Family as a whole or for any arbitrary 
 * descendant.  If any threshold is set to the special value INHERIT,
 * the effective value should be taken to be the threshold of its nearest
 * ancestor.
 */
class Family {
public:

    /** 
     * create a hierarchical container for threshold data
     * @param defaultThreshold  the threshold value to associate with the name
     */
    Family(int defaultThreshold=INHERIT);

    /**
     * delete this container
     */
    ~Family();

    /**
     * return the default threshold for the top of this hierarchy
     */
    int getThreshold() { return _thresh; }
        
    /**
     * return the default threshold for the top of this hierarchy
     */
    void setThreshold(int threshold) { _thresh = threshold; }
        
    /**
     * return the threshold associated with a descendent with a given name
     */
    int getThresholdFor(tokenizer::iterator toptoken, 
                        const tokenizer::iterator& end) const
    {
        const Family *nfd = findDescendant(toptoken, end, true);
        if (nfd == 0 || nfd->_thresh == INHERIT) return _thresh;
        return nfd->_thresh;
    }

    /**
     * set the threshold associated with a descendent with a given name
     */
    void setThresholdFor(tokenizer::iterator toptoken, 
                         const tokenizer::iterator& end,
                         int threshold);

    /**
     * reset the threshold associated with a descendent with a given name
     * to inherit from its parent.
     */
    void resetThresholdFor(tokenizer::iterator toptoken, 
                           const tokenizer::iterator& end);

    /**
     * print the thresholds store in this Family that are not set to INHERIT.
     *
     * This is intended for debugging purposes.
     */
    void printDescThresholds(std::ostream& out, 
                             const std::string& prefix) const;

    /**
     * delete the descendents 
     */
    void deleteDescendants();

protected: 

    /**
     * return the child data for a given descendant name.  Null is returned
     * if the name does not exist.
     * @param toptoken   the first field in the hierarchical name
     * @param end        the end iterator value from the tokenizer that 
     *                     produced toptoken
     */
    Family *ensureDescendant(tokenizer::iterator toptoken, 
                             const tokenizer::iterator& end);

    /**
     * return a threshold Family for a given descendant name.  What is 
     * returned when the name doesn't exist depends on the value of 
     * the orNearest parameter.
     * @param toptoken   the first field in the hierarchical name
     * @param end        the end iterator value from the tokenizer that 
     *                     produced toptoken
     * @param orNearest  if true and the exact named descendant does not 
     *                     exist, return the nearest ancestor that does 
     *                     exist.  If false, return null if it doesn't
     *                     exist.
     */
    const Family *findDescendant(tokenizer::iterator toptoken, 
                                 const tokenizer::iterator& end,
                                 bool orNearest=false) const;

    /**
     * create a threshold Family for a given descendant name.
     * @param toptoken   the first field in the hierarchical name
     * @param end        the end iterator value from the tokenizer that 
     *                     produced toptoken
     */
    Family *makeDescendants(tokenizer::iterator toptoken, 
                            const tokenizer::iterator& end);

private:
    typedef std::map<std::string, Family*> ChildMap;
    int _thresh;
    ChildMap *_children;
};

/**
 * a container for keeping track of the threshold data for a family of Logs.
 * The actually mappings of log names to remembered threshold values is 
 * stored internally (privately) as a Family instance.  One Memory instance 
 * shared by all the Log instances in a Log hierarchy, created first by the 
 * root log and passed (by shared pointer) to child logs as they are created.
 */
class Memory {
public:

    Memory(const std::string& delims=".");

    /**
     * return the threshold value associated with a given name
     */
    int getThresholdFor(const std::string& name) {
        if (name.length() == 0) return getRootThreshold();
        tokenizer fields(name, _sep);
        return _tree.getThresholdFor(fields.begin(), fields.end());
    }

    /**
     * set the threshold value associated with a given name
     */
    void setThresholdFor(const std::string& name, int threshold) {
        if (name.length() == 0) {
            setRootThreshold(threshold);
        }
        else {
            tokenizer fields(name, _sep);
            _tree.setThresholdFor(fields.begin(), fields.end(), threshold);
        }
    }

    /**
     * return the default threshold value associated with the root
     * of the hierarchy.
     */
    int getRootThreshold() { return _tree.getThreshold(); }

    /**
     * return the default threshold value associated with the root
     * of the hierarchy.
     */
    void setRootThreshold(int threshold) { _tree.setThreshold(threshold); }

    /**
     * reset the memory
     */
    void forgetAllNames() { _tree.deleteDescendants(); }

    /**
     * print the thresholds stored in this Memory that are not set to INHERIT.
     */
    void printThresholds(std::ostream& out);


private:
    Family _tree;
    boost::char_separator<char> _sep;
};

}}}} // end lsst::pex::logging::threshold

#endif // end LSST_PEX_LOGGING_THRESHOLD_MEMORY_H
