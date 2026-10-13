#!/usr/bin/env bash
set -eu
ssh -t -o BatchMode=yes "$TARGET_HOST" "tty; exec sleep infinity"
