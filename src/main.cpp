/*
 Copyright (C) 2014 by Insoft
 
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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <regex>
#include <fstream>
#include <iomanip>

#include "GFXFont.h"
#include "image.hpp"

#include "build.h"

bool verbose = false;

enum class MessageType {
    Warning,
    Error,
    Verbose
};

std::ostream& operator<<(std::ostream& os, MessageType type) {
    switch (type) {
        case MessageType::Error:
            os << "\033[91;1;1;1merror\033[0m: ";
            break;

        case MessageType::Warning:
            os << "\033[98;1;1;1mwarning\033[0m: ";
            break;
            
        case MessageType::Verbose:
            os << ": ";
            break;

        default:
            os << ": ";
            break;
    }

    return os;
}

void usage(void)
{
    std::cout << "Copyright (c) 2023 Insoft. All rights reserved\n";
    std::cout << "Adafruit GFX Pixel Font Creator\n";
    std::cout << "Usage: pixfont filename -n name [-w width] [-h height]\n";
    std::cout << " -o, --out         file\n";
    
    // TODO: Add verbose
//    std::cout << " -v, --verbose     display detailed processing information\n";
//    std::cout << " verbose :- flags\n";
//    std::cout << "            f font\n";
//    std::cout << "            g glyphs\n\n";
    
    
    std::cout << " -a                auto left-align glyphs.\n";
    std::cout << " -d, --distance    distance to advance cursor in the x-axis from the right edge of the glyph, default is 1.\n";
    std::cout << " -f, --first       first ASCII value of your first character.\n";
    std::cout << " -l, --last        last ASCII value of your last character.\n";
    std::cout << " -n, --name        font name.\n";
    std::cout << " -w, --width       max width of the bitmap in pixels.\n";
    std::cout << " -h, --height      font height in pixels.\n";
    std::cout << " -hs,              horizontal spacing in pixels between each glyph.\n";
    std::cout << " -vs,              vertical spacing in pixels between each glyph.\n";
    std::cout << " -x, --fixed       fixed char width.\n";
    std::cout << "\n";
    std::cout << " --version         displays the full version number.\n";
}

void error(void)
{
    std::cout << "piXfont: try 'pixfont --help' for more information\n";
    exit(0);
}

void version(void) {
    std::cout
    << "piXfont v"
    << (unsigned)__BUILD_NUMBER / 100000 << "."
    << (unsigned)__BUILD_NUMBER / 10000 % 10 << "."
    << (unsigned)__BUILD_NUMBER / 1000 % 10 << "."
    << std::setfill('0') << std::setw(3) << (unsigned)__BUILD_NUMBER % 1000
    << "\n";
}

GFXglyph autoGFXglyphSettings(Image *image)
{
    GFXglyph gfxGlyph = {0, 0, 0, 0, 0, 0};
    int minX, maxX, minY, maxY;
    
    if (!image || !image->data) return gfxGlyph;
    
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
    
    if (maxX < minX || maxY < minY) {
        return gfxGlyph;
    }
    
    gfxGlyph.bitmapOffset = 0;
    gfxGlyph.width = maxX - minX + 1;
    gfxGlyph.height = maxY - minY + 1;
    gfxGlyph.xAdvance = minX + gfxGlyph.width;
    gfxGlyph.dX = minX;
    gfxGlyph.dY = -image->height + minY;
    
    return gfxGlyph;
}

void concatenateImageData(Image *image, std::vector<uint8_t> &data)
{
    uint8_t *p = (uint8_t *)image->data;
    uint8_t bitPosition = 1 << 7;
    uint8_t byte = 0;
    
    for (int i = 0; i < image->width * image->height; i++) {
        if(!bitPosition) bitPosition = 1 << 7;
        if (p[i])
            byte |= bitPosition;
        bitPosition >>= 1;
        if (!bitPosition) {
            data.push_back(byte);
            byte = 0;
        }
    }
    if (bitPosition) {
        data.push_back(byte);
    }
}

std::string addCharacter(uint16_t character, GFXglyph &glyph, GFXfont &font)
{
    std::ostringstream os;
    std::string s;
    
    os << "  { ";
    os << std::setw(5) << (int)glyph.bitmapOffset << "," << std::setw(4) << (int)glyph.width << "," << std::setw(4) << (int)glyph.height << "," << std::setw(4) << (int)glyph.xAdvance << "," << std::setw(4) << (int)glyph.dX << "," << std::setw(4) << (int)glyph.dY << " },   ";
    
    os << "// 0x" << std::setfill('0') << std::setw(2) << std::hex << (int)character << std::dec << " ";
    
    if (character < ' ') {
        os << "'none printable'\n";
    } else {
        os << "'" << (char)character << "'\n";
    }
    
    s = os.str();
    if (font.last == character)
        s = std::regex_replace(s, std::regex(R"(\},)"), "} ");
    
    return s;
}

void indexToXY(const int index, int &x, int &y, const int w, const int h, const int cw, const int ch)
{
    x = index % (w / cw) * cw;
    y = index / (w / cw) * ch;
}

int asciiExtents(const Image *pixmap, int width, GFXfont &font, int hs, int vs)
{
    if (!pixmap || !pixmap->data) return 0;
    int x,y;
    int prevFirst = (int)font.first;
    
    for (uint16_t n = 0; n < 256; n++) {
        indexToXY(n, x, y, pixmap->width, pixmap->height, width + hs, font.yAdvance + vs);
        if (containsImage(pixmap, x, y, width, font.yAdvance)) {
            if (font.first == 0) font.first = n;
            font.last = n;
        }
    }
    return (int)font.first - prevFirst;
}

void createFile(std::string &filename, std::ostringstream &os, std::string &name)
{
    std::ofstream outfile;
    std::string path;
    
    size_t pos = filename.rfind("/");
    path = filename.substr(0, pos + 1);
    
    
    outfile.open(path + name + ".h", std::ios::out | std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(os.str().c_str(), os.str().length());
        outfile.close();
    }
}

void processAndCreateFile(std::string &filename, GFXfont &gfxFont, std::vector<uint8_t> &data, std::ostringstream &osGlyph,  std::string &name)
{
    std::ostringstream os;
    
    os << "\
// Generated by piXfont\n\
#ifndef PROGMEM\n\
    #define PROGMEM /* None Arduino */\n\
