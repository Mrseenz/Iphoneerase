#!/bin/bash
# WSL (Debian-based) dependency installer
echo "Installing dependencies for WSL (Debian-based)..."
echo "This script assumes you are running a Debian-based distribution (e.g., Ubuntu) under WSL."

# Ensure the script is not run as root directly, but sudo will be used for commands
if [ "$EUID" -eq 0 ]; then
  echo "Please do not run this script as root. Sudo will be used for commands that require root privileges."
  exit 1
fi

sudo apt update
sudo apt install -y build-essential libimobiledevice-dev libplist-dev libusbmuxd-dev pkg-config

echo ""
echo "Dependencies installed!"
echo "Ensure your WSL distribution has USB device support correctly configured to connect to iOS devices."
echo "You might need to install usbipd-win and attach the device to WSL."
echo "Refer to Microsoft's WSL documentation for USB support for more details."
