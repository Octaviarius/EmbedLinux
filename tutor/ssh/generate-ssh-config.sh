#!/usr/bin/env bash
target_ip=$1
config_path=${2:-"$HOME/.ssh/conf.d/pi-dev"}
mkdir -p "$(dirname "$config_path")"
umask 077
cat > "$config_path" <<EOF
Host pi-dev
    HostName $target_ip
    User dev
    IdentityFile ~/.ssh/rpi-dev
    IdentitiesOnly yes
    HostKeyAlias rpi0-2w
EOF
chmod 600 "$config_path"
