#!/bin/bash
# macOS dependency installer
echo "Installing dependencies for macOS..."
brew update
brew install libimobiledevice libplist pkg-config
echo "Dependencies installed!"
