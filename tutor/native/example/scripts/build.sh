#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_HOST" \
    "cmake --build '$TARGET_BUILD_DIR' --parallel '$TARGET_JOBS'"
