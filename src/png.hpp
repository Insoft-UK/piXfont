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


#ifndef png_hpp
#define png_hpp

#include <cstdint>
#include <vector>

namespace png {
    typedef struct {
        uint16_t width;
        uint16_t height;
        uint8_t  bpp;
        std::vector<uint8_t> bytes;
    } TImage;
    
    /**
     @brief    Loads a file in the Portable Network Graphics  (PNG) format.
     @param    filename The filename of the Portable Network Graphics  (PNG) to be loaded.
     @return   A structure containing the bitmap image data.
     */
    TImage load(const char *filename);
    
    /**
     @brief    Saves a file in the Portable Network Graphics  (PNG) format.
     @param    filename The filename of the Portable Network Graphics  (PNG) to be saved.
     @param    image A structure containing the bitmap image data.
     */
    bool save(const char *filename, const TImage &image);
}

#endif /* png_hpp */
