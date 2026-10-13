#!/usr/bin/env bash
set -eu
ssh -t -o BatchMode=yes "$TARGET_SSH_HOST" "tty; exec sleep infinity"
