#!/usr/bin/env bash
set -e

tty_file="$HOST_BUILD_DIR/target.tty"

target_tty=$(cat "$tty_file")
exec ssh -T "$TARGET_SSH_HOST" \
    "cd '$TARGET_WORK_DIR' && exec '$TARGET_GDBSERVER' --once --wrapper /bin/sh -c '\"exec $TARGET_APP_PATH < $target_tty > $target_tty 2>&1\"' -- stdio '$TARGET_APP_PATH'"
