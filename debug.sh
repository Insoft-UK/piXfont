#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

if [ ! -d "examples/debug" ]; then
    mkdir examples/debug
fi

PATH=$DIR/build/arm64:$PATH

pxfnt examples/BBC.bmp -o examples/debug/C.h -f 32 -l 255 -w 8 -h 16 -y 16 -H 8 -F -u 0
read -p "DONE! Press Enter to continue..."
pxfnt examples/debug/C.h -o examples/debug/PPL.hpprgm
read -p "DONE! Press Enter to continue..."
pxfnt examples/debug/PPL.hpprgm -o examples/debug/BMP.bmp
read -p "DONE! Press Enter to close window."

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
