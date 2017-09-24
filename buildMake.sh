#!/bin/bash
# Script for building clibdpa on Linux machine

set -e
project=clibdpa

#expected build dir structure
buildexp=build/Unix_Makefiles

currentdir=$PWD
builddir=./${buildexp}

mkdir -p ${builddir}

#debug
if [ ! -z $1 ]
then
# user selected
        if [ $1 == "Debug" ]
        then
                debug=-DCMAKE_BUILD_TYPE=$1
        else
                debug=-DCMAKE_BUILD_TYPE="Debug"
        fi
else
# release by default
        debug=""
fi
echo ${debug}

#get path to clibcdc libs
clibcdc=../clibcdc/${buildexp}
pushd ${clibcdc}
clibcdc=$PWD
popd

#get path to clibspi libs
clibspi=../clibspi/${buildexp}
pushd ${clibspi}
clibspi=$PWD
popd

#get path to cutils libs
cutils=../cutils/${buildexp}
pushd ${cutils}
cutils=$PWD
popd

#launch cmake to generate build environment
pushd ${builddir}
cmake -G "Unix Makefiles" -Dclibcdc_DIR:PATH=${clibcdc} -Dclibspi_DIR:PATH=${clibspi} -Dcutils_DIR:PATH=${cutils} ${currentdir} ${debug}
popd

#build from generated build environment
cmake --build ${builddir}
