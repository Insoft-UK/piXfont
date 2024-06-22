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

enum class Direction {
    Horizontal,
    Vertical
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
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Adafruit GFX Pixel Font Creator.\n\n";
    std::cout << "Usage: pixfont file [-options] -n name -w width -h height [-D h|v] [-o x y] [-v {fg}]\n";
    std::cout << "\n";
    
    // TODO: Add verbose
    std::cout << " -v display detailed processing information\n";
    std::cout << "\tFlags:\n";
    std::cout << "\t\tf font\n";
    std::cout << "\t\tg glyphs\n\n";
    
    std::cout << " -a\tAuto left-align glyphs.\n";
    std::cout << " -o\tX-axis and Y-axis offset to the start of the glyphs.\n";
    std::cout << " -d\tDistance to advance cursor in the x-axis from\n\tthe right edge of the glyph, default is 1.\n";
    std::cout << " -D\tDirection layout of glyphs.\n";
    std::cout << " -f\tFirst ASCII value of your first character.\n";
    std::cout << " -l\tLast ASCII value of your last character.\n";
    std::cout << " -n\tFont name.\n";
    std::cout << " -s\tDistance to advance cursor in the x-axis for\n\tascii char code 32 if not fixed char width.\n";
    std::cout << " -w\tMax width of the bitmap in pixels.\n";
    std::cout << " -h\tFont height in pixels.\n";
    std::cout << " -H\tHorizontal spacing in pixels between each glyph.\n";
    std::cout << " -V\tvertical spacing in pixels between each glyph.\n";
    std::cout << " -F\tFixed char width.\n";
    std::cout << "\n";
    std::cout << "Usage: pixfont {-version | -help}\n";
}

void error(void)
{
    std::cout << "piXfont: try 'pixfont -help' for more information\n";
    exit(0);
}

