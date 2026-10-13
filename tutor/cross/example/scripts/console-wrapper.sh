#!/bin/sh
target_tty=$1
shift

# Remaining arguments are the application command and its arguments.
exec "$@" < "$target_tty" > "$target_tty" 2>&1
