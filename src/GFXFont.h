/*
 The MIT License (MIT)

 Copyright (c) 2024 Insoft. All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#ifndef GFXFont_h
#define GFXFont_h
// 0x00fb, 2, 2, 2, -1, -7  w2 xA2 dX-1
typedef struct {
    uint16_t   bitmapOffset;    // Offset address into the bitmap data.
    uint8_t    width, height;   // Bitmap dimensions in pixels.
    uint8_t    xAdvance;        // Distance to advance cursor in the x-axis.
    int8_t     dX;              // Used to position the glyph within the cell in the horizontal direction.
    int8_t     dY;              // Distance from the baseline of the character to the top of the glyph.
} GFXglyph;

typedef struct {
    uint8_t   *bitmap;          // Glyph bitmaps, concatenated.
    GFXglyph  *glyph;           // Glyph array.
    uint8_t    first;           // The first ASCII value of your first character.
    uint8_t    last;            // The last ASCII value of your last character.
    uint8_t    yAdvance;        // Newline distance in the y-axis.
} GFXfont;

#endif /* GFXFont_h */
