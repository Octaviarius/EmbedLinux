#!/usr/bin/env bash
set -eu
find "$CROSS_SYSROOT" -type l -print0 |
while IFS= read -r -d '' link; do
    destination=$(readlink "$link")
    if [[ $destination == /* ]]; then
        relative=$(realpath -ms --relative-to="$(dirname "$link")" "$CROSS_SYSROOT$destination")
        ln -sfn "$relative" "$link"
    fi
done
