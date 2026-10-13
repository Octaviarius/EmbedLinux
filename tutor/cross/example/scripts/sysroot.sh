#!/usr/bin/env bash
set -eu
mkdir -p "$HOST_SYSROOT/usr/lib" "$HOST_SYSROOT/lib"
rsync -rlt --delete -e 'ssh -o BatchMode=yes' \
    "$TARGET_SSH_HOST:/usr/include/" "$HOST_SYSROOT/usr/include/"
rsync -rlt --delete -e 'ssh -o BatchMode=yes' \
    "$TARGET_SSH_HOST:/usr/lib/$TARGET_LIB_TRIPLE/" "$HOST_SYSROOT/usr/lib/$TARGET_LIB_TRIPLE/"
rsync -rlt --delete --link-dest="$HOST_SYSROOT/usr/lib/$TARGET_LIB_TRIPLE" \
    -e 'ssh -o BatchMode=yes' "$TARGET_SSH_HOST:/lib/$TARGET_LIB_TRIPLE/" "$HOST_SYSROOT/lib/$TARGET_LIB_TRIPLE/"
rsync -rlt -e 'ssh -o BatchMode=yes' \
    "$TARGET_SSH_HOST:$TARGET_LOADER" "$HOST_SYSROOT$TARGET_LOADER"
