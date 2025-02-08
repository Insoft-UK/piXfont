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

#include "graphics.hpp"

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
        uint8_t   *indices;         // Indices of glyphs.
        uint16_t   first;           // The first ASCII value of your first character.
        uint16_t   last;            // The last ASCII value of your last character.
        uint8_t    yAdvance;        // Newline distance in the y-axis.
    } TFont;
    
    /*
     The ASCII table stores the index of each glyph within the glyph table, indicating where the glyph entry
     for a particular ASCII character resides. The table does not include entries for the first and last glyph
     indices, as they are unnecessary: the first glyph index always corresponds to the first glyph entry, and
     the last glyph in the font is always the last glyph entry.
     */
    
    typedef struct {
        std::vector<uint8_t> data;
        std::vector<TGlyph> glyphs;
        uint16_t first;
        uint16_t last;
        uint8_t yAdvance;
    } TAdafruitFont;
    
    /**
     @brief Draw a single character
     @param x Bottom left corner x coordinate
     @param y Bottom left corner y coordinate
     @param c The 8-bit font-indexed character (likely ascii)
     @param color Color to draw chraracter with
     @param sizeX Font magnification level in X-axis, 1 is 'original' size
     @param sizeY Font magnification level in Y-axis, 1 is 'original' size
     */
    template <typename T >
    static void drawChar(int16_t x, int16_t y, unsigned char c, const T color, uint8_t sizeX, uint8_t sizeY, const TFont &font, const image::TImage &image)
    {
        const TGlyph *glyph = &font.glyph[c - font.first];
        
        int h = glyph->height;
        int w = glyph->width;
        
        x += glyph->dX;
        y += (glyph->dY + font.yAdvance) * sizeY;
        
        uint8_t *bitmap = font.bitmap + glyph->bitmapOffset;
        int bitPosition = 0;
        uint8_t bits = 0;
        
        while (h--) {
            for (int xx = 0; xx < w; xx++) {
                if (!(bitPosition++ & 7)) {
                    bits = *bitmap++;
                }
                if (bits & 0x80) {
                    if (sizeX == 1 && sizeY == 1) {
                        graphics::setPixel(x + xx, y, color, image);
                    } else {
                        graphics::drawFillRect(x + xx * sizeX, y, sizeX, sizeY, color, image);
                    }
                }
                bits <<= 1;
            }
            y+=sizeY;
        }
    }

    template <typename T >
    int drawGlyph(int16_t x, int16_t y, const uint8_t asciiCode, const T color, const TFont &font, const image::TImage &image)
    {
        if (asciiCode < font.first || asciiCode > font.last) return 0;
        const TGlyph *glyph = &font.glyph[(int)asciiCode - font.first];
        drawChar(x, y, asciiCode, color, 1, 1, font, image);
        return glyph->xAdvance;
    }

    template <typename T>
    int print(int16_t x, int16_t y, const char *s, const T color, const TFont &font, const image::TImage &image)
    {
        uint8_t *c = (uint8_t *)s;
        uint8_t asciiCode;
        while (*c) {
            asciiCode = *c++;
            if (asciiCode < font.first || asciiCode > font.last) {
                continue;
            }
            x += drawGlyph(x, y, asciiCode, color, font, image);
        }
        return x;
    }
}

