#!/usr/bin/env bash

#USAGE:
#From build directory:
#configure.sh source_directory install_prefix

echo "Begin configure.sh"

echo "configure.sh: checking bash version"
((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$0: Error: Please upgrade Bash."; exit 1; }

set -euxo pipefail

echo "configure.sh: setting default parameters"
if [ -z ${SNEEDACITY_BUILD_TYPE+x} ]; then
SNEEDACITY_BUILD_TYPE="debug"
fi
if [ -z ${SNEEDACITY_INSTALL_PREFIX+x} ]; then
inst_pref_str="-D CMAKE_INSTALL_PREFIX=""$2"
else
inst_pref_str="-D CMAKE_INSTALL_PREFIX=""${SNEEDACITY_INSTALL_PREFIX}"
fi
#if [ -z ${project_dir+x}] and [ -z "$1" ]; then
#echo "configure.sh: setting project_dir to $1"
#project_dir="$1"
#else
#echo "configure.sh: setting project_dir to empty string"
#project_dir=""
#fi

echo "configure.sh: setting cmake_args"
cmake_args=(
    --trace
    --debug-output
    -Wdev
#    -S .
    -B build
#    -G "${SNEEDACITY_CMAKE_GENERATOR}"
    -D sneedacity_use_pch=no
    -D CMAKE_BUILD_TYPE="${SNEEDACITY_BUILD_TYPE}"
    "${inst_pref_str}"
    -G "Unix Makefiles"
    -S "${1:-.}"
)

echo "configure.sh: cmake_args: $cmake_args"
#echo "configure.sh: SNEEDACITY_CMAKE_GENERATOR: ${SNEEDACITY_CMAKE_GENERATOR}"

#if [[ "${SNEEDACITY_CMAKE_GENERATOR}" == "Visual Studio"* ]]; then
#    cmake_args+=(
#        # skip unneeded configurations
#        -D CMAKE_CONFIGURATION_TYPES="${SNEEDACITY_BUILD_TYPE}"
#    )
#    case "${SNEEDACITY_ARCH_LABEL}" in
#    32bit)  cmake_args+=( -A Win32 ) ;;
#    64bit)  cmake_args+=( -A x64 ) ;;
#    *)      echo >&2 "$0: Unrecognised arch label '${SNEEDACITY_ARCH_LABEL}'" ; exit 1 ;;
#    esac
#elif [[ "${SNEEDACITY_CMAKE_GENERATOR}" == Xcode* ]]; then
#    cmake_args+=(
#        # skip unneeded configurations
#        -D CMAKE_CONFIGURATION_TYPES="${SNEEDACITY_BUILD_TYPE}"
#        -T buildsystem=1
#    )
#fi

#if [[ -n "${APPLE_CODESIGN_IDENTITY}" && "${OSTYPE}" == darwin* ]]; then
#    cmake_args+=(
#        -D APPLE_CODESIGN_IDENTITY="${APPLE_CODESIGN_IDENTITY}"
#        -D sneedacity_perform_codesign=yes
#    )

#    if [[ ${GIT_BRANCH} == release* ]]; then
#        cmake_args+=(
#            -D APPLE_NOTARIZATION_USER_NAME="${APPLE_NOTARIZATION_USER_NAME}"
#            -D APPLE_NOTARIZATION_PASSWORD="${APPLE_NOTARIZATION_PASSWORD}"
#            -D sneedacity_perform_notarization=yes
#        )
#    fi
#echo "configure.sh: OSTYPE: ${OSTYPE}"
#elif [[ -n "${WINDOWS_CERTIFICATE}" && "${OSTYPE}" == msys* ]]; then
#    # Windows certificate will be used from the environment
#    cmake_args+=(
#        -D sneedacity_perform_codesign=yes
#    )
#fi

#echo "configure.sh: GIT_BRANCH: "${GIT_BRANCH}

#if [[ ${GIT_BRANCH} == release* ]]; then
#    cmake_args+=(
#        -D sneedacity_package_manual=yes
#    )
#fi

# Configure Sneedacity
echo "configure.sh: running cmake with args: ${cmake_args[@]}"

cmake "${cmake_args[@]}"
