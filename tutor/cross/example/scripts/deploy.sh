#!/usr/bin/env bash
set -eu
rsync -rltz --protect-args -e 'ssh -o BatchMode=yes' \
    "$CROSS_BUILD_DIR/$TARGET_PROGRAM" scripts/console-wrapper.sh \
    "$TARGET_HOST:$TARGET_BUILD_DIR/"
