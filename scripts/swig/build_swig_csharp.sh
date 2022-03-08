#!/bin/bash

PLATFORM="linux-gcc"
XPCF_VERSION=2.5.1
TARGET_LANG="csharp"
DESTFOLDER="swig-xpcf-cxx"
DESTSAMPLEFOLDER="swig-xpcf-sample-cxx"
INTERFACESFOLDER=../../interfaces
SAMPLE_INTERFACESFOLDER=../../samples/sample_component
SWIGXPCFFOlDER=${INTERFACESFOLDER}/swig
SWIGSAMPLEFOlDER=${SAMPLE_INTERFACESFOLDER}/swig

display_usage() { 
	echo "This script builds swig csharp for xpcf."
    echo "It can receive two optional arguments." 
	echo -e "\nUsage: \$0 [XPCF VERSION | default='${XPCF_VERSION}'] [generation destination folder | default='${DESTFOLDER}'] \n" 
} 

if [ ! -d ${TARGET_LANG}_android ]; then
    rm -r ${TARGET_LANG}_android
fi

if [ ! -d ${TARGET_LANG}_sample ]; then
    rm -r ${TARGET_LANG}_sample
fi

if [ ! -d ${TARGET_LANG} ]; then
    rm -r ${TARGET_LANG}
fi

if [ ! -d ${DESTFOLDER} ]; then
    rm -r ${DESTFOLDER}
fi

if [ ! -d ${DESTSAMPLEFOLDER} ]; then
    rm -r ${DESTSAMPLEFOLDER}
fi



# check whether user had supplied -h or --help . If yes display usage 
if [[ ( $1 == "--help") ||  $1 == "-h" ]] 
then 
    display_usage
    exit 0
fi 

if [ $# -ge 1 ]; then
	XPCF_VERSION=$1
fi
if [ $# -eq 2 ]; then
	DESTFOLDER=$2
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
# overload for mac values
	PLATFORM=mac-clang
fi

echo "Generate xpcf csharp interfaces with SWIG"

REMAKEN_XPCF_PKG_ROOT=~/.remaken/packages/${PLATFORM}/xpcf/${XPCF_VERSION}

OPTIONS="-c++ -${TARGET_LANG} -fcompact -O -I${SWIGXPCFFOlDER} -I${SWIGSAMPLEFOLDER} -I${SAMPLE_INTERFACESFOLDER} -I${INTERFACESFOLDER} -DXPCF_USE_BOOST -DSWIG_CSHARP_NO_WSTRING_HELPER"
generate_swig() {
local swigFilesFolder=$1
local targetLangDestFolder=$2
local sourceLangDestFolder=$3
local swigImportSymbol=$4
for swigFile in ${swigFilesFolder}/*.i ; do
   echo "--> parsing $swigFile"
   file_name=${swigFile##*/}
   file=${file_name%.*}
   outfolder=${targetLangDestFolder}/${file/_//}
   if [ ! -d $outfolder ]; then
      mkdir -p $outfolder
   fi
   if [ ! -d ${sourceLangDestFolder} ]; then
      mkdir -p ${sourceLangDestFolder}
   fi
   find "$outfolder" -name "*.*" -type f -delete
   echo "swig ${OPTIONS} -dllimport ${swigImportSymbol} -namespace ${file/_/.} -outdir ${outfolder} -o ${sourceLangDestFolder}/${file}_wrap.cxx $swigFile"
   swig ${OPTIONS} -dllimport ${swigImportSymbol} -namespace ${file/_/.} -outdir ${outfolder} -o ${sourceLangDestFolder}/${file}_wrap.cxx $swigFile
   echo "-----------------------------------------------"
done
}


generate_swig ${SWIGXPCFFOlDER} ${TARGET_LANG} ${DESTFOLDER} "swig_xpcf"
generate_swig ${SWIGSAMPLEFOlDER} ${TARGET_LANG} ${DESTFOLDER} "swig_xpcf"
generate_swig ${SWIGXPCFFOlDER} ${TARGET_LANG}-sample ${DESTSAMPLEFOLDER} "swig_xpcf_sample"
generate_swig ${SWIGSAMPLEFOlDER} ${TARGET_LANG}-sample ${DESTSAMPLEFOLDER} "swig_xpcf_sample"

if [ -d build-swig-xpcf-sample/shared ]; then
        rm -rf build-swig-xpcf-sample/shared
fi
mkdir -p build-swig-xpcf-sample/shared/debug
mkdir -p build-swig-xpcf-sample/shared/release
echo "===========> building XPCF shared <==========="
pushd build-swig-xpcf-sample/shared/debug
`${QMAKE_PATH}/qmake ../../../swig_xpcf_sample.pro -spec ${QMAKE_SPEC} CONFIG+=debug CONFIG+=x86_64 CONFIG+=qml_debug && /usr/bin/make qmake_all`
make
make install
popd
pushd build-xpcf/shared/release
`${QMAKE_PATH}/qmake ../../../swig_xpcf_sample.pro -spec ${QMAKE_SPEC} CONFIG+=x86_64 CONFIG+=qml_debug && /usr/bin/make qmake_all`
make
make install
popd

echo "------------------ Patch for Android support -----------------------------"
cp -r ${TARGET_LANG} ${TARGET_LANG}_android
files=`find ${TARGET_LANG}_android -name "*PINVOKE.cs"`
#Comment: MonoPInvokeCallback should be added to static method generated by SWIG while using IL2CPP
echo "--> Editing files for Android support"
for file in $files
do
    echo "----> " $file
    sed -i -e 's/static void SetPending/[AOT.MonoPInvokeCallback(typeof(ExceptionDelegate))] static void SetPending/g' $file
    sed -i -e 's/static string CreateString(string cString)/[AOT.MonoPInvokeCallback(typeof(SWIGStringDelegate))] static string CreateString(string cString)/g' $file
done

echo "-----------------------------------------------"

REMAKEN_TARGET_FOLDER=${REMAKEN_XPCF_PKG_ROOT}/${TARGET_LANG}

echo "------------------ Export to ${REMAKEN_TARGET_FOLDER} -----------------------------"
mkdir -p ${REMAKEN_TARGET_FOLDER}
if [ "$(ls -A ${REMAKEN_TARGET_FOLDER})" ]; then
    echo "Suppress csharp interfaces in ${REMAKEN_TARGET_FOLDER}/"
    rm -r ${REMAKEN_TARGET_FOLDER}/*
fi
cp -R ${TARGET_LANG}/* ${REMAKEN_TARGET_FOLDER}
echo "-----------------------------------------------"

echo "------------------ Export xpcf csharp files to test folder -----------------------------"
if [ -d test/xpcf ]; then
    echo "Suppress csharp interfaces in test/xpcf"
    rm -r test/xpcf
fi
cp -r ${TARGET_LANG}-sample/xpcf test
echo "-----------------------------------------------"

echo "------------------ Deploying shared libraries (swig wrapper, xpcf...) to C# binary folder -----------------------------"
~/.remaken/packages/mac-clang/remaken/1.9.2/bin/x86_64/static/debug/remaken.app/Contents/MacOS/remaken bundle -v -c debug --cpp-std 17 -d test/bin/Debug/net6.0/. --recurse test/packagedependencies.txt
echo "-----------------------------------------------"
