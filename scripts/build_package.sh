#!/bin/bash
set -e

BUILDDIR="${BUILDDIR:-build}"
IPKDIR=${BUILDDIR}/ipk
PKGDIR=${BUILDDIR}/package

# (Re)create workin dirs
rm -rf ${PKGDIR}
mkdir -p ${PKGDIR}/{ipk,doc}
VERSIONNAME=$1

# Copy pdf
cp doc/user/latex/refman.pdf ${PKGDIR}/doc/eViewItf.pdf

# Copy HTML
pushd doc/user/
zip -r html.zip ./html
popd
cp doc/user/html.zip ${PKGDIR}/doc/

# Copy ipk
cp ${BUILDDIR}/eviewitf-${VERSIONNAME}.ipk ${PKGDIR}/ipk

# Create archive
pushd ${PKGDIR}
tar -cf ../eviewitf-${VERSIONNAME}.tar ./*
popd
