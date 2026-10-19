#!/usr/bin/env bash

set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_directory="${repository_root}/build/desktop"

if ! command -v cmake >/dev/null 2>&1; then
    echo "Error: CMake 3.22 or newer is required." >&2
    exit 1
fi

cmake -S "${repository_root}" -B "${build_directory}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_directory}" --parallel
exec "${build_directory}/ArenaClash"
