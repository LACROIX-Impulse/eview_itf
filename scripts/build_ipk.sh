#!/bin/bash
set -e

BUILDDIR="${BUILDDIR:-build}"
IPKDIR=${BUILDDIR}/ipk

# (Re)create workin dirs
rm -rf ${IPKDIR}
mkdir -p ${IPKDIR}/{control,data}
VERSIONNAME=$1

# Create debian-binary file
echo 2.0 > ${IPKDIR}/debian-binary

# Create control file
echo Package: eViewItf > ${IPKDIR}/control/control
echo Version: ${VERSIONNAME} >> ${IPKDIR}/control/control
echo Architecture: ecube >> ${IPKDIR}/control/control
echo Maintainer: LACROIX Impulse >> ${IPKDIR}/control/control
echo Description: This package contains the eViewItf libraries and binaries >> ${IPKDIR}/control/control
echo Depends: kernel-module-eviewitf-mfis >> ${IPKDIR}/control/control

# Populate data
make install DESTDIR=${IPKDIR}/data

# Build ipk (archive)
pushd ${IPKDIR}/control
tar --numeric-owner --group=0 --owner=0 -czf ../control.tar.gz ./*
popd

pushd ${IPKDIR}/data
tar --numeric-owner --group=0 --owner=0 -czf ../data.tar.gz ./*
popd

pushd ${IPKDIR}
tar --numeric-owner --group=0 --owner=0 -cf ../eviewitf-${VERSIONNAME}.ipk ./debian-binary ./data.tar.gz ./control.tar.gz 
popd