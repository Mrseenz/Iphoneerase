#!/bin/bash
# Arch Linux dependency installer
echo "Installing dependencies for Arch Linux..."
sudo pacman -Sy --noconfirm base-devel libimobiledevice libplist libusbmuxd pkgconf
echo "Dependencies installed!"
