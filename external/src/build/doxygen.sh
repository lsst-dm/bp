#!/bin/bash -x

NAME=$1
VERSION=$2
INSTALL=$3
EXTRAS=$4

rm -rf ${INSTALL}
mkdir -p ${INSTALL}/ups
cp ${EXTRAS}/${NAME}.table ${INSTALL}/ups/
setup -v -r ${INSTALL}

./configure --prefix ${INSTALL}
make
make install
