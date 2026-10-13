#!/usr/bin/env bash
set -eu
ssh -o BatchMode=yes "$TARGET_SSH_HOST" "mkdir -p '$TARGET_WORK_DIR'"
