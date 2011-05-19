#!/bin/bash -x

NAME=$1
VERSION=$2
INSTALL=$3
EXTRAS=$4

rm -rf ${INSTALL}
mkdir -p ${INSTALL}/ups
cp ${EXTRAS}/${NAME}.table ${INSTALL}/ups/
setup -v -r ${INSTALL}

./bootstrap.sh --prefix=${INSTALL}
cp ${EXTRAS}/boost-mpi-config.jam tools/build/v2/user-config.jam
cat ${EXTRAS}/darwin.jam >> tools/build/v2/tools/darwin.jam

./bjam install
