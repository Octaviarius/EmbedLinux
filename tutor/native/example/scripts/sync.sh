#!/usr/bin/env bash
set -eu
rsync -rltz --protect-args --delete --exclude=/.git/ \
    -e 'ssh -o BatchMode=yes' project/ "$TARGET_HOST:$TARGET_REMOTE_DIR/project/"
