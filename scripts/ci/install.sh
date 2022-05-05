#!/usr/bin/env bash

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$0: Error: Please upgrade Bash."; exit 1; }

set -euxo pipefail

if [ -z ${SNEEDACITY_BUILD_TYPE+x} ]; then
SNEEDACITY_BUILD_TYPE="debug"
fi

# Install Sneedacity
cmake --install build --config "${SNEEDACITY_BUILD_TYPE}" --verbose
