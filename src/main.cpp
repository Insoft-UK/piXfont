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


//#include <algorithm>


#include "GFXFont.h"
#include "pbm.hpp"
#include "png.hpp"
#include "image.hpp"
#include "font.hpp"

#include "version_code.h"
#define NAME "Adafruit GFX Pixel Font Creator"
#define COMMAND_NAME "pxfnt"

// MARK: - Command Line
void version(void) {
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Built on: " << DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n";
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n\n";
}

void help(void) {
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "\n";
    std::cout << "Usage: " << COMMAND_NAME << " <input-file> [-o <output-file>] -w <value> -h <value> [-c <columns>] [-n <name>] [-f <value>] [-l <value>] [-a] [-x <x-offset>] [-y <y-offset>] [-u <value>] [-g <h/v>] [-s <value>] [-H <value>] [-V <value>] [-F] [-ppl] [-i] [-v]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>   Specify the filename for generated .bmp, .h or .hpprgm file.\n";
    std::cout << "  -w <value>         Maximum glyph width in pixels.\n";
    std::cout << "  -h <value>         Maximum glyph height in pixels.\n";
    std::cout << "  -c <columns>       Number of glyphs per column when generating a glyph image.\n";
    std::cout << "  -n <name>          Font name.\n";
    std::cout << "  -f <value>         First ASCII value of the first character.\n";
    std::cout << "  -l <value>         Last ASCII value of the last character.\n";
    std::cout << "  -a                 Auto left-align glyphs.\n";
    std::cout << "  -x <x-offset>      X-axis offset where glyphs start within the image file.\n";
    std::cout << "  -y <y-offset>      Y-axis offset where glyphs start within the image file.\n";
    std::cout << "  -u <value>         Cursor advance distance in the x-axis from the\n";
    std::cout << "                     right edge of the glyph (default: 1).\n";
    std::cout << "  -g <h/v>           Set the glyph layout direction, horizontal or vertical.\n";
    std::cout << "  -s <value>         Cursor advance distance in the x-axis for ASCII\n";
    std::cout << "                     character 32 (space), if not using fixed width.\n";
    std::cout << "  -H <value>         Horizontal spacing in pixels between glyphs.\n";
    std::cout << "  -V <value>         Vertical spacing in pixels between glyphs.\n";
    std::cout << "  -F                 Use fixed glyph width.\n";
//    std::cout << "  -indices           Use glyph indices.\n";
    std::cout << "  -i <value>         The color index used to represent a pixel in a glyph when using\n";
    std::cout << "                     a non-monochrome image, for monochrome image value is 0 or 1.\n";
    std::cout << "  -v                 Enable verbose output for detailed processing information.\n";
    std::cout << "\n";
    std::cout << "Verbose Flags:\n";
    std::cout << "  f                  Font details.\n";
    std::cout << "  g                  Glyph details.\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  " << COMMAND_NAME << " {--version | --help}\n";
    std::cout << "    --version        Display version information.\n";
    std::cout << "    --help           Show this help message.\n";
}



static bool verbose = false;
//static bool PPL = false;

enum Direction {
    DirectionHorizontal,
    DirectionVertical
};

typedef struct {
    int cellWidth, cellHeight;
    int cursorAdvance;
    int spaceAdvance;
    int horizontalOffset, verticalOffset;
    int cellHorizontalSpacing, cellVerticalSpacing;
    Direction direction;
    uint8_t indexColor;
} TPiXfont;

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

static std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type pos = in.tellg();
    in.close();
    return pos;
}

static bool isUTF16le(std::ifstream &infile)
{
    uint16_t byte_order_mark;
    
    std::ifstream::pos_type pos = infile.tellg();
    
    infile.seekg(0);
    infile.read((char *)&byte_order_mark, sizeof(uint16_t));
    
#ifndef __LITTLE_ENDIAN__
    byte_order_mark = byte_order_mark >> 8 | byte_order_mark << 8;
#endif
    if (byte_order_mark == 0xFEFF) return true;
    
    infile.seekg(pos);
    return false;
}

