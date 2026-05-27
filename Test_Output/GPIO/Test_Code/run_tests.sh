#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
make clean all
echo "Build complete. Link libgenerated_tests.a with your target firmware or test harness to run tests on hardware."
