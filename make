#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

if [ ! -d "build" ]; then
    mkdir build
fi
make -j$(sysctl -n hw.ncpu) all

lipo -info build/arm64/pxfnt
lipo -info build/x86_64/pxfnt

# Code Signing...
echo "Code Signing... Please wait!"

IDENTITY=$(security find-identity -v -p codesigning | grep "Developer ID Application" | awk '{print $2}')
codesign -s "$IDENTITY" ./build/arm64/pxfnt
codesign -s "$IDENTITY" ./build/x86_64/pxfnt

zip build/pxfnt_arm64.zip build/arm64/pxfnt -x "*/.DS_Store"
zip build/pxfnt_x86_64.zip build/x86_64/pxfnt -x "*/.DS_Store"

read -p "DONE! Press Enter to close window."

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
