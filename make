#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

if [ ! -d "build" ]; then
    mkdir build
fi
make -j$(sysctl -n hw.ncpu)
#strip build/pxfnt
lipo -info build/pxfnt

# Code Signing...
echo "Code Signing... Please wait!"

IDENTITY=$(security find-identity -v -p codesigning | grep "Developer ID Application" | awk '{print $2}')
codesign -s "$IDENTITY" ./build/*

read -p "DONE! Press Enter to close window."

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
