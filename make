#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

# Detect hardware architecture
ARCH=$(uname -m)

if [[ "$ARCH" == "x86_64" ]]; then
    make -j$(sysctl -n hw.ncpu) x86_64
else
    make -j$(sysctl -n hw.ncpu) arm64
fi

read -p "Press Enter to exit!"

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
