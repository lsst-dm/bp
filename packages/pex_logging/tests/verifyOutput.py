#! /usr/bin/env python

# 
# LSST Data Management System
# Copyright 2008, 2009, 2010 LSST Corporation.
# 
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the LSST License Statement and 
# the GNU General Public License along with this program.  If not, 
# see <http://www.lsstcorp.org/LegalNotices/>.
#

#
# routines for testing logging output 
#
# The wrapped test program writes output to the screen.  This module tests
# the output by redirecting it to a file and comparing with canonical
# results.  Note that when the underlying test program is changed, the file
# containing the canonical results usually must change as well.  

import sys, os, re
import subprocess 

def captureOutput(prog, outfile):
    """execute the given program and append the output to the given file.
    """
    out = open(outfile, 'w')
    test = subprocess.Popen(prog, bufsize=1, close_fds=True,
                            stderr=subprocess.PIPE)
    filter = subprocess.Popen("egrep -v DATE:|TIMESTAMP:".split(), 
                              bufsize=1, close_fds=True,
                              stdin=test.stderr, stdout=out)
    filter.wait()
    excode = test.wait()
    out.close()

    if excode:
        print >> sys.stdout, prog, "exited with status", excode
    return excode == 0

def compareOutput(test, canon, showdiff=True):
    """return True if the two files have the same contents; False, otherwise.
    @param test       the output file to write results to
    @param canon      a file containing the "correct" results
    @param showdiff   if True (the default), print differences to stdout
    """
    corr = file(canon)
    act = file(test)

    fields = {}
    list = []
    pat = re.compile(r'^  ([A-Z\._]*):(.*)')

    for line in corr:
        match = pat.match(line)
        if match:
            fields[match.group(1)] = match.group(2)
        else:
            list.append([line, fields])
            fields = {}
    corr.close()
    if len(fields) != 0:
        list.append([None, fields])

    fields = {}
    for line in act:
        match = pat.match(line)
        if match:
            fields[match.group(1)] = match.group(2)
        else:
            if len(list) == 0:
                print >> sys.stderr, "Actual longer than correct"
                return False
            check = list.pop(0)
            if line != check[0]:
                print >> sys.stderr, "Mismatched lines:"
                print >> sys.stderr, "Actual: " + line + "Correct: " + check[0]
                return False
            if fields != check[1]:
                print >> sys.stderr, "Mismatched fields:"
                print >> sys.stderr, "Actual: " + line + repr(fields)
                print >> sys.stderr, "Correct: " + check[0] + repr(check[1])
                return False
                         
            fields = {}
    act.close()

    if len(fields) != 0:
        if len(list) == 0:
            print >> sys.stderr, "Actual longer than correct"
            return False
        check = list.pop(0)
        if check[0] is not None:
            print >> sys.stderr, "Correct longer than actual"
            return False
        if fields != check[1]:
            print >> sys.stderr, "Mismatched fields:"
            print >> sys.stderr, "Actual:"
            print >> sys.stderr, repr(fields)
            print >> sys.stderr, "Correct:"
            print >> sys.stderr, repr(check[1])
            return False

    if len(list) != 0:
        print >> sys.stderr, "Correct longer than actual"
        return False

    return True


def checkOutput(prog, outfile, canon, showdiff=True):
    """run the test program and compare its (text) output to the canonical 
    results found in a given file.  If the generated output is the same,
    return True; otherwise, return False.  Any differences are written to
    standard out when showdiff=True.
    @param prog       the program to run
    @param outfile    the output file to write results to
    @param canon      a file containing the "correct" results
    @param showdiff   if True (the default), print differences to stdout
    """
    if os.path.exists(outfile):
        os.remove(outfile)
    okay = captureOutput(prog, outfile)
    same = False
    if os.path.exists(outfile):
        same = compareOutput(outfile, canon, showdiff)

    return okay and same

def test(prog, canon_suffix="_correct.txt", test_suffix="_results.txt"):
    outdir = os.path.dirname(sys.argv[0])
    if len(outdir) > 0:  prog = os.path.join(outdir, prog)
    outfile = prog + test_suffix
    canon = prog + canon_suffix
    
    return checkOutput(prog, outfile, canon)


