#!/usr/bin/env bash
set -eu
ssh -t -o BatchMode=yes "$TARGET_HOST" \
    "cd '$TARGET_REMOTE_DIR' && exec '$TARGET_BUILD_DIR/$TARGET_PROGRAM'"
