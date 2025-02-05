# Adafruit GFX Pixel Font Creator
This is a handy utility that goes well with the original [fontconvert](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/fontconvert) tool provided by [Adafruit](https://www.adafruit.com/) for converting TTF fonts to Adafruit_GFX `.h` format.

This utility tool provides the ability to generate an Adafruit_GFX `.h` format from an image file.

**Fonts**

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/HD44780.bmp" >&nbsp;<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/BBC.bmp" >&nbsp;<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/C437.bmp" >&nbsp;<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/ARCADE.bmp" >
[HD44780](https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/HD44780.h)
[BBC](https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/BBC.h)
[C437](https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/C437.h)
[ARCADE](https://github.com/Insoft-UK/piXfont/blob/main/assets/fonts/ARCADE.h)

e.g.
### HD44780.h
<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/HD44780.png" width="20%" >

```
pxfnt HD44780.bmp -o HD44780.h -f 32 -l 255 -w 5 -h 10 -F -i 3 -g v -H 1 -V 1 -x 1 -y 1
```
Glyphs range from ASCII code 32 `-f 32` to ASCII code 255 `-l 255`.</br>
Each glyph has a maximum width of 5 pixels `-w 5` and a height of 10 pixels `-h 10`, with a fixed width of 10 pixels `-F`.</br>
Color index 3 `-i 3` is used for pixel data, with a default value of 1.</br>
Glyphs in the image are arranged top to bottom, left to right `-g v`.</br>
Glyphs are spaced 1 pixel apart horizontally `-H 1` and 1 pixel apart vertically `-V 1`.</br>
The glyphs begin 1 pixel from the left `-x 1` and 1 pixel from the top `-y 1` of the image.</br>

```
Copyright (C) 2024-2025 Insoft. All rights reserved.
Insoft Adafruit GFX Pixel Font Creator version, 1.0.6 (BUILD A0G01-25B2)

Usage: pxfnt <input-file> [-o <output-file>] -w <value> -h <value> [-c <columns>] [-n <name>] [-f <value>] [-l <value>] [-a] [-x <x-offset>] [-y <y-offset>] [-u <value>] [-g <h/v>] [-s <value>] [-H <value>] [-V <value>] [-F] [-ppl] [-i] [-v]

Options:
  -o <output-file>   Specify the filename for generated .bmp, .h or .hpprgm file.
  -w <value>         Maximum glyph width in pixels.
  -h <value>         Maximum glyph height in pixels.
  -c <columns>       Number of glyphs per column when generating a glyph image.
  -n <name>          Font name.
  -f <value>         First ASCII value of the first character.
  -l <value>         Last ASCII value of the last character.
  -a                 Auto left-align glyphs.
  -x <x-offset>      X-axis offset where glyphs start within the image file.
  -y <y-offset>      Y-axis offset where glyphs start within the image file.
  -u <value>         Cursor advance distance in the x-axis from the
                     right edge of the glyph (default: 1).
  -g <h/v>           Set the glyph layout direction, horizontal or vertical.
  -s <value>         Cursor advance distance in the x-axis for ASCII
                     character 32 (space), if not using fixed width.
  -H <value>         Horizontal spacing in pixels between glyphs.
  -V <value>         Vertical spacing in pixels between glyphs.
  -F                 Use fixed glyph width.
  -ppl               Generate PPL code (not required for .h files).
  -i <value>         The color index used to represent a pixel in a glyph when
                     using a non-monochrome image.
  -v                 Enable verbose output for detailed processing information.

Verbose Flags:
  f                  Font details.
  g                  Glyph details.

Additional Commands:
  pxfnt {--version | --help}
    --version        Display version information.
    --help           Show this help message.
```

```
pxfnt HD44780.h -o HD44780.hpprgm
```
The HP Prime stores its data as a list of 64-bit unsigned integers. The bitmap, however, is stored in a specific bit order (little-endian) where each byte of the 64-bit value is mirror-flipped.

e.g.
<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/Hart.png" width="128" >
```
01101100 #6Ch to 00110110 #36h
11111110 #FEh to 01111111 #7Fh
11111110 #FEh to 01111111 #7Fh
11111110 #FEh to 01111111 #7Fh
01111110 #7Eh to 01111110 #7Eh
00111000 #38h to 00011100 #1Ch
00010000 #10h to 00001000 #08h
00000000 #00h to 00000000 #00h := #00081C7E7F7F7F36:64h
```
> [!NOTE]
The only image file format currently supported by this utility tool is the Portable Bitmap (PBM) P4 and Bitmap (BMP) format.
