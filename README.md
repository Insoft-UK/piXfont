# Adafruit GFX Pixel Font Creator
This is a handy utility that goes well with the original [fontconvert](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/fontconvert) tool provided by [Adafruit](https://www.adafruit.com/) for converting TTF fonts to Adafruit_GFX `.h` format.

This utility tool provides the ability to generate an Adafruit_GFX `.h` format from an image file.

<img src="https://github.com/Insoft-UK/piXfont/blob/main/assets/HD44780.png" width="20%" >

```
pxfnt HD44780.bmp -f 32 -l 255 -w 5 -h 10 -x 1 -y 1 -H 1 -V 1 -F -g v -i 3
```



> [!NOTE]
The only image file format currently supported by this utility tool is the Portable Bitmap (PBM) P4 and Bitmap (BMP) format.