static std::string utf16_to_utf8(const uint16_t* utf16_str, size_t utf16_size) {
    std::string utf8_str;
    
    for (size_t i = 0; i < utf16_size; ++i) {
        uint16_t utf16_char = utf16_str[i];
        
#ifndef __LITTLE_ENDIAN__
        utf16_char = utf16_char >> 8 | utf16_char << 8;
#endif

        if (utf16_char < 0x0080) {
            // 1-byte UTF-8
            utf8_str += static_cast<char>(utf16_char);
        }
        else if (utf16_char < 0x0800) {
            // 2-byte UTF-8
            utf8_str += static_cast<char>(0xC0 | ((utf16_char >> 6) & 0x1F));
            utf8_str += static_cast<char>(0x80 | (utf16_char & 0x3F));
        }
        else {
            // 3-byte UTF-8
            utf8_str += static_cast<char>(0xE0 | ((utf16_char >> 12) & 0x0F));
            utf8_str += static_cast<char>(0x80 | ((utf16_char >> 6) & 0x3F));
            utf8_str += static_cast<char>(0x80 | (utf16_char & 0x3F));
        }
    }
    
    return utf8_str;
}

// A list is limited to 10,000 elements. Attempting to create a longer list will result in error 38 (Insufficient memory) being thrown.
static std::string pplList(const void *data, const size_t lengthInBytes, const int columns, bool le = true) {
    std::ostringstream os;
    uint64_t n;
    size_t count = 0;
    size_t length = lengthInBytes;
    uint64_t *bytes = (uint64_t *)data;
    
    while (length >= 8) {
        n = *bytes++;
        
        if (!le) {
            n = swap_endian<uint64_t>(n);
        }
        
#ifndef __LITTLE_ENDIAN__
        /*
         This platform utilizes big-endian, not little-endian. To ensure
         that data is processed correctly when generating the list, we
         must convert between big-endian and little-endian.
         */
        if (le) n = swap_endian<uint64_t>(n);
#endif

        if (count) os << ", ";
        if (count % columns == 0) {
            os << (count ? "\n  " : "  ");
        }
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << n << ":64h";
        
        count += 1;
        length -= 8;
    }
    
    if (length) {
        n = *bytes++;
        
        if (!le) {
            n = swap_endian<uint64_t>(n);
        }
        
        os << " ,";
        if (count % columns == 0) {
            os << (count ? "\n  " : "  ");
        }
        
        // TODO: improve this code for readability.
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << (n & (0xFFFFFFFFFFFFFFFF >> ((8-length) * 8))) << ":64h";
    }
    
    return os.str();
}

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

void appendImageData(std::vector<uint8_t> &data, const image::TImage &image, uint8_t indexColor = 1)
{
    uint8_t *p = (uint8_t *)image.bytes.data();
    uint8_t bitPosition = 1 << 7;
    uint8_t byte = 0;
    
    for (int i = 0; i < image.width * image.height; i++) {
        if(!bitPosition) bitPosition = 1 << 7;
        if (p[i] == indexColor)
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

void createUTF8File(const std::string &filename, std::ostringstream &os, std::string &name)
{
    std::ofstream outfile;
    
    outfile.open(filename, std::ios::out | std::ios::binary);
    if (outfile.is_open()) {
        outfile.write(os.str().c_str(), os.str().length());
        outfile.close();
    }
}

void createUTF16LEFile(const std::string& filename, std::ostringstream &os, std::string &name) {
    std::ofstream outfile;
    std::string str = os.str();
    
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

#include <stdint.h>

uint8_t mirror_byte(uint8_t b) {
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);  // Swap upper and lower 4 bits
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);  // Swap pairs of bits
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);  // Swap individual bits
    return b;
}

void removeLeadingBlankGlyphs(TFont &font, std::vector<TGlyph> &glyphs)
{
    /*
     Remove unnecessary blank glyph entries one by one from the beginning until
     a non-blank entry is found or the [space] (ASCII 32) glyph is reached, ensuring
     not to go past the last glyph, then stop.
    */
    for (int i = font.first; i < font.last && i < 32; i++) {
        // If width or height is zero, it's a blank entry.
        if (glyphs.front().width) break;
        
        std::reverse(glyphs.begin(),glyphs.end());
        glyphs.pop_back();
        std::reverse(glyphs.begin(),glyphs.end());
        font.first++;
    }
}

void removeTrailingBlankGlyphs(TFont &font, std::vector<TGlyph> &glyphs)
{
    /*
     Remove blank glyph entries from the end one by one until a non-blank entry
     is found or the first glyph is reached, then stop.
     */
    for (int i = font.last; i > font.first; i--) {
        // If width or height is zero, it's a blank entry.
        if (glyphs.back().width) break;
        
        glyphs.pop_back();
        font.last--;
    }
}

