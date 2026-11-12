#!/usr/bin/env bash
set -e
mkdir -p "$SDK_SYSROOT/usr/lib" "$SDK_SYSROOT/lib"
rsync -rlt --delete \
    "$TARGET_SSH_HOST:/usr/include/" "$SDK_SYSROOT/usr/include/"
rsync -rlt --delete \
    "$TARGET_SSH_HOST:/usr/lib/$SDK_TARGET_MULTIARCH/" "$SDK_SYSROOT/usr/lib/$SDK_TARGET_MULTIARCH/"
rsync -rlt --delete --link-dest="$SDK_SYSROOT/usr/lib/$SDK_TARGET_MULTIARCH" \
    "$TARGET_SSH_HOST:/lib/$SDK_TARGET_MULTIARCH/" "$SDK_SYSROOT/lib/$SDK_TARGET_MULTIARCH/"
rsync -rlt \
    "$TARGET_SSH_HOST:$SDK_TARGET_LOADER" "$SDK_SYSROOT$SDK_TARGET_LOADER"
