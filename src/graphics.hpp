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

#ifndef graphics_hpp
#define graphics_hpp

#include "image.hpp"

namespace graphics {
    template <typename T >
    void drawFillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, const T color, const image::TImage &image)
    {
        uint8_t *dst = (uint8_t *)image.bytes.data();
        while (h--) {
            if (y >= image.height) return;
            for (int i = 0; i < w; i++) {
                if (x + i >= image.width) break;
                dst[x + i + y * image.width * sizeof(T)] = color;
            }
            y++;
        }
    }
    
    template <typename T >
    void setPixel(int16_t x, int16_t y, const T color, const image::TImage &image)
    {
        if (x < 0 || y < 0) return;
        if (x >= image.width || y >= image.height) return;
        uint8_t *dst = (uint8_t *)image.bytes.data();
        dst[x + y * image.width * sizeof(T)] = color;
    }
}

#endif /* graphics_hpp */
