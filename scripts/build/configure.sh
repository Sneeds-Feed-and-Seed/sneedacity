if [ "$#" -lt 1 ]; then
	echo "Usage:"
	echo "configure.sh <install directory> <build type>"
	exit
fi

export SNEEDACITY_INSTALL_PREFIX="$1"

export SNEEDACITY_BUILD_TYPE="${2:-release}"
echo "scripts/build/configure.sh Build type: $SNEEDACITY_BUILD_TYPE"

script_path=$(dirname `which $0`)
export project_dir="$script_path"/../../
"$project_dir""/scripts/ci/configure.sh"
