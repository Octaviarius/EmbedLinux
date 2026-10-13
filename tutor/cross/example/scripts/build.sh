#!/usr/bin/env bash
set -eu
cmake --build "$CROSS_BUILD_DIR" --parallel "$CROSS_JOBS"
