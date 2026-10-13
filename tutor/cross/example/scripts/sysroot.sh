#!/usr/bin/env bash
set -eu
mkdir -p "$CROSS_SYSROOT/usr/lib" "$CROSS_SYSROOT/lib"
rsync -rlt --delete -e 'ssh -o BatchMode=yes' \
    "$TARGET_HOST:/usr/include/" "$CROSS_SYSROOT/usr/include/"
rsync -rlt --delete -e 'ssh -o BatchMode=yes' \
    "$TARGET_HOST:/usr/lib/$CROSS_TRIPLE/" "$CROSS_SYSROOT/usr/lib/$CROSS_TRIPLE/"
rsync -rlt --delete --link-dest="$CROSS_SYSROOT/usr/lib/$CROSS_TRIPLE" \
    -e 'ssh -o BatchMode=yes' "$TARGET_HOST:/lib/$CROSS_TRIPLE/" "$CROSS_SYSROOT/lib/$CROSS_TRIPLE/"
rsync -rlt -e 'ssh -o BatchMode=yes' \
    "$TARGET_HOST:$TARGET_LOADER" "$CROSS_SYSROOT$TARGET_LOADER"
