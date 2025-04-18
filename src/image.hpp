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


#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>

namespace image {
    typedef struct {
        uint16_t width;
        uint16_t height;
        uint8_t  bpp;
        std::vector<uint32_t> palette;
        std::vector<uint8_t> bytes;
    } TImage;
    
    enum BPP : uint8_t {
        TrueColor = 32,
        HighColor = 16,
        Index256Colors = 8,
        Index16Colors = 4,
        Index4Colors = 2,
        Monochrome = 1
    };
    
    /**
     @brief    Loads a file in the Bitmap (BMP) or Portable Bitmap (PBM)  format.
     @param    filename The filename of the Bitmap (BMP) or Portable Bitmap (PBM)  to be loaded.
     @return   A structure containing the bitmap image data.
     */
    TImage loadImage(const char *filename);
    
    /**
     @brief    Saves a file in the Bitmap (BMP) format.
     @param    filename The filename of the Bitmap (BMP)  to be saved.
     @param    image A structure containing the bitmap image data.
     */
    bool saveImage(const char *filename, const TImage &image);
    
    void setPaletteToRGB332(image::TImage &image);
    
    /**
     @brief Creates a bitmap with the specified dimensions.
     @param w The width of the bitmap.
     @param h The height of the bitmap.
     @return A structure containing the bitmap image data.
     */
    TImage createImage(int w, int h, uint8_t bpp);
    
    /**
     @brief Copies a section of a bitmap to another bitmap.
     @param dst The bitmap to which the section will be copied.
     @param dx The horizontal position where the copied section will be placed within the destination bitmap.
     @param dy The vertical axis position within the destination bitmap where the copied section will be placed.
     @param src The bitmap from which the section will be copied.
     @param x The x-axis from which the bitmap will be copied.
     @param y The y-axis source of the bitmap to be copyed from.
     @param w The width of the bitmap to be copied.
     @param h The height of the bitmap to be copied.
     */
    void copyImage(const TImage &dst, int dx, int dy, const TImage &src, int x, int y, uint16_t w, uint16_t h);
    
    /**
     @brief Converts a monochrome image to an 8-bit indexed image.
     
     This function reads each bit from the input monochrome image data and expands it
     into a full byte in the output buffer, where a bit value of 0 becomes index 0,
     and a bit value of 1 becomes index 1. The result is an 8-bit-per-pixel indexed
     image suitable for further processing or palette mapping.
     
     @param monochrome The monochrome image to be converted.
     */
    void convertMonochromeToIndexed(TImage &monochrome);
    
    /**
     @brief Converts a monochrome bitmap to a pixmap, where each pixel is represented by a single byte.
     @param monochrome The monochrome bitmap to be converted to a pixmap bitmap.
     @return A structure containing the bitmap image data.
     */
    TImage convert16ColorTo256Color(const TImage &image);
    
    void convertIndexedToRGBA(TImage &image);
    
    /**
     @brief Takes an input image and identifies if the image contains an actual image at the specified section.
     @param image The input image that is to be inspected.
     @param x The start x-axis position from which the pixmap will be inspected at.
     @param y The start y-axis position from which the pixmap will be inspected at.
     @param w The width of the pixmap section to inspect.
     @param h The height of the pixmap section to inspect.
     */
    bool containsRegion(const TImage &image, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    
    /**
     @brief Takes an input image and identifies and extracts a section of the image that contains an actual image.
     @param image The input image from which a section containing an actual image will be extracted.
     */
    /**
     @brief Crops an image to its content by removing surrounding blank or empty space.
     
     This function scans the image for non-blank pixels (e.g., non-zero), determines the minimal
     bounding box that contains all such pixels, and creates a new cropped image containing only
     that region. It is useful for trimming padding or whitespace from images.
     
     @param image  The input image.
     @return A structure containing the bitmap image data.
     */
    TImage cropToContent(const TImage &image);
    
    /**
     @brief Takes an input image and inverts it.
     @param image The input image that will to inverte.
     */
    void invertImage(TImage &image);
    
    /**
     @brief Creates a binary-style mask of an indexed image, setting matching pixels to a specific value.
     
     This function scans through the indexed image data and replaces pixels that match
     the specified index with a user-defined value, while setting all other pixels to 0.
     This is useful for generating grayscale masks or alpha masks from indexed images.
     
     @param image  The input image (indexed color values).
     @param index  The index value to preserve; all others will be set to 0.
     @param value  The value to assign to matching pixels (e.g., 255 for full intensity).
     */
    void binarizeImageByIndexWithValue(TImage &image, uint8_t index, uint8_t value);

    /**
     @brief Binarizes an indexed image by setting matching index pixels to 1 and all others to 0.
     
     This function scans through the indexed image data and replaces pixels that match
     the specified index with 1, while setting all other pixels to 0. This effectively
     produces a binary mask highlighting the target index.
     
     @param image  The input image (indexed color values).
     @param index  The index value to preserve; all others will be set to 0.
     */
    void binarizeImageByIndex(TImage &image, uint8_t index);
    
    /**
     @brief Converts a subpixel-rendered grayscale image into a standard RGBA image.
     
     This function interprets the input image as a horizontally 3x-scaled subpixel mask,
     where each group of three grayscale pixels (0 or 255) represents a single RGB pixel.
     It reconstructs a standard-resolution 32-bit RGBA image by mapping each set of
     three subpixels (red, green, blue) to one output pixel, improving clarity on LCD screens.
     
     The original image is modified in-place with the resulting RGBA data.
     
     @param image  The input image containing subpixel-rendered mask data.
     */
    void convertSubpixel(TImage &image);
    
    void pasteImage(const TImage &image, const TImage &destImage, int x, int y);
}
