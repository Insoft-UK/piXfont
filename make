#!/bin/bash
DIR=$(dirname "$0")
cd $DIR
clear

make

# Close the Terminal window
osascript -e 'tell application "Terminal" to close window 1' & exit
