#!/usr/bin/env bash
set -e

host_build_dir=$1
target_ssh_host=$2
target_work_dir=$3
target_app_path=$4
target_gdbserver=$5

tty_file="$host_build_dir/target.tty"

target_tty=$(cat "$tty_file")
exec ssh -T "$target_ssh_host" \
    "cd '$target_work_dir' && exec '$target_gdbserver' --once --wrapper /bin/sh -c '\"exec $target_app_path < $target_tty > $target_tty 2>&1\"' -- stdio '$target_app_path'"
