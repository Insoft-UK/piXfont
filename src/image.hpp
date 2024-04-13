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

#ifndef image_hpp
#define image_hpp

#include <iostream>
#include <sstream>
#include <stdint.h>

typedef enum ImageType {
    ImageType_Bitmap,
    ImageType_Pixmap
} ImageType;

typedef struct __attribute__((__packed__)) {
    ImageType type;
    uint16_t width;
    uint16_t height;
    void *data;
} Image;

/**
 @brief Loads a file in the Portable Bitmap (PBM) format.
 @param filename The filename of the Portable Bitmap (PBM) to be loaded.
 @return A structure containing the image data.
 */
Image *loadPBGraphicFile(std::string &filename);

/**
 @brief Creates a bitmap with the specified dimensions.
 @param w The width of the bitmap.
 @param h The height of the bitmap.
 @return A structure containing the bitmap image data.
 */
Image *createBitmap(int w, int h);

/**
 @brief Creates a pixmap with the specified dimensions.
 @param w The width of the pixmap.
 @param h The height of the pixmap.
 @return A structure containing the pixmap image data.
 */
Image *createPixmap(int w, int h);

/**
 @brief Copies a section of a pixmap to another bitmap.
 @param dst The pixmap to which the section will be copied.
 @param dx The horizontal position where the copied section will be placed within the destination pixmap.
 @param dy The vertical axis position within the destination pixmap where the copied section will be placed.
 @param src The pixmap from which the section will be copied.
 @param x The horizontal position from which the pixmap will be copied.
 @param y The y-axis source of the pixmap to be copyed from.
 @param w The width of the pixmap to be copied.
 @param h The height of the pixmap to be copied.
 */
void copyPixmap(const Image *dst, int dx, int dy, const Image *src, int x, int y, uint16_t w, uint16_t h);

/**
 @brief Converts a monochrome bitmap to a pixmap, where each pixel is represented by a single byte.
 @param monochrome The monochrome bitmap to be converted to a pixmap bitmap.
 @return A structure containing the pixmap image data.
 */
Image *convertMonochromeBitmapToPixmap(const Image *monochrome);

/**
 @brief Frees the memory allocated for the image.
 @param image The image to be deallocated.
 */
void reset(Image *&image);

/**
 @brief Takes an input image and identifies and extracts a section of the image that contains an actual image.
 @param image The input image from which a section containing an actual image will be extracted.
 */
Image *extractImageSection(Image *image);

#endif /* image_hpp */
