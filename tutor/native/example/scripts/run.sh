#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_HOST" \
    "cd '$TARGET_REMOTE_DIR' && exec './build/$TARGET_PROGRAM'"
