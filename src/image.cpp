// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
// Originally created in 2023
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

#include "image.hpp"

#include <fstream>

#include "pbm.hpp"
#include "bmp.hpp"

image::TBitmap image::loadBitmap(const char *filename)
{
    TBitmap bitmap;
    
    auto bmp = bmp::load(filename);
    if (!bmp.bytes.empty()) {
        bitmap.width = bmp.width;
        bitmap.height = bmp.height;
        bitmap.bpp = bmp.bpp;
        bitmap.palette = bmp.palette;
        bitmap.bytes = bmp.bytes;
        return bitmap;
    }
    
    auto pbm = pbm::load(filename);
    if (!pbm.bytes.empty()) {
        bitmap.width = pbm.width;
        bitmap.height = pbm.height;
        bitmap.bpp = 1;
        bitmap.bytes = pbm.bytes;
        return bitmap;
    }
    
    return bitmap;
}

void image::saveBitmap(const char *filename, const image::TBitmap &bitmap)
{
    bmp::TImage image = {
        .width = bitmap.width,
        .height = bitmap.height,
        .bpp = bitmap.bpp,
        .palette = bitmap.palette,
        .bytes = bitmap.bytes
    };
    bmp::save(filename, image);
}

image::TBitmap image::createBitmap(int w, int h, uint8_t bpp)
{
    TBitmap bitmap = {
        .width = static_cast<uint16_t>(w),
        .height = static_cast<uint16_t>(h),
        .bpp = bpp
    };
    
    size_t length;
    
    length = w * h;
    if (bpp == 1) length /= 8;
    if (bpp == 16) length *= 2;
    
    bitmap.bytes.reserve(length);
    bitmap.bytes.resize(length);
    
    bitmap.palette.reserve(2);
    bitmap.palette.resize(2);
    
    bitmap.palette.at(1) = 0;
    bitmap.palette.at(0) = 0x00FFFFFF;

    return bitmap;
}

void image::copyBitmap(const TBitmap &dst, int dx, int dy, const TBitmap &src, int x, int y, uint16_t w, uint16_t h)
{
    uint8_t *d = (uint8_t *)dst.bytes.data();
    uint8_t *s = (uint8_t *)src.bytes.data();
    
    d += dx + dy * dst.width;
    s += x + y * src.width;
    while (h--) {
        for (int i=0; i<w; i++) {
            d[i] = s[i];
        }
        d += dst.width;
        s += src.width;
    }
}

image::TBitmap image::convertMonochromeToGrayScale(const TBitmap monochrome)
{
    TBitmap grayscale;
    
    if (monochrome.bpp != 1) return grayscale;

    grayscale.bpp = 8;
    grayscale.width = monochrome.width;
    grayscale.height = monochrome.height;
    size_t length = (size_t)monochrome.width * (size_t)monochrome.height;
    grayscale.bytes.reserve(length);
    grayscale.bytes.resize(length);
    
    uint8_t *src = (uint8_t *)monochrome.bytes.data();
    uint8_t *dest = (uint8_t *)grayscale.bytes.data();
    uint8_t bitPosition = 1 << 7;
    
    int x, y;
    for (y=0; y<monochrome.height; y++) {
        bitPosition = 1 << 7;
        for (x=0; x<monochrome.width; x++) {
            *dest++ = (*src & bitPosition ? 1 : 0);
            if (bitPosition == 1) {
                src++;
                bitPosition = 1 << 7;
            } else {
                bitPosition >>= 1;
            }
        }
        if (x & 7) src++;
    }
    
    return grayscale;
}



bool image::containsImage(const TBitmap &image, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (image.bytes.empty()) return false;
    if (x + w > image.width || y + h > image.height) return false;
    uint8_t *p = (uint8_t *)image.bytes.data();
    
    p += x + y * image.width;
    while (h--) {
        for (int i=0; i<w; i++) {
            if (p[i]) return true;
        }
        p+=image.width;
    }
    return false;
}

image::TBitmap image::extractImageSection(TBitmap &image)
{
    TBitmap extractedImage;
    
    int minX, maxX, minY, maxY;
    
    if (image.bytes.empty()) return extractedImage;
    
    uint8_t *p = (uint8_t *)image.bytes.data();
    
    maxX = 0;
    maxY = 0;
    minX = image.width - 1;
    minY = image.height - 1;
    
    for (int y=0; y<image.height; y++) {
        for (int x=0; x<image.width; x++) {
            if (!p[x + y * image.width]) continue;
            if (minX > x) minX = x;
            if (maxX < x) maxX = x;
            if (minY > y) minY = y;
            if (maxY < y) maxY = y;
        }
    }
    
    if (maxX < minX || maxY < minY)
        return extractedImage;
    
    
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    
    extractedImage = createBitmap(width, height, image.bpp);
    if (extractedImage.bytes.empty()) return extractedImage;
    copyBitmap(extractedImage, 0, 0, image, minX, minY, width, height);
    
    return extractedImage;
}
