// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
// Originally created in 2025
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>

#include "image.hpp"

namespace font {
    
    typedef struct {
        uint16_t   bitmapOffset;    // Offset address into the bitmap data.
        uint8_t    width, height;   // Bitmap dimensions in pixels.
        uint8_t    xAdvance;        // Distance to advance cursor in the x-axis.
        int8_t     dX;              // Used to position the glyph within the cell in the horizontal direction.
        int8_t     dY;              // Distance from the baseline of the character to the top of the glyph.
    } TGlyph;

    typedef struct {
        uint8_t   *bitmap;          // Glyph bitmaps, concatenated.
        TGlyph    *glyph;           // Glyph array.
        uint8_t    first;           // The first ASCII value of your first character.
        uint8_t    last;            // The last ASCII value of your last character.
        uint8_t    yAdvance;        // Newline distance in the y-axis.
    } TFont;
    
    

    template <typename T >
    int glyph(int16_t x, int16_t y, uint8_t c, T color, TFont &font, image::TBitmap &image)
    {
        if (c < font.first || c > font.last) {
            return 0;
        }
        
        uint8_t *dst = image.bytes.data();
        TGlyph *glyph = &font.glyph[(int)c - font.first];
        
        int height = glyph->height;
        int width = glyph->width;
       
        x += glyph->dX;
        y += glyph->dY + font.yAdvance;
        
        uint8_t *bitmap = font.bitmap + glyph->bitmapOffset;
        uint8_t bitPosition = 1 << 7;
        while (height--) {
            for (int xx=0; xx<width; xx++) {
                if (!bitPosition) {
                    bitPosition = 1 << 7;
                    bitmap++;
                }
                if (*bitmap & bitPosition) {
                    dst[x + xx + y * image.width * sizeof(T)] = color;
                }
                bitPosition >>= 1;
            }
            y++;
        }
        return glyph->xAdvance;
    }

    template <typename T>
    int print(int16_t x, int16_t y, const char *s, T color, TFont &font, image::TBitmap &image)
    {
        uint8_t *c = (uint8_t *)s;
        
        while (*c) {
            x += glyph(x, y, (uint8_t)*c, color, font, image);
            c++;
        }
        return x;
    }
}