void trimBlankGlyphs(TFont &font, std::vector<TGlyph> &glyphs)
{
    removeLeadingBlankGlyphs(font, glyphs);
    removeTrailingBlankGlyphs(font, glyphs);
}


void createHpprgmFile(const std::string &filename, TFont &font, std::vector<uint8_t> &data, std::vector<TGlyph> &glyphs,  std::string &name)
{
    std::ostringstream os;
    
    for (auto it = data.begin(); it < data.end(); it++) {
        *it = mirror_byte(*it);
    }
    
    os << "#pragma mode( separator(.,;) integer(h64) )\n\n"
       << "// Generated by Insoft Adafruit GFX Pixel Font Creator version, " << VERSION_NUMBER << "\n"
       << "// Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n\n"
       << "EXPORT " << name << " := {\n"
       << " {\n" << pplList(data.data(), data.size(), 16) << "\n"
       << " },{\n"
       << pplList(glyphs.data(), glyphs.size() * sizeof(TGlyph), 16) << "\n"
       << " }, " << (int)font.first << ", " << (int)font.last << ", " << (int)font.yAdvance << "\n"
       << "};";
    createUTF16LEFile(filename, os, name);
}

void drawAllGlyphsHorizontaly(const int rows, const int columns, const font::TFont &adafruitFont, const image::TImage &image)
{
    int x, y, xAdvance, yAdvance;
    
    x = 0; y = 0;
    xAdvance = image.width / columns;
    yAdvance = image.height / rows;
    
    for (int i = adafruitFont.first; i <= adafruitFont.last; i++, x += xAdvance) {
        if (x > image.width - 1) {
            x = 0;
            y += yAdvance;
        }
        font::drawGlyph(x, y, (char)i, (uint8_t)1, adafruitFont, image);
    }
}

void drawAllGlyphsVerticaly(const int rows, const int columns, const font::TFont &adafruitFont, const image::TImage &image)
{
    int x, y, xAdvance, yAdvance;
    
    x = 0; y = 0;
    xAdvance = image.width / columns;
    yAdvance = image.height / rows;
    
    for (int i = adafruitFont.first; i <= adafruitFont.last; i++, y += yAdvance) {
        if (y > image.height - 1) {
            y = 0;
            x += xAdvance;
        }
        font::drawGlyph(x, y, (char)i, (uint8_t)1, adafruitFont, image);
    }
}

void createImageFile(const std::string &filename, TFont &font, std::vector<uint8_t> &data, std::vector<TGlyph> &glyphs, const std::string &name, const int columns, const TPiXfont &piXfont)
{
    image::TImage image;
    
    font::TFont adafruitFont = {
        .bitmap = data.data(),
        .glyph = (font::TGlyph *)glyphs.data(),
        .first = font.first,
        .last = font.last,
        .yAdvance = font.yAdvance
    };
    
    int w = 0;
    for (auto it = glyphs.begin(); it < glyphs.end(); it++) {
        if (it->xAdvance > w) w = it->xAdvance;
        if (w - it->dX > w) w = w - it->dX;
    }
    
    int h = font.yAdvance;
    for (auto it = glyphs.begin(); it < glyphs.end(); it++) {
        if (it->dY + font.yAdvance + it->height > h) h = it->dY + font.yAdvance + it->height;
    }
    
    int rows = ceil((float)(font.last - font.first + 1) / (float)columns);
    int height = rows * h;
    int width = columns * w;
    
    image = image::createImage(width, height, 8);
    if (piXfont.direction == DirectionHorizontal) {
        drawAllGlyphsHorizontaly(rows, columns, adafruitFont, image);
    } else {
        drawAllGlyphsVerticaly(rows, columns, adafruitFont, image);
    }
    
    saveImage(filename.c_str(), image);
}

