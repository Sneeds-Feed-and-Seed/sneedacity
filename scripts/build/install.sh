export SNEEDACITY_BUILD_TYPE="debug"
script_path=$(dirname `which $0`)
VERBOSE=1 $script_path"/../ci/install.sh"
