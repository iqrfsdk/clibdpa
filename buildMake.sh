project=clibdpa

#expected build dir structure
buildexp=build/Unix_Makefiles

currentdir=$PWD
builddir=./${buildexp}

mkdir -p ${builddir}

#get path to clicdc libs
clibcdc=../clibcdc/${buildexp}
pushd ${clibcdc}
clibcdc=$PWD
popd

#launch cmake to generate build environment
pushd ${builddir}
cmake -G "Unix Makefiles" -Dclibcdc_DIR:PATH=${clibcdc} ${currentdir}
popd

#build from generated build environment
cmake --build ${builddir}

