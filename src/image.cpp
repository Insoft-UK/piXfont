/*
 Copyright © 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "image.hpp"

#include <fstream>
Image *loadPBGraphicFile(std::string &filename)
{
    std::ifstream infile;
    
    Image *image = (Image *)malloc(sizeof(Image));
    if (!image) {
        return nullptr;
    }
    
    infile.open(filename, std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        free(image);
        return nullptr;
    }
    
    std::string s;
    
    getline(infile, s);
    if (s != "P4") {
        infile.close();
        return image;
    }
    
    image->type = ImageType_Bitmap;
    
    getline(infile, s);
    image->width = atoi(s.c_str());
    
    getline(infile, s);
    image->height = atoi(s.c_str());
    
    size_t length = ((image->width + 7) >> 3) * image->height;
    image->data = (unsigned char *)malloc(length);
    
    if (!image->data) {
        free(image);
        infile.close();
        return nullptr;
    }
    infile.read((char *)image->data, length);
    
    infile.close();
    return image;
}

Image *createBitmap(int w, int h)
{
    Image *image = (Image *)malloc(sizeof(Image));
    if (!image) {
        return nullptr;
    }
    
    w = (w + 7) & ~7;
    image->data = malloc(w * h / 8);
    if (!image->data) {
        free(image);
        return nullptr;
    }
    
    image->type = ImageType_Bitmap;
    image->width = w;
    image->height = h;
    
    return image;
}

Image *createPixmap(int w, int h)
{
    Image *image = (Image *)malloc(sizeof(Image));
    if (!image) {
        return nullptr;
    }
    
    image->data = malloc(w * h);
    if (!image->data) {
        free(image);
        return nullptr;
    }
    
    image->type = ImageType_Pixmap;
    image->width = w;
    image->height = h;
    
    return image;
}

void copyPixmap(const Image *dst, int dx, int dy, const Image *src, int x, int y, uint16_t w, uint16_t h)
{
    uint8_t *d = (uint8_t *)dst->data;
    uint8_t *s = (uint8_t *)src->data;
    
    d += dx + dy * dst->width;
    s += x + y * src->width;
    while (h--) {
        for (int i=0; i<w; i++) {
            d[i] = s[i];
        }
        d += dst->width;
        s += src->width;
    }
}

Image *convertMonochromeBitmapToPixmap(const Image *monochrome)
{
    Image *image = (Image *)malloc(sizeof(Image));
    if (!image)
        return nullptr;
    
    uint8_t *src = (uint8_t *)monochrome->data;
    uint8_t bitPosition = 1 << 7;
    
    image->type = ImageType_Pixmap;
    image->width = monochrome->width;
    image->height = monochrome->height;
    image->data = malloc(image->width * image->height);
    if (!image->data) return image;
    
    memset(image->data, 0, image->width * image->height);
    
    uint8_t *dest = (uint8_t *)image->data;
    
    int x, y;
    for (y=0; y<monochrome->height; y++) {
        bitPosition = 1 << 7;
        for (x=0; x<monochrome->width; x++) {
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
    
    return image;
}

void reset(Image *&image)
{
    if (image) {
        if (image->data) free(image->data);
        free(image);
        image = nullptr;
    }
}

Image *extractImageSection(Image *image)
{
    Image *extractedImage = nullptr;
    
    int minX, maxX, minY, maxY;
    
    if (!image || !image->data) return nullptr;
    
    uint8_t *p = (uint8_t *)image->data;
    
    maxX = 0;
    maxY = 0;
    minX = image->width - 1;
    minY = image->height - 1;
    
    for (int y=0; y<image->height; y++) {
        for (int x=0; x<image->width; x++) {
            if (!p[x + y * image->width]) continue;
            if (minX > x) minX = x;
            if (maxX < x) maxX = x;
            if (minY > y) minY = y;
            if (maxY < y) maxY = y;
        }
    }
    
    if (maxX < minX || maxY < minY)
        return nullptr;
    
    
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    
    extractedImage = createPixmap(width, height);
    if (!extractedImage) return nullptr;
    copyPixmap(extractedImage, 0, 0, image, minX, minY, width, height);
    
    return extractedImage;
}
