#!/usr/bin/env bash
rsync -t --mkpath "$HOST_APP_PATH" "$TARGET_SSH_HOST:$TARGET_APP_PATH"
