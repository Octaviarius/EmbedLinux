#!/usr/bin/env bash
set -e

tty_file="$HOST_BUILD_DIR/target.tty"

trap 'rm -f "$tty_file"' EXIT
echo "Opening target console"
ssh -tt "$TARGET_SSH_HOST" "tty; exec sleep infinity" | {
    read -r target_tty
    printf '%s\n' "${target_tty%$'\r'}" > "$tty_file"
    echo "Target console ready"
    cat
}
