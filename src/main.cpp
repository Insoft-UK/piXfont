// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
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

#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <fstream>
#include <iomanip>

#include "common.hpp"
#include "hpprgm.hpp"
#include "calctype.hpp"
#include "HD44780.h"

#include "pbm.hpp"
#include "png.hpp"
#include "image.hpp"
#include "font.hpp"

using namespace cmn;

static bool verbose = false;

enum Direction {
    DirectionHorizontal,
    DirectionVertical
};

typedef struct {
    int w, h;
    int cursorAdvance;
    int spaceAdvance;
    int xOffset, yOffset;
    int HPadding, VPadding;
    Direction direction;
    uint8_t color;
    unsigned short scale;
    bool subpixel;
    int columns;
} TOptions;

// TODO: Impliment "Indices of glyphs"


#include "../version_code.h"
#define NAME "Adafruit GFX Pixel Font Creator"
#define COMMAND_NAME "pxfnt"

// MARK: - Command Line
void version(void) {
    std::cout 
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n"
    << "Built on: " << DATE << "\n"
    << "Licence: MIT License\n\n"
    << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout 
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n"
    << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n\n";
}

void help(void) {
    std::cout 
    << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n"
    << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n"
    << "\n"
    << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] [-calctype] -w <value> -h <value> [-c <columns>] [-n <name>] [-f <value>] [-l <value>] [-a] [-x <x-offset>] [-y <y-offset>] [-u <value>] [-g <h/v>] [-s <value>] [-H <value>] [-V <value>] [-F] [-i] [-v]\n"
    << "\n"
    << "Options:\n"
    << "  -o <output-file>   Specify the filename for generated .bmp, .h or .hpprgm file.\n"
    << "  -w <value>         Maximum glyph width in pixels.\n"
    << "  -h <value>         Maximum glyph height in pixels.\n"
    << "  -c <columns>       Number of glyphs per column when generating a glyph atlas.\n"
    << "  -n <name>          Font name.\n"
    << "  -f <value>         First ASCII value of the first character.\n"
    << "  -l <value>         Last ASCII value of the last character.\n"
    << "  -a                 Auto left-align glyphs.\n"
    << "  -x <x-offset>      X-axis offset where glyphs start within the image file.\n"
    << "  -y <y-offset>      Y-axis offset where glyphs start within the image file.\n"
    << "  -u <value>         Cursor advance distance in the x-axis from the\n"
    << "                     right edge of the glyph (default: 1).\n"
    << "  -g <h/v>           Glyph layout direction, horizontal or vertical.\n"
    << "  -s <value>         Cursor advance distance in the x-axis for ASCII\n"
    << "                     character 32 (space), if not using fixed width.\n"
    << "  -H <value>         Horizontal padding in pixels between glyphs.\n"
    << "  -V <value>         Vertical padding in pixels between glyphs.\n"
    << "  -F                 Use fixed glyph width.\n"
    << "  --scale <value>    Scale glyphs when generating a glyph atlas.\n"
    << "  --subpixel         Subpixel glyphs when generating a glyph atlas.\n"
    << "  -i <value>         The color index used to represent a pixel in a glyph when using\n"
    << "                     a non-monochrome image. For monochrome image, color is 0 or 1.\n"
    << "  -v                 Enable verbose output for detailed processing information.\n"
    << "\n"
    << "Verbose Flags:\n"
    << "  f                  Font details.\n"
    << "  g                  Glyph details.\n"
    << "\n"
    << "Additional Commands:\n"
    << "  " << COMMAND_NAME << " {--version | --help}\n"
    << "    --version        Display version information.\n"
    << "    --help           Show this help message.\n";
}

// MARK: -



// MARK: - Image

void findImageBounds(int &top, int &left, int &bottom, int &right, const image::TImage &image)
{
    if (image.bytes.empty() || image.bpp != image::Index256Colors)
        return;
    
    uint8_t *p = (uint8_t *)image.bytes.data();
    
    right = 0; bottom = 0;
    left = image.width - 1; top = image.height - 1;
    
    
    for (int y=0; y<image.height; y++) {
        for (int x=0; x<image.width; x++) {
            if (!p[x + y * image.width]) continue;
            if (left > x) left = x;
            if (right < x) right = x;
            if (top > y) top = y;
            if (bottom < y) bottom = y;
        }
    }
    
    if (right < left || bottom < top) {
        top = 0; left = 0; bottom = image.height - 1; right = image.width - 1;
        return;
    }
}

