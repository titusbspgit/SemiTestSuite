#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
FILTER="${1:-}"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake ..
cmake --build .

if [[ -n "${FILTER}" ]]; then
    "${BUILD_DIR}/pcie_tests" "${FILTER}"
else
    "${BUILD_DIR}/pcie_tests"
fi
