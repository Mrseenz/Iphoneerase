#!/bin/bash
# Debian/Ubuntu dependency installer
echo "Installing dependencies for Debian/Ubuntu..."
sudo apt update
sudo apt install -y build-essential libimobiledevice-dev libplist-dev libusbmuxd-dev pkg-config
echo "Dependencies installed!"