#endif\n\n\
#ifndef " << name << "_h\n\
#define " << name << "_h\n\n\
const uint8_t " << name << "_Bitmaps[] PROGMEM = {\n  \
" << std::setfill('0') << std::setw(2) << std::hex;
    
    for (int n = 0; n < data.size(); n++) {
        if (n % 12) os << " ";
        os << "0x" << std::setw(2) << (int)data.at(n);
        if (n < data.size()-1) os << ",";
        if (n % 12 == 11) os << "\n  ";
    }
    os << "\n};\n\n";
    
    os << "const GFXglyph " << name << "_Glyphs[] PROGMEM = {\n";
    
    os << osGlyph.str() << std::dec << "\n\
const GFXfont " << name << " PROGMEM = {(uint8_t *) " << name << "_Bitmaps, (GFXglyph *) " << name << "_Glyphs, " << (int)gfxFont.first << ", " << (int)gfxFont.last << ", " << (int)gfxFont.yAdvance << "};\n\n\
#endif /* " << name << "_h */\n";
    
    createFile(filename, os, name);
}

// TODO: Add extended support for none monochrome glyphs.

void createNewFont(std::string &filename, std::string &name, GFXfont &gfxFont, int width, int hs, int vs, bool fixed, bool leftAlign, int distance)
{
    Image *monochrome;
    monochrome = loadPBGraphicFile(filename);
    
    if (!monochrome) {
        std::cout << MessageType::Error << "Failed to load the monochrome bitmap file." << filename << ".\n";
        return;
    }
    
    if ((width + hs) * (gfxFont.yAdvance + vs) * (gfxFont.last - gfxFont.first + 1) > monochrome->width * monochrome->height) {
        std::cout << MessageType::Error << "The extraction of glyphs from the provided bitmap image exceeds what is possible based on the image dimensions.\n";
        reset(monochrome);
        return;
    }
    
    // We convert the monochrome bitmap to one byte per pixel to simplify working with the image.
    Image *pixmap = convertMonochromeBitmapToPixmap(monochrome);
    if (!pixmap) {
        // This error shouldn't occur, but if it does, we must release resources and exit.
        std::cout << MessageType::Error << "Failed to Allocate Memory.\n";
        reset(monochrome);
        return;
    }
    
    
    reset(monochrome);
    
    std::vector<uint8_t> data;
    std::ostringstream osGlyph;
    uint16_t offset = 0;
    
    Image *image = createPixmap(width, gfxFont.yAdvance);
    int x, y;
    int indexOffset = asciiExtents(pixmap, width, gfxFont, hs, vs);
    for (int index = 0; index < gfxFont.last - gfxFont.first + 1; index++) {
        indexToXY(index + indexOffset, x, y, pixmap->width, pixmap->height, width + hs, gfxFont.yAdvance + vs);
        copyPixmap(image, 0, 0, pixmap, x, y, image->width, image->height);
        
        GFXglyph gfxGlyph = {
            offset, 0, 0, 0, 0, 0
        };
        gfxGlyph = autoGFXglyphSettings(image);
        
        Image *extractedImage = extractImageSection(image);
        if (!extractedImage) {
            osGlyph << addCharacter(index + gfxFont.first, gfxGlyph, gfxFont);
            continue;
        }
        
        concatenateImageData(extractedImage, data);
        
        if (leftAlign) {
            gfxGlyph.xAdvance -= gfxGlyph.dX;
            gfxGlyph.dX = 0;
        }
        
        if (fixed) {
            gfxGlyph.xAdvance = width;
        } else {
            gfxGlyph.xAdvance += distance;
        }
        
        gfxGlyph.bitmapOffset = offset;
        offset += (extractedImage->width * extractedImage->height + 7) / 8;
        osGlyph << addCharacter(index + gfxFont.first, gfxGlyph, gfxFont);
        reset(extractedImage);
    }
    osGlyph << "};\n";
    
    reset(image);
    reset(pixmap);
    
    processAndCreateFile(filename, gfxFont, data, osGlyph, name);
}

