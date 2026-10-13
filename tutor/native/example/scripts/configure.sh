#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_SSH_HOST" \
    "cmake -S '$TARGET_WORK_DIR/project' -B '$TARGET_BUILD_DIR' -G Ninja -DCMAKE_BUILD_TYPE='$BUILD_TYPE' -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
