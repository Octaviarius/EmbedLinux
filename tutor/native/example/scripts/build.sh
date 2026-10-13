#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_SSH_HOST" \
    "cmake --build '$TARGET_BUILD_DIR' --parallel '$BUILD_JOBS'"