void appendImageData(font::TAdafruitFont &adafruitFont, const image::TImage &image)
{
    uint8_t *p = (uint8_t *)image.bytes.data();
    uint8_t bitPosition = 1 << 7;
    uint8_t byte = 0;
    
    for (int i = 0; i < image.width * image.height; i++) {
        if(!bitPosition) bitPosition = 1 << 7;
        if (p[i] == 1)
            byte |= bitPosition;
        bitPosition >>= 1;
        if (!bitPosition) {
            adafruitFont.data.push_back(byte);
            byte = 0;
        }
    }
    if (bitPosition) {
        adafruitFont.data.push_back(byte);
    }
}


/**
 * @brief Converts a given index into (x, y) coordinates within a grid.
 *
 * This function maps a linear index to 2D coordinates based on the grid dimensions
 * and cell size. If the index is negative, it is interpreted as traversing the grid
 * from top to bottom, left to right. If the index is positive, it follows the standard
 * left-to-right, top-to-bottom order.
 *
 * @param cellIndex The index of the cell.
 *                  - Positive: Traverses left to right, top to bottom.
 *                  - Negative: Traverses top to bottom, left to right.
 * @param outX Reference to store the computed x-coordinate.
 * @param outY Reference to store the computed y-coordinate.
 * @param gridWidth The total width of the grid in pixels.
 * @param gridHeight The total height of the grid in pixels.
 * @param cellWidth The width of each individual cell in pixels.
 * @param cellHeight The height of each individual cell in pixels.
 */
void getCellCoordinates(int cellIndex, int &outX, int &outY,
                        int gridWidth, int gridHeight,
                        int cellWidth, int cellHeight,
                        Direction direction)
{
    int numCols = gridWidth / cellWidth;
    int numRows = gridHeight / cellHeight;

    if (direction == DirectionHorizontal) {
        // Normal indexing: left-to-right, top-to-bottom
        outX = (cellIndex % numCols) * cellWidth;
        outY = (cellIndex / numCols) * cellHeight;
    } else {
        // Reverse indexing: top-to-bottom, left-to-right
        outX = (cellIndex / numRows) * cellWidth;
        outY = (cellIndex % numRows) * cellHeight;
    }
}

// MARK: - File IO

void createUTF8File(const std::string &filename, const std::string &str)
{
    std::ofstream outfile;
    
    outfile.open(filename, std::ios::out | std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(str.c_str(), str.length());
        outfile.close();
    }
}

void createUTF16LEFile(const std::string& filename, const std::string str) {
    std::ofstream outfile;
    
    outfile.open(filename, std::ios::out | std::ios::binary);
    if(!outfile.is_open()) {
        error();
        exit(0x02);
    }
    
    outfile.put(0xFF);
    outfile.put(0xFE);
    
    uint8_t* ptr = (uint8_t*)str.c_str();
    for ( int n = 0; n < str.length(); n++) {
        if (0xc2 == ptr[0]) {
            ptr++;
            continue;
        }
        
        if (0xE0 <= ptr[0]) {
            // 3 Bytes
            uint16_t utf16 = ptr[0];
            utf16 <<= 6;
            utf16 |= ptr[1] & 0b111111;
            utf16 <<= 6;
            utf16 |= ptr[1] & 0b111111;
            
#ifndef __LITTLE_ENDIAN__
            utf16 = utf16 >> 8 | utf16 << 8;
#endif
            outfile.write((const char *)&utf16, 2);
            
            ptr+=3;
            continue;
        }
        
        
        if ('\r' == ptr[0]) {
            ptr++;
            continue;
        }

        // Output as UTF-16LE
        outfile.put(*ptr++);
        outfile.put('\0');
    }
    
    outfile.close();
}

std::string loadTextFile(const std::string &filename)
{
    std::ifstream infile;
    std::string str;
    
    // Open the file in text mode
    infile.open(filename, std::ios::in);
    
    // Check if the file is successfully opened
    if (!infile.is_open()) {
        return str;
    }
    
    if (is_utf16le(infile)) {
        char c;
        while (!infile.eof()) {
            infile.get(c);
            str += c;
            infile.peek();
        }
        
        uint16_t *utf16_str = (uint16_t *)str.c_str();
        str = utf16_to_utf8(utf16_str, str.size() / 2);
    } else {
        // Use a stringstream to read the file's content into the string
        std::stringstream buffer;
        buffer << infile.rdbuf();
        str = buffer.str();
    }

    infile.close();
    
    return str;
}

// MARK: - Deleaing with Glyphs

