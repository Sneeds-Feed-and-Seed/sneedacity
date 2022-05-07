if [ "$#" -ne 1 ]; then
	echo "Usage:"
	echo "configure.sh <install_directory>"
	exit
fi
export SNEEDACITY_BUILD_TYPE="release"
export SNEEDACITY_INSTALL_PREFIX="$1"
script_path=$(dirname `which $0`)
export project_dir="$script_path"/../../
"$project_dir""/scripts/ci/configure.sh"
