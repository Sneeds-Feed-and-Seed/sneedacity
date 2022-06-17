#!/usr/bin/env bash

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$0: Error: Please upgrade Bash."; exit 1; }

set -euxo pipefail

# Install Sneedacity
if [ -z ${SNEEDACITY_BUILD_TYPE+x} ]
then
  cmake --install build --verbose
else
 cmake --install build --verbose --config "${SNEEDACITY_BUILD_TYPE}"
fi
