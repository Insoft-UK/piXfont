# Adafruit GFX Pixel Font Creator
This is a handy utility that goes well with the original [fontconvert](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/fontconvert) tool provided by [Adafruit](https://www.adafruit.com/) for converting TTF fonts to Adafruit_GFX `.h` format.

This utility tool provides the ability to generate an Adafruit_GFX `.h` format from an image file.

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/HD44780.png" width="20%" >

```
pxfnt HD44780.bmp -f 32 -l 255 -w 5 -h 10 -x 1 -y 1 -H 1 -V 1 -F -g v -i 3
```
HD44780.h
Glyphs range from ASCII code 32 `-f 32` to ASCII code 255 `-l 255`.</br>
Each glyph has a maximum width of 5 pixels `-w 5` and a height of 10 pixels `-h 10`, with a fixed width of 10 pixels `-F`.</br>
Color index 3 `-i 3` is used for pixel data, with a default value of 1.</br>
Glyphs in the image are arranged top to bottom, left to right `-g v`.</br>
Glyphs are spaced 1 pixel apart horizontally `-H 1` and 1 pixel apart vertically `-V 1`.</br>
The glyphs begin 1 pixel from the left `-x 1` and 1 pixel from the top `-y 1` of the image.</br>


> [!NOTE]
The only image file format currently supported by this utility tool is the Portable Bitmap (PBM) P4 and Bitmap (BMP) format.