void createAdafruitFontFile(const std::string &filename, TFont &font, std::vector<uint8_t> &data, std::vector<TGlyph> &glyphs,  std::string &name)
{
    std::ostringstream os;
    
    os << "// Generated by Insoft Adafruit GFX Pixel Font Creator version, " << VERSION_NUMBER << "\n"
       << "// Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n\n"
       << "#ifndef PROGMEM\n"
       << "    #define PROGMEM /* None Arduino */\n"
       << "#endif\n\n"
       << "#ifndef " << name << "_h\n"
       << "#define " << name << "_h\n\n"
       << "const uint8_t " << name << "_Bitmaps[] PROGMEM = {\n"
       << "    " << std::setfill('0') << std::setw(2) << std::hex;
    
    for (int n = 0; n < data.size(); n++) {
        if (n % 12) os << " ";
        os << "0x" << std::setw(2) << (int)data.at(n);
        if (n < data.size()-1) os << ",";
        if (n % 12 == 11) os << "\n    ";
    }
    os << "\n};\n\n";
    
    os << "const GFXglyph " << name << "_Glyphs[] PROGMEM = {\n";
    
    for (auto it = glyphs.begin(); it < glyphs.end(); it++) {
        size_t index = it - glyphs.begin();
        char character = index + font.first;
        
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
        os << (it == glyphs.end() - 1 ? "  " : ", ");
        
        // Print the character and index in hex
        os << " // 0x" << std::setfill('0') << std::setw(2) << std::hex << (int)index + font.first
           << std::dec << " '" << character << "'";
        
        os << "\n";
    }
    os << std::dec
       << "};\n\n"
       << "const GFXfont " << name << " PROGMEM = {\n"
       << "    (uint8_t *) " << name << "_Bitmaps, \n"
       << "    (GFXglyph *) " << name << "_Glyphs, \n"
       << "    " << (int)font.first << ", \n"
       << "    " << (int)font.last << ", \n"
       << "    " << (int)font.yAdvance << "\n"
       << "};\n\n"
       << "#endif /* " << name << "_h */\n";
    
    
    createUTF8File(filename, os, name);
}