void removeLeadingBlankGlyphs(font::TAdafruitFont &adafruitFont)
{
    /*
     Remove unnecessary blank glyph entries one by one from the beginning until
     a non-blank entry is found or the [space] (ASCII 32) glyph is reached, ensuring
     not to go past the last glyph, then stop.
    */
    for (int i = adafruitFont.first; i < adafruitFont.last && i < 32; i++) {
        // If width or height is zero, it's a blank entry.
        if (adafruitFont.glyphs.front().width) break;
        
        std::reverse(adafruitFont.glyphs.begin(), adafruitFont.glyphs.end());
        adafruitFont.glyphs.pop_back();
        std::reverse(adafruitFont.glyphs.begin(), adafruitFont.glyphs.end());
        adafruitFont.first++;
    }
}

void removeTrailingBlankGlyphs(font::TAdafruitFont &adafruitFont)
{
    /*
     Remove blank glyph entries from the end one by one until a non-blank entry
     is found or the first glyph is reached, then stop.
     */
    for (int i = adafruitFont.last; i > adafruitFont.first; i--) {
        // If width or height is zero, it's a blank entry.
        if (adafruitFont.glyphs.back().width) break;
        
        adafruitFont.glyphs.pop_back();
        adafruitFont.last--;
    }
}

void trimBlankGlyphs(font::TAdafruitFont &adafruitFont)
{
    removeLeadingBlankGlyphs(adafruitFont);
    removeTrailingBlankGlyphs(adafruitFont);
}


void drawAllGlyphsHorizontaly(const font::TFont &adafruitFont, const image::TImage &image, const TOptions &options)
{
    int x, y, xAdvance, yAdvance;
    
    x = 0; y = 0;
    xAdvance = image.width / options.columns;
    yAdvance = image.height / (int)ceil(256 / (float)options.columns);
    
    for (int i = 0; i <= 255; i++, x += xAdvance) {
        if (x > image.width - 1) {
            x = 0;
            y += yAdvance;
        }
        if (i < adafruitFont.first || i > adafruitFont.last) continue;
        font::drawGlyph(x, y + yAdvance, (char)i, (uint8_t)1, options.scale, options.scale, adafruitFont, image);
    }
}

void drawAllGlyphsVerticaly(const font::TFont &adafruitFont, const image::TImage &image, const TOptions &options)
{
    int x, y, xAdvance, yAdvance;
    
    x = 0; y = 0;
    xAdvance = image.width / options.columns;
    yAdvance = image.height / (int)ceil(256 / (float)options.columns);
    
    for (int i = 0; i <= 255; i++, y += yAdvance) {
        if (y > image.height - 1) {
            y = 0;
            x += xAdvance;
        }
        if (i < adafruitFont.first || i > adafruitFont.last) continue;
        font::drawGlyph(x, y + yAdvance, (char)i, (uint8_t)1, options.scale, options.scale, adafruitFont, image);
    }
}

// MARK: - Creating

image::TImage createImageSubTypeFont(const std::string in_filename, const TOptions &options)
{
    image::TImage image;
    
    image = image::loadImage(in_filename.c_str());
    
    if (image.bytes.empty()) {
        std::cout << "Error: Failed to load the monochrome bitmap file." << in_filename << ".\n";
        return image;
    }
    
    if (image.bpp != 1) {
        std::cout << "Error: Not a monochrome image." << in_filename << ".\n";
        return image;
    }
    
    image::convertMonochromeToIndexed(image);
    image::binarizeImageByIndexWithValue(image, 1, 255);
    image::invertImage(image);
    
    image::convertSubpixel(image);
    return image;
   
}