void version(void) {
    std::cout
    << "Version: piXfont "
    << (unsigned)__BUILD_NUMBER / 100000 << "."
    << (unsigned)__BUILD_NUMBER / 10000 % 10 << "."
    << (unsigned)__BUILD_NUMBER / 1000 % 10 << "."
    << std::setfill('0') << std::setw(3) << (unsigned)__BUILD_NUMBER % 1000
    << "\n";
    std::cout << "Copyright: (C) 2024 Insoft. All rights reserved.\n";
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
    
    if (character == 32) {
        glyph.xAdvance = font.xSpace;
    }
    
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

void indexToXY(int index, int &x, int &y, const int w, const int h, const int cw, const int ch)
{
    if (index < 0) {
        index = -index;
        y = index % (h / ch) * ch;
        x = index / (h / ch) * ch;
        return;
    }
    
    x = index % (w / cw) * cw;
    y = index / (w / cw) * ch;
}

int asciiExtents(const Image *pixmap, GFXfont &font, int hs, int vs, Direction direction)
{
    if (!pixmap || !pixmap->data) return 0;
    int x,y;
    int prevFirst = (int)font.first;
    
    for (uint16_t n = 0; n < 256; n++) {
        indexToXY(direction==Direction::Horizontal ? n : -n, x, y, pixmap->width, pixmap->height, font.width + hs, font.yAdvance + vs);
        
        if (containsImage(pixmap, x, y, font.width, font.yAdvance)) {
            if (font.first == 0) font.first = n;
            if (font.last < n) font.last = n;
            continue;
        }
    
    }
    if (font.first > 32) font.first = 32;
    if (verbose) std::cout << MessageType::Verbose <<
        "ASCII Range :" <<
        " from "<< (int)font.first << " to " << (int)font.last << "\n";
    
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

void processAndCreateFile(std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::ostringstream &osGlyph,  std::string &name)
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
const GFXfont " << name << " PROGMEM = {(uint8_t *) " << name << "_Bitmaps, (GFXglyph *) " << name << "_Glyphs, " << (int)font.first << ", " << (int)font.last << ", " << (int)font.yAdvance << "};\n\n\
#endif /* " << name << "_h */\n";
    
    createFile(filename, os, name);
}

// TODO: Add extended support for none monochrome glyphs.

void createNewFont(std::string &filename, std::string &name, GFXfont &font, int hs, int vs, bool fixed, bool leftAlign, int distance, Direction direction)
{
    Image *monochrome;
    monochrome = loadPBMGraphicFile(filename);
    
    if (!monochrome) {
        std::cout << MessageType::Error << "Failed to load the monochrome bitmap file." << filename << ".\n";
        return;
    }
    
    if ((font.width + hs) * (font.yAdvance + vs) * (font.last - font.first + 1) > monochrome->width * monochrome->height) {
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
    
    Image *image = createPixmap(font.width, font.yAdvance);
    int x, y;
    int indexOffset = asciiExtents(pixmap, font, hs, vs, direction);
    for (int index = 0; index < font.last - font.first + 1; index++) {
        int n = direction == Direction::Horizontal ? index + indexOffset : -(index + indexOffset);
        indexToXY(n, x, y, pixmap->width, pixmap->height, font.width + hs, font.yAdvance + vs);
        copyPixmap(image, 0, 0, pixmap, x + font.xOffset, y + font.yOffset, image->width, image->height);
        
        GFXglyph glyph = {
            offset, 0, 0, 0, 0, 0
        };
        glyph = autoGFXglyphSettings(image);
        
        
        
        Image *extractedImage = extractImageSection(image);
        if (!extractedImage) {
            osGlyph << addCharacter(index + font.first, glyph, font);
            continue;
        }
        
        concatenateImageData(extractedImage, data);
        
        if (leftAlign) {
            glyph.xAdvance -= glyph.dX;
            glyph.dX = 0;
        }
        
        if (fixed) {
            glyph.xAdvance = font.width;
        } else {
            glyph.xAdvance += distance;
        }
        
        glyph.bitmapOffset = offset;
        offset += (extractedImage->width * extractedImage->height + 7) / 8;
        osGlyph << addCharacter(index + font.first, glyph, font);
        reset(extractedImage);
    }
    osGlyph << "};\n";
    
    reset(image);
    reset(pixmap);
    
    processAndCreateFile(filename, font, data, osGlyph, name);
}

int main(int argc, const char * argv[])
{
    if ( argc == 1 ) {
        error();
        return 0;
    }
    
    std::string filename, name, prefix, sufix;
    
    GFXfont font = { 0, 0, .first=0, .last=0, .yAdvance=8, .bitWidth=1, .palette=0, .width=8, .xSpace=8, .xOffset=0, .yOffset=0 };
    int hs = 0, vs = 0;
    bool fixed = false;
    bool leftAlign = false;
    int distance = 1;
    Direction direction = Direction::Horizontal;
    
    
    
    for( int n = 1; n < argc; n++ ) {
        if (*argv[n] == '-') {
            std::string args(argv[n]);
            
            if (args == "-d") {
                if (++n > argc) error();
                distance = atoi(argv[n]);
                continue;
            }
            
            if (args == "-D") {
                if (++n > argc) error();
                args = argv[n];
                if (args!="h" && args!="v") error();
                direction = args=="h" ? Direction::Horizontal : Direction::Vertical;
                continue;
            }
            
            if (args == "-n") {
                if (++n > argc) error();
                name = argv[n];
                continue;
            }
            
            
            if (args == "-o") {
                if (++n > argc) error();
                font.xOffset = atoi(argv[n]);
                if (++n > argc) error();
                font.xOffset = atoi(argv[n]);
                continue;
            }
            
            if (args == "-h") {
                if (++n > argc) error();
                font.yAdvance = atoi(argv[n]);
                continue;
            }
            
            if (args == "-w") {
                if (++n > argc) error();
                font.width = atoi(argv[n]);
                continue;
            }
            
            if (args == "-f") {
                if (++n > argc) error();
                font.first = atoi(argv[n]);
                continue;
            }
            
            if (args == "-l") {
                if (++n > argc) error();
                font.last = atoi(argv[n]);
                continue;
            }
            
            if (args == "-V") {
                if (++n > argc) error();
                vs = atoi(argv[n]);
                continue;
            }
            
            if (args == "-H") {
                if (++n > argc) error();
                hs = atoi(argv[n]);
                continue;
            }
            
            if (args == "-F") {
                fixed = true;
                continue;
            }
            
            if (args == "-a") {
                leftAlign = true;
                continue;
            }
            
            if (args == "-s") {
                if (++n > argc) error();
                font.xSpace = atoi(argv[n]);
                continue;
            }
            
            if (args == "-help") {
                usage();
                return 0;
            }
            
            if (args == "-version") {
                version();
                return 0;
            }
            
            if (args == "-v") {
                if (++n > argc) error();
                args = argv[n];
                verbose = true;
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
    
    createNewFont(filename, name, font, hs, vs, fixed, leftAlign, distance, direction);
    return 0;
}

