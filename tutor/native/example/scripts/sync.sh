#!/usr/bin/env bash
set -eu
rsync -rltz --protect-args --delete --exclude=/project/.git/ \
    -e 'ssh -o BatchMode=yes' project "$TARGET_SSH_HOST:$TARGET_WORK_DIR/"
