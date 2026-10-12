#!/usr/bin/env bash
set -eu
rsync -rltz --protect-args --delete --exclude=/project/.git/ \
    -e 'ssh -o BatchMode=yes' project "$TARGET_HOST:$TARGET_REMOTE_DIR/"