std::string loadAdafruitFont(const std::string &filename)
{
    std::ifstream infile;
    std::string str;
    
    // Open the file in text mode
    infile.open(filename, std::ios::in);
    
    // Check if the file is successfully opened
    if (!infile.is_open()) {
        return str;
    }
    
    if (isUTF16le(infile)) {
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

static uint64_t parseHex(const std::string str) {
    std::regex re(R"(^(?:0x)?([\da-fA-F]{1,16})$)");
    std::smatch match;
    
    if (!regex_search(str, match, re)) return 0;
    uint64_t number = 0;
    for (int i = 0; i < match.str(1).length(); i++) {
        char c = match.str(1).at(i);
        if (c & 0b01000000) c += 9;
        c &= 15;
        number <<= 4;
        number |= c;
    }
    return number;
}

static int parseNumber(const std::string &str)
{
    std::regex hexPattern("^0x[\\da-fA-F]+$");
    std::regex decPattern("^[+-]?\\d+$");
    std::regex octPattern("^0[0-8]+$");
    std::regex binPattern("^0b[01]+$");
    
    if (std::regex_match(str, hexPattern)) return std::stoi(str, nullptr, 16);
    if (std::regex_match(str, decPattern)) return std::stoi(str, nullptr, 10);
    if (std::regex_match(str, octPattern)) return std::stoi(str, nullptr, 8);
    if (std::regex_match(str, binPattern)) return std::stoi(str, nullptr, 2);
    
    return 0;
}

bool extractAdafruitFont(const std::string &filename, TFont &font, std::vector<uint8_t> &data, std::vector<TGlyph> &glyphs)
{
    std::ifstream infile;
    std::string utf8;
    
    utf8 = loadAdafruitFont(filename);
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }
    
    std::smatch match;
    std::regex_search(utf8, match, std::regex(R"(const uint8_t \w+\[\] PROGMEM = \{([^}]*))"));
    if (match[1].matched) {
        auto s = match.str(1);
        while (std::regex_search(s, match, std::regex(R"((?:0x)?[\d[a-fA-F]{1,2})"))) {
            data.push_back(parseNumber(match.str()));
            s = match.suffix().str();
        }
    } else {
        std::cout << "Failed to find <Bitmap Data>.\n";
        return false;
    }
    
    auto s = utf8;
    while (std::regex_search(s, match, std::regex(R"(\{ *((?:0x)?[\d[a-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *, *(-?[xb\da-fA-F]+) *\})"))) {
        TGlyph glyph;
        glyph.bitmapOffset = parseNumber(match.str(1));
        glyph.width = parseNumber(match.str(2));
        glyph.height = parseNumber(match.str(3));
        glyph.xAdvance = parseNumber(match.str(4));
        glyph.dX = parseNumber(match.str(5));
        glyph.dY = parseNumber(match.str(6));
        glyphs.push_back(glyph);
        s = match.suffix().str();
    }
    if (glyphs.empty()) {
        std::cout << "Failed to find <Glyph Table>.\n";
        return false;
    }
    
    if (std::regex_search(s, match, std::regex(R"(((?:0x)?[\da-fA-F]+)\s*,\s*((?:0x)?[\da-fA-F]+)\s*,\s*((?:0x)?[\da-fA-F]+)\s*\};)"))) {
        font.first = parseNumber(match.str(1));
        font.last = parseNumber(match.str(2));
        font.yAdvance = parseNumber(match.str(3));
    } else {
        std::cout << "Failed to find <Font>.\n";
        return false;
    }
    
    return true;
}



bool extractAdafruitFontFromHpprgm(const std::string &filename, TFont &font, std::vector<uint8_t> &data, std::vector<TGlyph> &glyphs)
{
    std::ifstream infile;
    std::string utf8;
    std::string str;
    
    utf8 = loadAdafruitFont(filename);
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }
    
    std::smatch match;
    std::regex re(R"(\{([#0-9A-F:h\s,]*)\} *, *\{([#0-9A-F:h\s,]*)\} *, *(\d+) *, *(\d+) *, *(\d+) *\s*\};)");
    
    if (std::regex_search(utf8, match, re) == false) {
        return false;
    }
    
    re = R"(#([0-9A-F]{2,16}):64h)";
    
    str = match.str(1);
    for (std::sregex_iterator it(str.begin(), str.end(), re), end; it != end; ++it) {
        uint64_t number = parseHex(it->str(1));
        for (int n = 0; n < 8; n++) {
            uint8_t byte = number & 255;
            data.push_back(mirror_byte(byte));
            number >>= 8;
        }
    }

    if (data.empty()) {
        std::cout << "Failed to find <Bitmap Data>.\n";
        return false;
    }
    
    str = match.str(2);
    for (std::sregex_iterator it(str.begin(), str.end(), re), end; it != end; ++it) {
        uint64_t number = parseHex(it->str(1));
        
        TGlyph glyph;
        
        glyph.bitmapOffset = number & 0xFFFFF;
        glyph.width = (number >> 16) & 255;
        glyph.height = (number >> 24) & 255;
        glyph.xAdvance = (number >> 32) & 255;
        glyph.dX = (number >> 40) & 255;
        glyph.dY = (number >> 48) & 255;
        
        glyphs.push_back(glyph);
    }
    if (glyphs.empty()) {
        std::cout << "Failed to find <Glyph Table>.\n";
        return false;
    }
    
    font.first = parseNumber(match.str(3));
    font.last = parseNumber(match.str(4));
    font.yAdvance = parseNumber(match.str(5));
    
    return true;
}

void convertAdafruitFontToHpprgm(std::string &in_filename, std::string &out_filename, std::string &name)
{
    std::vector<uint8_t> data;
    std::vector<TGlyph> glyphs;
    TFont font;
    
    if (!extractAdafruitFont(in_filename, font, data, glyphs)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }

    createHpprgmFile(out_filename, font, data, glyphs, name);
}

void convertAdafruitFontToImage(const std::string &in_filename, const std::string out_filename, const std::string &name, const int glyphsPercolumn, const TPiXfont &piXfont)
{
    std::vector<uint8_t> data;
    std::vector<TGlyph> glyphs;
    TFont font;
    
    if (std::filesystem::path(in_filename).extension() == ".hpprgm") {
        if (!extractAdafruitFontFromHpprgm(in_filename, font, data, glyphs)) {
            std::cout << "Failed to find valid Adafruit Font data.\n";
            exit(2);
        }
    } else {
        if (!extractAdafruitFont(in_filename, font, data, glyphs)) {
            std::cout << "Failed to find valid Adafruit Font data.\n";
            exit(2);
        }
    }
    
    createImageFile(out_filename, font, data, glyphs, name, glyphsPercolumn, piXfont);
}

