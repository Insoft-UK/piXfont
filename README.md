# Adafruit GFX Pixel Font Creator
This is a handy utility that goes well with the original [fontconvert](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/fontconvert) tool provided by [Adafruit](https://www.adafruit.com/) for converting TTF fonts to Adafruit_GFX `.h` format.

This utility tool provides the ability to generate an Adafruit_GFX `.h` format from an image file.

## Fonts

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/HD44780.bmp">

### [HD44780](https://github.com/Insoft-UK/piXfont/blob/main/examples/HD44780.h)

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/BBC.bmp" >

### [BBC](https://github.com/Insoft-UK/piXfont/blob/main/examples/BBC.h)

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/CGA.bmp" >

### [IBM CGA CP437 8x8](https://github.com/Insoft-UK/piXfont/blob/main/examples/CGA.h)

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/EGA.bmp" >

### [IBM EGA CP437 8x14](https://github.com/Insoft-UK/piXfont/blob/main/examples/EGA.h)

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/VGA.bmp" >

### [IBM VGA CP437 9x16](https://github.com/Insoft-UK/piXfont/blob/main/examples/VGA.h)

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/ARCADE.bmp" >

### [ARCADE](https://github.com/Insoft-UK/piXfont/blob/main/examples/ARCADE.h)

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/Arial.bmp?" >

### [Arial (v0.1)](https://github.com/Insoft-UK/piXfont/blob/main/examples/subpixel/Arial.h)
Sub-Pixel Atlas

e.g.
### HD44780
<img src="https://github.com/Insoft-UK/piXfont/blob/main/examples/HD44780.bmp">

```
pxfnt HD44780.bmp -o HD44780.h -f 32 -l 255 -w 5 -h 10 -F -i 3 -g v -H 1 -V 1 -x 1 -y 1
```
Glyphs range from ASCII code 32 `-f 32` to ASCII code 255 `-l 255`.</br>
Each glyph has a maximum width of 5 pixels `-w 5` and a height of 10 pixels `-h 10`, with a fixed width of 10 pixels `-F`.</br>
Color index 3 `-i 3` is used for pixel data, with a default value of 1.</br>
Glyphs in the image are arranged top to bottom, left to right `-g v`.</br>
Glyphs are spaced 1 pixel apart horizontally `-H 1` and 1 pixel apart vertically `-V 1`.</br>
The glyphs begin 1 pixel from the left `-x 1` and 1 pixel from the top `-y 1` of the image.</br>

> [!NOTE]
The only image file format currently supported by this utility tool is the Portable Network Graphic (**8-Bit PNG**), Portable Bitmap (**PBM**) P4 and Bitmap (**BMP**) format.
