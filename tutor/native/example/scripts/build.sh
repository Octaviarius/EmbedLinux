#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_HOST" \
    "cmake --build '$TARGET_REMOTE_DIR/build' --parallel '$TARGET_JOBS'"
