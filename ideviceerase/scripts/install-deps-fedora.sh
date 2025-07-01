#!/bin/bash
# Fedora dependency installer
echo "Installing dependencies for Fedora..."
sudo dnf install -y gcc make libimobiledevice-devel libplist-devel libusbmuxd-devel pkgconfig
echo "Dependencies installed!"