image::TImage createImageCalcTypeFont(calctype::TCalcTypeFont &font, const TOptions &options)
{
    image::TImage image;
    uint8_t *glyphData = (uint8_t *)font.glyphData.data();
    
    int maxGlyphWidth = 0;
    int maxGlyphHeight = font.height + 1;
    
    for (int i = 0; i < 224; i++) {
        uint16_t offset = font.glyphOffset[i];
        if (offset == 65535) continue;
        
        calctype::TCalcTypeGlyphData *glyph = (calctype::TCalcTypeGlyphData *)&glyphData[offset];
        
        if (glyph->xAdvance < maxGlyphWidth) continue;
        maxGlyphWidth = glyph->xAdvance;
    }
    maxGlyphWidth += 9;
    
    if (options.scale == 3) {
        image = image::createImage(maxGlyphWidth * 16, maxGlyphHeight * 14 + HD44780.yAdvance * 2, 32);
    } else {
        image = image::createImage(maxGlyphWidth / 3 * 16, maxGlyphHeight * 14 + HD44780.yAdvance * 2, 32);
    }
    
    for (int i = 0; i < 224; i++) {
        int xOffset = i % 16 * maxGlyphWidth;
        int yOffset = i / 16 * maxGlyphHeight;
    
        uint16_t offset = font.glyphOffset[i];
        if (offset == 65535) continue;
        
        calctype::TCalcTypeGlyphData *glyph = (calctype::TCalcTypeGlyphData *)&glyphData[offset];
        
        if (options.scale != 3) {
            calctype::drawGlyph(glyph, (xOffset + glyph->xOffset) / 3, yOffset + glyph->yOffset, 0xFFFFFFFF, image);
            continue;
        }
        
        calctype::drawRawGlyph(glyph, xOffset + glyph->xOffset, yOffset + glyph->yOffset, image);
    }
    
    uint32_t color = 0xFFFFFFFF;
    std::ostringstream os;
    os << " CalcType Font\n H:" << (options.scale != 3 ? maxGlyphWidth / 3 : maxGlyphWidth) << " V:" << maxGlyphHeight << " height:" << (int)font.height << " space:" << (int)font.space;
    font::print(0, image.height - 11, os.str().c_str(), color, HD44780, image);
    return image;
}

image::TImage createImageAdafruitFont(font::TAdafruitFont &adafruitFont, const TOptions &options)
{
    image::TImage image;
    
    font::TFont font = {
        .bitmap = adafruitFont.data.data(),
        .glyph = (font::TGlyph *)adafruitFont.glyphs.data(),
        .first = adafruitFont.first,
        .last = adafruitFont.last,
        .yAdvance = adafruitFont.yAdvance
    };
    
    int w = 0;
    for (auto it = adafruitFont.glyphs.begin(); it < adafruitFont.glyphs.end(); it++) {
        if (it->xAdvance > w) w = it->xAdvance;
        if (w - it->dX > w) w = w - it->dX;
    }
    
    int h = font.yAdvance;
    for (auto it = adafruitFont.glyphs.begin(); it < adafruitFont.glyphs.end(); it++) {
        if (it->dY + font.yAdvance + it->height > h) h = it->dY + font.yAdvance + it->height;
    }
    
    
    
    int height, width;
    
    width = options.columns * w * options.scale;
    height = (int)ceil(256 / (float)options.columns) * h * options.scale;

    image = image::createImage(width, height, 8);
    if (options.direction == DirectionHorizontal) {
        drawAllGlyphsHorizontaly(font, image, options);
    } else {
        drawAllGlyphsVerticaly(font, image, options);
    }
    
    return image;
}

std::string buildHAdafruitFont(font::TAdafruitFont &adafruitFont,  std::string &name)
{
    std::ostringstream os;
    
    os << "// Generated by Insoft Adafruit GFX Pixel Font Creator version, " << VERSION_NUMBER << "\n"
       << "// Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n\n"
       << "#ifndef PROGMEM\n"
       << "    #define PROGMEM /* None Arduino */\n"
       << "#endif\n\n"
       << "#ifndef " << name << "_h\n"
       << "#define " << name << "_h\n\n"
       << "const unsigned char " << name << "_Bitmaps[] PROGMEM = {\n"
       << "    " << std::setfill('0') << std::setw(2) << std::hex;
    
    for (int n = 0; n < adafruitFont.data.size(); n++) {
        if (n % 12) os << " ";
        os << "0x" << std::setw(2) << (int)adafruitFont.data.at(n);
        if (n < adafruitFont.data.size()-1) os << ",";
        if (n % 12 == 11) os << "\n    ";
    }
    os << "\n};\n\n";
    
    os << "const GFXglyph " << name << "_Glyphs[] PROGMEM = {\n";
    
    for (auto it = adafruitFont.glyphs.begin(); it < adafruitFont.glyphs.end(); it++) {
        size_t index = it - adafruitFont.glyphs.begin();
        char character = index + adafruitFont.first;
        
        // Ensure character is at least a space if it's below ASCII 32
        if (character < ' ') {
            character = ' ';
        }
        
        // Print the glyph data with proper formatting
        os << "    { "
           << std::setfill(' ') << std::dec << std::setw(5) << (int)it->bitmapOffset << ","
           << std::setw(4) << (int)it->width << ","
           << std::setw(4) << (int)it->height << ","
           << std::setw(4) << (int)it->xAdvance << ","
           << std::setw(4) << (int)it->dX << ","
           << std::setw(4) << (int)it->dY << " }";
        
        // Add comma unless it's the last element
        os << (it == adafruitFont.glyphs.end() - 1 ? "  " : ", ");
        
        // Print the character and index in hex
        os << " // 0x" << std::setfill('0') << std::setw(2) << std::hex << (int)index + adafruitFont.first
           << std::dec << " '" << character << "'";
        
        os << "\n";
    }
    os << std::dec
       << "};\n\n"
       << "const GFXfont " << name << " PROGMEM = {\n"
       << "    (unsigned char *) " << name << "_Bitmaps, \n"
       << "    (GFXglyph *) " << name << "_Glyphs, \n"
       << "    " << (int)adafruitFont.first << ", \n"
       << "    " << (int)adafruitFont.last << ", \n"
       << "    " << (int)adafruitFont.yAdvance << "\n"
       << "};\n\n"
       << "#endif /* " << name << "_h */\n";
    
    return os.str();
}

