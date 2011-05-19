#!/bin/bash
#
# Note: this file is written as a shell script, but you'll need
# to manually download some files first in order to make it work.
# You may want to just copy the lines below each comment into
# a terminal to see what's going on.
#
# 0) Download tarballs for Boost (1.46.1), SCons (2.0.1), xpa (2.1.13),
#    Doxygen (1.7.4) and optionally IPython (0.10.2).
#    These should be put in bp/external/src, and the script below
#    will check if they exist (the expected tarball filenames are the
#    default filenames you get from downloading from the respective
#    project websites.
#    The script will skip any package it doesn't have a tarball for.
#
# 1) Install new versions of Tcl/Tk, Python, and Numpy:
#

lsstpkg install numpy 1.5.1+1

#
# 2) Fix the new Python install by making it require tcltk 8.5.9
#    exactly (the old tcltk, 8.5+a4, which is probably current,
#    is considered > 8.5.9, so ">= 8.5.9" doesn't work properly):
#

sed -ir -e 's/>= 8.5.9/== 8.5.9/' $LSST_PKGS/external/python/2.7.1/ups/python.table

#
# 3) Build a version of Boost that uses the new Python:
#

if [ -f external/src/boost_1_46_1.tar.gz ]; then
    external/build.py boost
else
    echo "Boost 1.46.1 tarball not found, skipping."
fi

#
# 4) Build a version of SCons(distrib) that uses the new Python:
#

if [ -f external/src/scons-2.0.1.tar.gz ]; then
    external/build.py scons
else
    echo "SCons 2.0.1 tarball not found, skipping."
fi

#
# 5) Build a version of XPA that uses the new TclTk:
#

if [ -f external/src/xpa-2.1.13.tar.gz ]; then
    external/build.py xpa
else
    echo "XPA 2.1.13 tarball not found."
fi

#
# 6) Build a version of PyFITS that uses the new Python/Numpy:
#

if [ -f external/src/pyfits-2.4.0.tar.gz ]; then
    external/build.py pyfits
else
    echo "PyFITS 2.4.0 tarball not found."
fi

#
# 7) Build the new Doxygen.
#

if [ -f external/src/doxygen-1.7.4.src.tar.gz ]; then
    external/build.py doxygen
else
    echo "Doxygen 1.7.4 tarball not found."
fi

#
# 8) (OPTIONAL) Build IPython.
#

if [ -f external/src/ipython-0.10.2.tar.gz ]; then
    external/build.py ipython
else
    echo "IPython 0.10.2 tarball not found."
fi
