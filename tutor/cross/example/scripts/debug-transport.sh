#!/usr/bin/env bash
set -eu
ssh -T -o BatchMode=yes "$1" \
    "cd '$2' && exec '$5' --once --wrapper /bin/sh '$3/console-wrapper.sh' '$6' -- stdio '$3/$4'"