bool createNewFont(const std::string &in_filename, const std::string &out_filename, std::string &name, font::TAdafruitFont &adafruitFont, bool fixed, bool leftAlign, const TOptions &options)
{
    image::TImage image;
    image = image::loadImage(in_filename.c_str());
    
    if (image.bytes.empty()) {
        std::cout << "Error: Failed to load the monochrome bitmap file." << in_filename << ".\n";
        return false;
    }
    
    image::convertMonochromeToIndexed(image);
    image::binarizeImageByIndex(image, options.color);
    
//    int cols = (image.width + options.HPadding) / (options.w + options.HPadding);
//    int rows = (image.height + options.VPadding) / (options.h + options.VPadding);

//    if (adafruitFont.last - adafruitFont.first + 1 > cols * rows) {
//        std::cout << "Error: The extraction of glyphs from the provided bitmap image exceeds what is possible based on the image dimensions.\n";
//        return false;
//    }
    
    
    
    uint16_t bitmapOffset = 0;
    
    image::TImage cellImage = image::createImage(options.w, options.h, image::Index256Colors);
    
    
    int x, y;
    
    for (int index = 0; index < adafruitFont.last - adafruitFont.first + 1; index++) {
        getCellCoordinates(index, x, y, image.width - options.xOffset, image.height - options.yOffset, options.w + options.HPadding, options.h + options.VPadding, options.direction);
       
        x += options.xOffset;
        y += options.yOffset;
        
        if (!image::containsRegion(image, x, y, cellImage.width, cellImage.height)) {
            /*
             If the image does not contain a glyph for a character,
             insert a blank entry with xAdvance set to the cell width.
            */
            font::TGlyph glyph = {0, 0, 0, static_cast<uint8_t>(options.w + options.cursorAdvance), 0, 0};
            adafruitFont.glyphs.push_back(glyph);
            continue;
        }
        
        copyImage(cellImage, 0, 0, image, x, y, cellImage.width, cellImage.height);
        
        image::TImage extractedImage = image::cropToContent(cellImage);

        
        
        int top, left, bottom, right;
        findImageBounds(top, left, bottom, right, cellImage);
        
        font::TGlyph glyph = {
            .bitmapOffset = 0,
            .width = static_cast<uint8_t>(right - left + 1),
            .height = static_cast<uint8_t>(bottom - top + 1),
            .xAdvance = static_cast<uint8_t>(left + (right - left + 1)),
            .dX = static_cast<int8_t>(left),
            .dY = static_cast<int8_t>(-cellImage.height + top)
        };
    
        appendImageData(adafruitFont, extractedImage);
        
        if (leftAlign) {
            glyph.xAdvance -= glyph.dX;
            glyph.dX = 0;
        }
        
        if (fixed) {
            glyph.xAdvance = options.w + options.cursorAdvance;
        } else {
            glyph.xAdvance += options.cursorAdvance;
        }
        
        
        glyph.bitmapOffset = bitmapOffset;
        bitmapOffset += (extractedImage.width * extractedImage.height + 7) / 8;
        adafruitFont.glyphs.push_back(glyph);
    }
    
    trimBlankGlyphs(adafruitFont);
    
    std::string str;
    
    if (std::filesystem::path(out_filename).extension() == ".hpprgm") {
        str = hpprgm::buildHpprgmAdafruitFont(adafruitFont, name);
        createUTF16LEFile(out_filename, str);
        return true;
    }
    str = buildHAdafruitFont(adafruitFont, name);
    createUTF8File(out_filename, str);
    return true;
}

// MARK: - Decoding

