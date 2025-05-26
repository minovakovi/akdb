#!/bin/bash
##@file updateVersion.sh
##
##Updating project version
##
VER="$(cat projectDetails.h | awk -F '"|"' '{print $2}')"
sed -i "41cPROJECT_NUMBER         = ${VER}" doxygen.config