void createNewFont(const std::string &in_filename, const std::string &out_filename, std::string &name, TFont &font, bool fixed, bool leftAlign, const TPiXfont &piXfont)
{
    image::TImage image;
    image = image::loadImage(in_filename.c_str());
    
    if (image.bytes.empty()) {
        std::cout << "Failed to load the monochrome bitmap file." << in_filename << ".\n";
        return;
    }
    
    if (image.bpp == 1) {
        image = image::convertMonochromeToGrayScale(image);
        if (piXfont.indexColor == 0) {
            for (int i = 0; i < image.width * image.height; i++) {
                image.bytes.at(i) = !image.bytes.at(i);
            }
        }
    }
    
    if (image.bpp != 8) {
        std::cout << "Failed to load a monochrome, grayscale or 256 color bitmap file." << in_filename << ".\n";
        return;
    }
    
    if ((piXfont.cellWidth + piXfont.cellHorizontalSpacing) * (piXfont.cellHeight + piXfont.cellVerticalSpacing) * (font.last - font.first + 1) > image.width * image.height) {
        std::cout << "The extraction of glyphs from the provided bitmap image exceeds what is possible based on the image dimensions.\n";
        return;
    }
    
    for (int i = 1; i < image.width * image.height; i++) {
        if (image.bytes.at(i) == piXfont.indexColor) continue;
        image.bytes.at(i) = 0;
    }
    
    std::vector<uint8_t> data;
    std::vector<TGlyph> glyphs;
    uint16_t bitmapOffset = 0;
    
    image::TImage cellImage = image::createImage(piXfont.cellWidth, piXfont.cellHeight, image::Index256Colors);
    
    
    int x, y;
    
    for (int index = 0; index < font.last - font.first + 1; index++) {
        getCellCoordinates(index, x, y, image.width - piXfont.horizontalOffset, image.height - piXfont.verticalOffset, piXfont.cellWidth + piXfont.cellHorizontalSpacing, piXfont.cellHeight + piXfont.cellVerticalSpacing, piXfont.direction);
       
        x += piXfont.horizontalOffset;
        y += piXfont.verticalOffset;
        
        copyImage(cellImage, 0, 0, image, x, y, cellImage.width, cellImage.height);
        
        image::TImage extractedImage = image::extractImageSection(cellImage);
        if (extractedImage.bytes.empty()) {
            /*
             If the image does not contain a glyph for a character,
             insert a blank entry with xAdvance set to the cell width.
            */
            TGlyph glyph = {0, 0, 0, static_cast<uint8_t>(piXfont.cellWidth + piXfont.cursorAdvance), 0, 0};
            glyphs.push_back(glyph);
            continue;
        }
        
        
        int top, left, bottom, right;
        findImageBounds(top, left, bottom, right, cellImage);
        
        TGlyph glyph = {
            .bitmapOffset = 0,
            .width = static_cast<uint8_t>(right - left + 1),
            .height = static_cast<uint8_t>(bottom - top + 1),
            .xAdvance = static_cast<uint8_t>(left + (right - left + 1)),
            .dX = static_cast<int8_t>(left),
            .dY = static_cast<int8_t>(-cellImage.height + top)
        };
    
        appendImageData(data, extractedImage, piXfont.indexColor);
        
        if (leftAlign) {
            glyph.xAdvance -= glyph.dX;
            glyph.dX = 0;
        }
        
        if (fixed) {
            glyph.xAdvance = piXfont.cellWidth + piXfont.cursorAdvance;
        } else {
            glyph.xAdvance += piXfont.cursorAdvance;
        }
        
        
        glyph.bitmapOffset = bitmapOffset;
        bitmapOffset += (extractedImage.width * extractedImage.height + 7) / 8;
        glyphs.push_back(glyph);
    }
    
    trimBlankGlyphs(font, glyphs);
    
    
    if (std::filesystem::path(out_filename).extension() == ".hpprgm") {
        createHpprgmFile(out_filename, font, data, glyphs, name);
        return;
    }
    createAdafruitFontFile(out_filename, font, data, glyphs, name);
}