bool parseHAdafruitFont(const std::string &filename, font::TAdafruitFont &font)
{
    std::ifstream infile;
    std::string utf8;
    
    utf8 = loadTextFile(filename);
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }
    
    std::smatch match;
    std::regex_search(utf8, match, std::regex(R"(const (?:unsigned )?[\w]+ \w+\[\] PROGMEM = \{([^}]*))"));
    if (match[1].matched) {
        auto s = match.str(1);
        while (std::regex_search(s, match, std::regex(R"((?:0x)?[\d[a-fA-F]{1,2})"))) {
            font.data.push_back(parse_number(match.str()));
            s = match.suffix().str();
        }
    } else {
        std::cout << "Failed to find <Bitmap Data>.\n";
        return false;
    }
    
    auto s = utf8;
    while (std::regex_search(s, match, std::regex(R"(\{ *((?:0x)?[\d[a-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *\})"))) {
        font::TGlyph glyph;
        glyph.bitmapOffset = parse_number(match.str(1));
        glyph.width = parse_number(match.str(2));
        glyph.height = parse_number(match.str(3));
        glyph.xAdvance = parse_number(match.str(4));
        glyph.dX = parse_number(match.str(5));
        glyph.dY = parse_number(match.str(6));
        font.glyphs.push_back(glyph);
        s = match.suffix().str();
    }
    if (font.glyphs.empty()) {
        std::cout << "Failed to find <Glyph Table>.\n";
        return false;
    }
    
    if (std::regex_search(s, match, std::regex(R"(((?:0x)?[\da-fA-F]+)\s*,\s*(?:\.\w+ *= *)?((?:0x)?[\da-fA-F]+)\s*,\s*(?:\.\w+ *= *)?((?:0x)?[\da-fA-F]+)\s*\};)"))) {
        font.first = parse_number(match.str(1));
        font.last = parse_number(match.str(2));
        font.yAdvance = parse_number(match.str(3));
    } else {
        std::cout << "Failed to find <Font>.\n";
        return false;
    }
    
    return true;
}


// MARK: - Converting

void convertAdafruitFontToHpprgm(std::string &in_filename, std::string &out_filename, std::string &name)
{
    font::TAdafruitFont adafruitFont;
    std::string str;
    
    if (!parseHAdafruitFont(in_filename, adafruitFont)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }

    str = hpprgm::buildHpprgmAdafruitFont(adafruitFont, name);
    createUTF16LEFile(out_filename, str);
}



image::TImage convertAdafruitFontToImage(const std::string &in_filename, const TOptions &options)
{
    font::TAdafruitFont adafruitFont;
    
    if (std::filesystem::path(in_filename).extension() == ".hpprgm") {
        if (!hpprgm::parseAdafruitFontFile(in_filename, adafruitFont)) {
            std::cout << "Failed to find valid Adafruit Font data.\n";
            exit(2);
        }
    } else {
        if (!parseHAdafruitFont(in_filename, adafruitFont)) {
            std::cout << "Failed to find valid Adafruit Font data.\n";
            exit(2);
        }
    }
    
    return createImageAdafruitFont(adafruitFont, options);
}

image::TImage convertCalcTypeFontToImage(const std::string &in_filename, const TOptions &options)
{
    calctype::TCalcTypeFont calcTypeFont;
    calctype::decodeFont(in_filename, calcTypeFont);
    return createImageCalcTypeFont(calcTypeFont, options);
}


// MARK: - Main

