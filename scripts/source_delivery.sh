#!/bin/bash
set -e

BUILDDIR="${BUILDDIR:-build}"
WORKDIR=${BUILDDIR}/source-delivery
SOURCEDIR=${WORKDIR}/source
VERSIONNAME=$(make version)
echo ${VERSIONNAME}

# (Re)create source delivery dir
rm -rf ${WORKDIR}
mkdir -p ${SOURCEDIR}

# Base directory
cp README.md ${SOURCEDIR}
cp LICENSE ${SOURCEDIR}
cp makefile ${SOURCEDIR}

# include
cp -rf include ${SOURCEDIR}

# src
cp -rf src ${SOURCEDIR}

# fake git.mk
mkdir -p ${SOURCEDIR}/make
echo "VERSION:=${VERSIONNAME}" > ${SOURCEDIR}/make/git.mk

# create an archive
pushd ${SOURCEDIR}
tar czvf ../eviewitf-source-delivery-${VERSIONNAME}.tgz *
popd
