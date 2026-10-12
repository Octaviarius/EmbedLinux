#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_HOST" \
    "cmake -S '$TARGET_REMOTE_DIR/project' -B '$TARGET_REMOTE_DIR/build' -G Ninja -DCMAKE_BUILD_TYPE='$TARGET_BUILD_TYPE' -DCMAKE_EXPORT_COMPILE_COMMANDS=ON '-DAPP_NAME=$TARGET_PROGRAM'"