int main(int argc, const char * argv[])
{
    if ( argc == 1 ) {
        error();
        return 0;
    }
    
    std::string args(argv[0]);
    
    TOptions options{
        .spaceAdvance = 8,
        .cursorAdvance = 1,
        .h = 8,
        .w = 8,
        .color = 1,
        .scale = 1,
        .columns = 16
    };
    
    std::string in_filename, out_filename, name, prefix, sufix;
   
    
    
    bool fixed = false;
    bool leftAlign = false;
    
    font::TAdafruitFont adafruitFont = {
        .first = 0,
        .last = 255,
        .yAdvance = 8
    };
    
    for( int n = 1; n < argc; n++ ) {
        if (*argv[n] == '-') {
            std::string args(argv[n]);
            
            if (args == "-o") {
                if (++n > argc) error();
                out_filename = argv[n];
                continue;
            }
            
            if (args == "-u") {
                if (++n > argc) error();
                options.cursorAdvance = parse_number(argv[n]);
                if (options.cursorAdvance < 0) options.cursorAdvance = 1;
                continue;
            }
            
            if (args == "-g") {
                if (++n > argc) error();
                args = argv[n];
                if (args!="h" && args!="v") error();
                options.direction = args=="h" ? DirectionHorizontal : DirectionVertical;
                continue;
            }
            
            if (args == "-n") {
                if (++n > argc) error();
                name = argv[n];
                continue;
            }
            
            
            if (args == "-x") {
                if (++n > argc) error();
                options.xOffset = parse_number(argv[n]);
                if (options.xOffset < 0) options.xOffset = 0;
                continue;
            }
            
            if (args == "-y") {
                if (++n > argc) error();
                options.yOffset = parse_number(argv[n]);
                if (options.yOffset < 0) options.yOffset = 0;
                continue;
            }
            
            if (args == "-h") {
                if (++n > argc) error();
                options.h = parse_number(argv[n]);
                if (options.h < 1) options.h = 1;
                adafruitFont.yAdvance = options.h;
                continue;
            }
            
            if (args == "-w") {
                if (++n > argc) error();
                options.w = parse_number(argv[n]);
                if (options.w < 1) options.w = 1;
                continue;
            }
            
            if (args == "-c") {
                if (++n > argc) error();
                options.columns = parse_number(argv[n]);
                if (options.columns < 1) options.columns = 1;
                continue;
            }
            
            if (args == "-f") {
                if (++n > argc) error();
                adafruitFont.first = parse_number(argv[n]);
                if (adafruitFont.first < 0 || adafruitFont.first > 255) adafruitFont.first = 0;
                continue;
            }
            
            if (args == "-l") {
                if (++n > argc) error();
                adafruitFont.last = parse_number(argv[n]);
                if (adafruitFont.last < 0 || adafruitFont.last > 255) adafruitFont.last = 255;
                continue;
            }
            
            if (args == "-V") {
                if (++n > argc) error();
                options.VPadding = parse_number(argv[n]);
                continue;
            }
            
            if (args == "-H") {
                if (++n > argc) error();
                options.HPadding = parse_number(argv[n]);
                continue;
            }
            
            if (args == "-F") {
                fixed = true;
                continue;
            }
            
            if (args == "--scale") {
                if (++n > argc) error();
                options.scale = parse_number(argv[n]);
                if (options.scale < 2 || options.scale >= 10) options.scale = 1;
                continue;
            }
            
           
            
            if (args == "-a") {
                leftAlign = true;
                continue;
            }
            
            if (args == "-s") {
                if (++n > argc) error();
                options.spaceAdvance = parse_number(argv[n]);
                continue;
            }
            
            if (args == "-i") {
                if (++n > argc) error();
                options.color = parse_number(argv[n]);
                continue;
            }
            
            if (args == "--subpixel") {
                options.subpixel = true;
                continue;
            }
            
            if (args == "--help") {
                help();
                return 0;
            }
            
            if (args == "--version") {
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
        
        if (!in_filename.empty()) error();
        in_filename = argv[n];
    }
    
    
    if (name.empty()) {
        name = std::filesystem::path(in_filename).stem().string();
    }
    
    /*
     We ensure that if an invalid ‘last’ or ‘first’ entry is provided, we
     revert to the default values for ‘last’ and ‘first’.
     */
    if (adafruitFont.last < adafruitFont.first) {
        adafruitFont.first = 0;
        adafruitFont.last = 255;
    }
    
    /*
     Initially, we display the command-line application’s basic information,
     including its name, version, and copyright details.
     */
    info();
    
    /*
     If no output file is specified, the program will use the input file’s name
     (excluding its extension) as the output file name.
     */
    if (out_filename.empty() || out_filename == in_filename) {
        out_filename = std::filesystem::path(in_filename).parent_path();
        out_filename.append("/");
        out_filename.append(std::filesystem::path(in_filename).stem().string());
        
        auto type = std::filesystem::path(in_filename).extension().string();
    }
    
    std::string in_extension = std::filesystem::path(in_filename).extension();
    
    /*
     If the output file does not have an extension, a default is applied based on
     the input file’s extension:
     
     • For an input file with a .bmp extension, the default output extension is .h.
     • For an input file with a .h extension, the default output extension is .hpprgm.
     • For an input file with a .hpprgm extension, the default output extension is .h.
     */
    if (std::filesystem::path(out_filename).extension().empty()) {
        if (in_extension == ".bmp" || in_extension == ".pbm" || in_extension == ".png") out_filename.append(".h");
        if (in_extension == ".h") out_filename.append(".bmp");
        if (in_extension == ".hpprgm") out_filename.append(".bmp");
    }
    
    std::string out_extension = std::filesystem::path(out_filename).extension();
    
    /*
     We need to ensure that the specified output filename includes a path.
     If no path is provided, we prepend the path from the input file.
     */
    if (std::filesystem::path(out_filename).parent_path().empty()) {
        out_filename.insert(0, "/");
        out_filename.insert(0, std::filesystem::path(in_filename).parent_path());
    }
    
    /*
     The output file must differ from the input file. If they are the same, the
     process will be halted and an error message returned to the user.
     */
    if (in_filename == out_filename) {
        std::cout << "Error: The output file must differ from the input file. Please specify a different output file name.\n";
        return 0;
    }
    
    /*
     If the input file is a .h file, the output must be either .hpprgm or .bmp;
     otherwise, the process is halted and an error is reported to the user.
     */
    if (in_extension == ".h") {
        if (out_extension == ".hpprgm") {
            convertAdafruitFontToHpprgm(in_filename, out_filename, name);
            std::cout << "Adafruit GFX Pixel Font for HP Prime " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        if (out_extension == ".bmp" || out_extension == ".png") {
            image::TImage image;
            
            if (calctype::isCalcType(in_filename)) {
                image = convertCalcTypeFontToImage(in_filename, options);
            } else {
                image = convertAdafruitFontToImage(in_filename, options);
            }
            
            saveImage(out_filename.c_str(), image);
            if (!filesize(out_filename.c_str())) {
                std::cout << "Error: For ‘." << std::filesystem::path(out_filename).filename() << "’ output file, failed to output file.\n";
                return 0;
            }
            std::cout << "Bitmap Representation of Adafruit GFX Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        std::cout << "Error: For ‘." << in_extension << "’ input file, the output file must have a ‘.hpprgm’ or ‘.bmp’ extension. Please choose a valid output file type.\n";
        return 0;
    }
    
    /*
     If the input file is a .hpprgm file, the output must be either .h or .bmp;
     otherwise, the process is halted and an error is reported to the user.
     */
    if (in_extension == ".hpprgm") {
        if (out_extension == ".h") {
            hpprgm::parseAdafruitFontFile(in_filename, adafruitFont);
            std::string utf8 = buildHAdafruitFont(adafruitFont, name);
            createUTF8File(out_filename, utf8);
            std::cout << "Adafruit GFX Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        if (out_extension == ".bmp" || out_extension == ".png") {
            image::TImage image = convertAdafruitFontToImage(in_filename, options);
            if (!saveImage(out_filename.c_str(), image)) {
                std::cout << "Error: For ‘." << std::filesystem::path(out_filename).filename() << "’ output file, failed to output file.\n";
                return 0;
            }
            std::cout << "Bitmap Representation of Adafruit GFX Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        std::cout << "Error: For ‘" << in_extension << "’ input file, the output file must have a ‘.h’ or ‘.bmp’ extension. Please choose a valid output file type.\n";
        return 0;
    }
    
    /*
     If the input file is a .bmp, .pbm or .png file, the output must be either .h or .hpprgm;
     otherwise, the process is halted and an error is reported to the user.
     */
    if (in_extension == ".pbm" || in_extension == ".bmp" || in_extension == ".png") {
        if (out_extension == ".hpprgm" || out_extension == ".h") {
            if (createNewFont(in_filename, out_filename, name, adafruitFont, fixed, leftAlign, options) == false) {
                return 0;
            }
            if (out_extension == ".h") {
                std::cout << "Adafruit GFX Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
                return 0;
            }
            std::cout << "Adafruit GFX Pixel Font for HP Prime " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        // Debugging sub-pixel
        if (out_extension == ".bmp") {
            image::TImage image;
            image = createImageSubTypeFont(in_filename, options);
            saveImage(out_filename.c_str(), image);
            if (!filesize(out_filename.c_str())) {
                std::cout << "Error: For ‘." << std::filesystem::path(out_filename).filename() << "’ output file, failed to output file.\n";
                return 0;
            }
            std::cout << "Bitmap Representation of Sub Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            
            return 0;
        }
        
        std::cout << "Error: For ‘" << in_extension << "’ input file, the output file must have a ‘.h’ or ‘.hpprgm’ extension. Please choose a valid output file type.\n";
        return 0;
    }
    
    std::cout << "Error: The specified input ‘" << std::filesystem::path(in_filename).filename() << "‘ file is invalid or not supported. Please ensure the file exists and has a valid format.\n";
    
    return 0;
}

