#!/usr/bin/env bash
set -eu
cmake --build "$HOST_BUILD_DIR" --parallel "$BUILD_JOBS"
