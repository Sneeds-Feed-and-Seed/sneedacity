#!/usr/bin/env bash

echo "Begin environment.sh"

echo "environment.sh: Checking bash version"

if [[ "$0" == "${BASH_SOURCE}" ]]; then
    echo >&2 "$0: Please source this script instead of running it."
    exit 1
fi

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "${BASH_SOURCE}: Error: Please upgrade Bash."; return 1; }

function gh_export()
{
    echo "environment.sh: Begin gh_export()"
    [[ "${GITHUB_ENV-}" ]] || local -r GITHUB_ENV="/dev/null"
    export -- "$@" && printf "%s\n" "$@" >> "${GITHUB_ENV}"
    echo "environment.sh: returning from gh_export()"
}

repository_root="$(cd "$(dirname "${BASH_SOURCE}")/../.."; echo "${PWD}")"

echo "environment.sh: repository_root: "$repository_root


gh_export GIT_HASH="$(git show -s --format='%H')"
gh_export GIT_HASH_SHORT="$(git show -s --format='%h')"

gh_export SNEEDACITY_BUILD_TYPE="release"
echo "environment.sh: SNEEDACITY_BUILD_TYPE: "$SNEEDACITY_BUILD_TYPE

gh_export SNEEDACITY_INSTALL_PREFIX="${repository_root}/build/install"
echo "environment.sh: SNEEDACITY_INSTALL_PREFIX: "$SNEEDACITY_INSTALL_PREFIX

gh_export GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "environment.sh: GIT_BRANCH: " $GIT_BRANCH
