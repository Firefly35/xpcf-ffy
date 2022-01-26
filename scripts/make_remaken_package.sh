#!/bin/bash
#!/bin/bash


display_usage() { 
	echo "This script compress a remaken built package in order to deploy the compressed result on a remote."
    echo "It expects two arguments." 
	echo -e "\nUsage: \$0 [remaken package name] [remaken package version] \n" 
} 

if [  $# -lt 2 ] 
then 
    display_usage
    exit 1
fi 

# check whether user had supplied -h or --help . If yes display usage 
if [[ ( $1 == "--help") ||  $1 == "-h" ]] 
then 
    display_usage
    exit 0
fi 

PKGNAME=$1
PKGVERSION=$2

REMAKEN_PLATFORM=linux-gcc
if [[ "$OSTYPE" == "darwin"* ]]; then
# overload for mac values
	REMAKEN_PLATFORM=mac-clang
fi

BUILDMODES="debug release"
LINKMODES="static shared"
for linkMode in ${LINKMODES}
do
for buildMode in ${BUILDMODES}
do
BUILDFOLDER=$HOME/.remaken/packages/${REMAKEN_PLATFORM}/${linkMode}/${buildMode}
if [ -d ${BUILDFOLDER}/${PKGNAME} ]
then
echo "Packaging ${PKGNAME}-${PKGVERSION} from folder ${BUILDFOLDER}/${PKGNAME}"
pushd ${BUILDFOLDER}
zip -r -y ${PKGNAME}_${PKGVERSION}_x86_64_${linkMode}_${buildMode}.zip ${PKGNAME}
popd
fi
done
done
