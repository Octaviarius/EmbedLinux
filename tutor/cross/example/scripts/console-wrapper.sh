#!/bin/sh
terminal=$1
shift
exec "$@" < "$terminal" > "$terminal" 2>&1