int main(int argc, const char * argv[])
{
    if ( argc == 1 ) {
        error();
        return 0;
    }
    
    std::string filename, name, prefix, sufix;
    
    GFXfont gfxFont = { 0, 0, 32, 95, 8 };
    int width = 8, hs = 0, vs = 0;
    bool fixed = false;
    bool leftAlign = false;
    int distance = 1;
    
    for( int n = 1; n < argc; n++ ) {
        if (*argv[n] == '-') {
            std::string args(argv[n]);
            
            if (args == "-d" || args == "--distance") {
                if (++n > argc) error();
                distance = atoi(argv[n]);
                continue;
            }
            
            if (args == "-n" || args == "--name") {
                if (++n > argc) error();
                name = argv[n];
                continue;
            }
            
            if (args == "-h" || args == "--height") {
                if (++n > argc) error();
                gfxFont.yAdvance = atoi(argv[n]);
                continue;
            }
            
            if (args == "-w" || args == "--width") {
                if (++n > argc) error();
                width = atoi(argv[n]);
                continue;
            }
            
            if (args == "-f" || args == "--first") {
                if (++n > argc) error();
                gfxFont.first = atoi(argv[n]);
                continue;
            }
            
            if (args == "-l" || args == "--last") {
                if (++n > argc) error();
                gfxFont.last = atoi(argv[n]);
                continue;
            }
            
            if (args == "-vs") {
                if (++n > argc) error();
                vs = atoi(argv[n]);
                continue;
            }
            
            if (args == "-hs") {
                if (++n > argc) error();
                hs = atoi(argv[n]);
                continue;
            }
            
            if (args == "-x" || args == "--fixed") {
                fixed = true;
                continue;
            }
            
            if (args == "-a") {
                leftAlign = true;
                continue;
            }
            
            if (args == "--help") {
                usage();
                return 0;
            }
            
            if (args == "--version") {
                version();
                continue;
            }
            
            error();
            return 0;
        }
        filename = argv[n];
    }
    
    if (name.empty()) {
        name = regex_replace(filename, std::regex(R"(\.\w+$)"), "");
        size_t pos = name.rfind("/");
        name = name.substr(pos + 1, name.length() - pos);
    }
 
    createNewFont(filename, name, gfxFont, width, hs, vs, fixed, leftAlign, distance);
    return 0;
}

