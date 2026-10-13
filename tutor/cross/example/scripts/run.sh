#!/usr/bin/env bash
set -eu
ssh -t -o BatchMode=yes "$TARGET_SSH_HOST" \
    "cd '$TARGET_WORK_DIR' && exec '$TARGET_APP_PATH'"
