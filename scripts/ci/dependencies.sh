#!/usr/bin/env bash

((${BASH_VERSION%%.*} >= 4)) || echo >&2 "$0: Warning: Using ancient Bash version ${BASH_VERSION}."

set -euxo pipefail

if [[ "${OSTYPE}" == msys* ]]; then # Windows
    echo windows, no deps to install
elif [[ "${OSTYPE}" == darwin* ]]; then # macOS

    # Homebrew packages
    brew_packages=(
        bash # macOS ships with Bash v3 for licensing reasons so upgrade it now
    )
    brew install "${brew_packages[@]}"

else # Linux & others

    if ! which sudo; then
        function sudo() { "$@"; } # no-op sudo for use in Docker images
    fi

    # Distribution packages
    if which apt-get; then
        apt_packages=(
            # Docker image
            file
            g++
            git
            wget

            # GitHub Actions
            libasound2-dev
            libgtk2.0-dev
            gettext
            python3-pip
        )
        sudo apt-get update -y
        sudo apt-get install -y --no-install-recommends "${apt_packages[@]}"
        sudo apt-get remove -y ccache
    else
        echo >&2 "$0: Error: You don't have a recognized package manager installed."
        exit 1
    fi

    # Python packages
    pip_packages=(
        wheel
        setuptools
    )

    which cmake || pip_packages+=( cmake ) # get latest CMake when inside Docker image
    pip3 install "${pip_packages[@]}"

fi
