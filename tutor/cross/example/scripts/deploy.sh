#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_SSH_HOST" "mkdir -p '$(dirname "$TARGET_APP_PATH")'"
rsync -rltz --protect-args -e 'ssh -o BatchMode=yes' \
    "$HOST_APP_PATH" "$TARGET_SSH_HOST:$TARGET_APP_PATH"
rsync -rltz --protect-args -e 'ssh -o BatchMode=yes' \
    scripts/console-wrapper.sh "$TARGET_SSH_HOST:$TARGET_WORK_DIR/"