int main(int argc, const char * argv[])
{
    if ( argc == 1 ) {
        error();
        return 0;
    }
    
    std::string args(argv[0]);
    
    TPiXfont piXfont{
        .spaceAdvance = 8,
        .cursorAdvance = 1,
        .cellHeight = 8,
        .cellWidth = 8,
        .indexColor = 1
    };
    
    std::string in_filename, out_filename, name, prefix, sufix;
    int columns = 16;
    
    TFont font = { 0, 0, .first = 0, .last = 255, .yAdvance = static_cast<uint8_t>(piXfont.cellHeight) };
    
    bool fixed = false;
    bool leftAlign = false;
    
    
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
                piXfont.cursorAdvance = parseNumber(argv[n]);
                if (piXfont.cursorAdvance < 0) piXfont.cursorAdvance = 1;
                continue;
            }
            
            if (args == "-g") {
                if (++n > argc) error();
                args = argv[n];
                if (args!="h" && args!="v") error();
                piXfont.direction = args=="h" ? DirectionHorizontal : DirectionVertical;
                continue;
            }
            
            if (args == "-n") {
                if (++n > argc) error();
                name = argv[n];
                continue;
            }
            
            
            if (args == "-x") {
                if (++n > argc) error();
                piXfont.horizontalOffset = parseNumber(argv[n]);
                if (piXfont.horizontalOffset < 0) piXfont.horizontalOffset = 0;
                continue;
            }
            
            if (args == "-y") {
                if (++n > argc) error();
                piXfont.verticalOffset = parseNumber(argv[n]);
                if (piXfont.verticalOffset < 0) piXfont.verticalOffset = 0;
                continue;
            }
            
            if (args == "-h") {
                if (++n > argc) error();
                piXfont.cellHeight = parseNumber(argv[n]);
                if (piXfont.cellHeight < 1) piXfont.cellHeight = 1;
                font.yAdvance = piXfont.cellHeight;
                continue;
            }
            
            if (args == "-w") {
                if (++n > argc) error();
                piXfont.cellWidth = parseNumber(argv[n]);
                if (piXfont.cellWidth < 1) piXfont.cellWidth = 1;
                continue;
            }
            
            if (args == "-c") {
                if (++n > argc) error();
                columns = parseNumber(argv[n]);
                if (columns < 1) columns = 1;
                continue;
            }
            
            if (args == "-f") {
                if (++n > argc) error();
                font.first = parseNumber(argv[n]);
                if (font.first < 0 || font.first > 255) font.first = 0;
                continue;
            }
            
            if (args == "-l") {
                if (++n > argc) error();
                font.last = parseNumber(argv[n]);
                if (font.last < 0 || font.last > 255) font.last = 255;
                continue;
            }
            
            if (args == "-V") {
                if (++n > argc) error();
                piXfont.cellVerticalSpacing = parseNumber(argv[n]);
                continue;
            }
            
            if (args == "-H") {
                if (++n > argc) error();
                piXfont.cellHorizontalSpacing = parseNumber(argv[n]);
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
                piXfont.spaceAdvance = parseNumber(argv[n]);
                continue;
            }
            
            if (args == "-i") {
                if (++n > argc) error();
                piXfont.indexColor = parseNumber(argv[n]);
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
    if (font.last < font.first) {
        font.first = 0;
        font.last = 255;
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
        if (in_extension == ".bmp") out_filename.append(".h");
        if (in_extension == ".h") out_filename.append(".hpprgm");
        if (in_extension == ".hpprgm") out_filename.append(".h");
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
        
        if (out_extension == ".bmp") {
            convertAdafruitFontToImage(in_filename, out_filename, name, columns, piXfont);
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
            std::vector<uint8_t> data;
            std::vector<TGlyph> glyphs;
            TFont font;
            extractAdafruitFontFromHpprgm(in_filename, font, data, glyphs);
            createAdafruitFontFile(out_filename, font, data, glyphs, name);
            std::cout << "Adafruit GFX Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        if (out_extension == ".bmp") {
            convertAdafruitFontToImage(in_filename, out_filename, name, columns, piXfont);
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
            createNewFont(in_filename, out_filename, name, font, fixed, leftAlign, piXfont);
            if (out_extension == ".h") {
                std::cout << "Adafruit GFX Pixel Font " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
                return 0;
            }
            std::cout << "Adafruit GFX Pixel Font for HP Prime " << std::filesystem::path(out_filename).filename() << " has been succefuly created.\n";
            return 0;
        }
        
        std::cout << "Error: For ‘" << in_extension << "’ input file, the output file must have a ‘.h’ or ‘.hpprgm’ extension. Please choose a valid output file type.\n";
        return 0;
    }
    
    std::cout << "Error: The specified input ‘" << std::filesystem::path(in_filename).filename() << "‘ file is invalid or not supported. Please ensure the file exists and has a valid format.\n";
    
    return 0;
}

