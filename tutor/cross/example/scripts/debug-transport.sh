#!/usr/bin/env bash
set -eu

target_ssh_host=$1
target_work_dir=$2
target_app_path=$3
target_gdbserver=$4
target_tty=$5

ssh -T -o BatchMode=yes "$target_ssh_host" \
    "cd '$target_work_dir' && exec '$target_gdbserver' --once --wrapper /bin/sh '$target_work_dir/console-wrapper.sh' '$target_tty' -- stdio '$target_app_path'"
