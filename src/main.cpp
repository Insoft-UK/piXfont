// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft. All rights reserved.
// Originally created in 2024
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

#include "GFXFont.h"
#include "pbm.hpp"
#include "image.hpp"
#include "font.hpp"

#include "version_code.h"
#define NAME "Adafruit GFX Pixel Font Creator"
#define COMMAND_NAME "pxfnt"
#define basename(path)  path.substr(path.find_last_of("/") + 1)

static std::string _basename;

// MARK: - Command Line
void version(void) {
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Built on: " << DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void) {
    std::cout << COMMAND_NAME << ": try '" << COMMAND_NAME << " --help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << "\n\n";
}

void help(void) {
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "\n";
    std::cout << "Usage: " << _basename << " <input-file> -w <value> -h <value> [-c <columns>] [-n <name>] [-f <value>] [-l <value>] [-a] [-x <x-offset>] [-y <y-offset>] [-u <value>] [-g <h/v>] [-s <value>] [-H <value>] [-V <value>] [-F] [-ppl] [-v]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
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
    std::cout << "  -ppl               Generate PPL code (not required for .h files).\n";
    std::cout << "  -v                 Enable verbose output for detailed processing information.\n";
    std::cout << "\n";
    std::cout << "Verbose Flags:\n";
    std::cout << "  f                  Font details.\n";
    std::cout << "  g                  Glyph details.\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  ansiart {--version | --help}\n";
    std::cout << "    --version        Display version information.\n";
    std::cout << "    --help           Show this help message.\n";
}



static bool verbose = false;
static bool PPL = false;

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
    return os.str();
}


GFXglyph autoGFXglyphSettings(image::TImage &image)
{
    GFXglyph gfxGlyph = {0, 0, 0, 0, 0, 0};
    int minX, maxX, minY, maxY;
    
    if (image.bytes.empty()) return gfxGlyph;
    
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
    
    if (maxX < minX || maxY < minY) {
        return gfxGlyph;
    }
    
    gfxGlyph.bitmapOffset = 0;
    gfxGlyph.width = maxX - minX + 1;
    gfxGlyph.height = maxY - minY + 1;
    gfxGlyph.xAdvance = minX + gfxGlyph.width;
    gfxGlyph.dX = minX;
    gfxGlyph.dY = -image.height + minY;
    
    return gfxGlyph;
}

void concatenateImageData(image::TImage &image, std::vector<uint8_t> &data)
{
    uint8_t *p = (uint8_t *)image.bytes.data();
    uint8_t bitPosition = 1 << 7;
    uint8_t byte = 0;
    
    for (int i = 0; i < image.width * image.height; i++) {
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
void getCellCoordinates(int cellIndex, int& outX, int& outY,
                        int gridWidth, int gridHeight,
                        int cellWidth, int cellHeight)
{
    int numCols = gridWidth / cellWidth;
    int numRows = gridHeight / cellHeight;

    if (cellIndex >= 0) {
        // Normal indexing: left-to-right, top-to-bottom
        outX = (cellIndex % numCols) * cellWidth;
        outY = (cellIndex / numCols) * cellHeight;
    } else {
        // Reverse indexing: top-to-bottom, left-to-right
        int absIndex = -cellIndex - 1; // Convert negative index to range
        outX = (absIndex / numRows) * cellWidth;
        outY = (absIndex % numRows) * cellHeight;
    }
}

int asciiExtents(const image::TImage &bitmap, GFXfont &font, const TPiXfont &piXfont)
{
    if (bitmap.bytes.empty()) return 0;
    int x,y;
    int prevFirst = (int)font.first;
    
    for (uint16_t n = 0; n < 256; n++) {
        getCellCoordinates(piXfont.direction == DirectionHorizontal ? n : -n, x, y, bitmap.width - piXfont.horizontalOffset, bitmap.height - piXfont.verticalOffset, piXfont.cellWidth + piXfont.cellHorizontalSpacing, piXfont.cellHeight + piXfont.cellVerticalSpacing);
        x += piXfont.horizontalOffset;
        y += piXfont.verticalOffset;
        
        if (containsImage(bitmap, x, y, piXfont.cellWidth, piXfont.cellHeight)) {
            if (font.first == 0) font.first = n;
            if (font.last < n) font.last = n;
            continue;
        }
    
    }
    if (font.first > 32) font.first = 32;
    if (verbose) std::cout <<
        "ASCII Range :" <<
        " from "<< (int)font.first << " to " << (int)font.last << "\n";
    
    return (int)font.first - prevFirst;
}

void createUTF8File(std::string &filename, std::ostringstream &os, std::string &name)
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

void createUTF16LEFile(std::string& filename, std::ostringstream &os, std::string &name) {
    std::ofstream outfile;
    
    std::string str = os.str();
    
    std::string path;
    
    size_t pos = filename.rfind("/");
    path = filename.substr(0, pos + 1);
    
    outfile.open(path + name + ".hpprgm", std::ios::out | std::ios::binary);
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

void createHpprgmFile(std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::vector<GFXglyph> &glyphs,  std::string &name)
{
    std::ostringstream os;
    
    for (auto it = data.begin(); it < data.end(); it++) {
        *it = mirror_byte(*it);
    }
    
    os << "// Generated by piXfont\nEXPORT " << name << " := {\n {\n";
    os << pplList(data.data(), data.size(), 16);
    os << "\n }";
    os << ",{\n" << pplList(glyphs.data(), glyphs.size() * sizeof(GFXglyph), 16) << "\n }, ";
    os << (int)font.first << ", " << (int)font.last << ", " << (int)font.yAdvance << "\n};";
    createUTF16LEFile(filename, os, name);
}

void drawAllGlyphs(const int rows, const int columns, const font::TFont &adafruitFont, const image::TImage &image)
{
    int x, y, xAdvance, yAdvance;
    
    x = 0; y = 0;
    xAdvance = image.width / columns;
    yAdvance = image.height / rows;
    
    for (int i = adafruitFont.first; i < adafruitFont.last; i++, x += xAdvance) {
        if (x > image.width - 1) {
            x = 0;
            y += yAdvance;
        }
        font::drawGlyph(x, y, (char)i, (uint8_t)1, adafruitFont, image);
    }
}

void createImageFile(const std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::vector<GFXglyph> &glyphs, const std::string &name, const int columns)
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
    drawAllGlyphs(rows, columns, adafruitFont, image);

    std::string path;
    
    size_t pos = filename.rfind("/");
    path = filename.substr(0, pos + 1);
    
    saveImage((path + name + ".bmp").c_str(), image);
}

void createAdafruitFontFile(std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::vector<GFXglyph> &glyphs,  std::string &name)
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
    
    for (auto it = glyphs.begin(); it < glyphs.end(); it++) {
        size_t index = it - glyphs.begin();
        char character = index + font.first;
        if (character < ' ') character = ' ';
        os << "  { " << std::setfill(' ') << std::dec << std::setw(5) << (int)it->bitmapOffset << "," << std::setw(4) << (int)it->width << "," << std::setw(4) << (int)it->height << "," << std::setw(4) << (int)it->xAdvance << "," << std::setw(4) << (int)it->dX << "," << std::setw(4) << (int)it->dY << " }";
        os << (it == glyphs.end() - 1 ? "  " : ", ");
        os << " // 0x" << std::setfill('0') << std::setw(2) << std::hex << (int)index + font.first << std::dec << " ";
        os << "'" << character << "'";
        os << "\n";
    }
    os << std::dec << "};\nconst GFXfont " << name << " PROGMEM = {(uint8_t *) " << name << "_Bitmaps, (GFXglyph *) " << name << "_Glyphs, " << (int)font.first << ", " << (int)font.last << ", " << (int)font.yAdvance << "};\n\n#endif /* " << name << "_h */\n";
    
    
    createUTF8File(filename, os, name);
}

std::string loadAdafruitFont(const std::string &filename)
{
    std::ifstream infile;
    std::string utf8;
    
    // Open the file in text mode
    infile.open(filename, std::ios::in);
    
    // Check if the file is successfully opened
    if (!infile.is_open()) {
        return utf8;
    }
    
    // Use a stringstream to read the file's content into the string
    std::stringstream buffer;
    buffer << infile.rdbuf();
    utf8 = buffer.str();

    infile.close();
    
    return utf8;
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

bool extractAdafruitFont(const std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::vector<GFXglyph> &glyphs)
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
        auto s = match[1].str();
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
        GFXglyph glyph;
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

void convertAdafruitFontToHpprgm(std::string &filename, std::string &name)
{
    std::vector<uint8_t> data;
    std::vector<GFXglyph> glyphs;
    GFXfont font;
    
    if (!extractAdafruitFont(filename, font, data, glyphs)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }

    createHpprgmFile(filename, font, data, glyphs, name);
}

void convertAdafruitFontToImage(const std::string &filename, const std::string &name, const int glyphsPercolumn)
{
    std::vector<uint8_t> data;
    std::vector<GFXglyph> glyphs;
    GFXfont font;
    
    if (!extractAdafruitFont(filename, font, data, glyphs)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }
    
    createImageFile(filename, font, data, glyphs, name, glyphsPercolumn);
}

// TODO: Add extended support for none monochrome glyphs.

void createNewFont(std::string &filename, std::string &name, GFXfont &font, bool fixed, bool leftAlign, const TPiXfont &piXfont)
{
    image::TImage bitmap;
    bitmap = image::loadImage(filename.c_str());
    
    if (bitmap.bytes.empty()) {
        std::cout << "Failed to load the monochrome bitmap file." << filename << ".\n";
        return;
    }
    
    if (bitmap.bpp == 1) {
        bitmap = convertMonochromeToGrayScale(bitmap);
    }
    
    if (bitmap.bpp != 8) {
        std::cout << "Failed to load a monochrome, grayscale or 256 color bitmap file." << filename << ".\n";
        return;
    }
    
    if ((piXfont.cellWidth + piXfont.cellHorizontalSpacing) * (piXfont.cellHeight + piXfont.cellVerticalSpacing) * (font.last - font.first + 1) > bitmap.width * bitmap.height) {
        std::cout << "The extraction of glyphs from the provided bitmap image exceeds what is possible based on the image dimensions.\n";
        return;
    }
    

    std::vector<uint8_t> data;
    std::vector<GFXglyph> glyphs;
    uint16_t offset = 0;
    
    image::TImage image = {
        .width = static_cast<uint16_t>(piXfont.cellWidth),
        .height = static_cast<uint16_t>(piXfont.cellHeight)
    };
    image.bytes.reserve((size_t)image.width * (size_t)image.height);
    image.bytes.resize((size_t)image.width * (size_t)image.height);
    
    int x, y;
    int indexOffset = asciiExtents(bitmap, font, piXfont);
    
    for (int index = 0; index < font.last - font.first + 1; index++) {
        if (piXfont.direction == DirectionHorizontal) {
            getCellCoordinates(index + indexOffset, x, y, bitmap.width - piXfont.horizontalOffset, bitmap.height - piXfont.verticalOffset, piXfont.cellWidth + piXfont.cellHorizontalSpacing, piXfont.cellHeight + piXfont.cellVerticalSpacing);
        } else {
            getCellCoordinates(-(index + indexOffset), x, y, bitmap.width - piXfont.horizontalOffset, bitmap.height - piXfont.verticalOffset, piXfont.cellWidth + piXfont.cellHorizontalSpacing, piXfont.cellHeight + piXfont.cellVerticalSpacing);
        }
        
        
        x += piXfont.horizontalOffset;
        y += piXfont.verticalOffset;
        
        copyImage(image, 0, 0, bitmap, x, y, image.width, image.height);
        
        GFXglyph glyph = {
            offset, 0, 0, 0, 0, 0
        };
        glyph = autoGFXglyphSettings(image);
        
        image::TImage extractedImage = image::extractImageSection(image);
        if (extractedImage.bytes.empty()) {
            glyph.xAdvance = piXfont.cellWidth;
            glyphs.push_back(glyph);
            continue;
        }
        
        concatenateImageData(extractedImage, data);
        
        if (leftAlign) {
            glyph.xAdvance -= glyph.dX;
            glyph.dX = 0;
        }
        
        if (fixed) {
            glyph.xAdvance = piXfont.cellWidth;
        } else {
            glyph.xAdvance += piXfont.cursorAdvance;
        }
        
        glyph.bitmapOffset = offset;
        offset += (extractedImage.width * extractedImage.height + 7) / 8;
        glyphs.push_back(glyph);
    }
    
    if (PPL) {
        createHpprgmFile(filename, font, data, glyphs, name);
        return;
    }
    createAdafruitFontFile(filename, font, data, glyphs, name);
}

int main(int argc, const char * argv[])
{
    if ( argc == 1 ) {
        error();
        return 0;
    }
    
    std::string args(argv[0]);
    _basename = basename(args);
    
    TPiXfont piXfont{
        .spaceAdvance = 8,
        .cursorAdvance = 1,
        .cellHeight = 8,
        .cellWidth = 8
    };
    
    std::string filename, name, prefix, sufix;
    int columns = 16;
    
    GFXfont font = { 0, 0, .first=0, .last=0, .yAdvance=static_cast<uint8_t>(piXfont.cellHeight) };
    bool fixed = false;
    bool leftAlign = false;
    
    
    for( int n = 1; n < argc; n++ ) {
        if (*argv[n] == '-') {
            std::string args(argv[n]);
            
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
                font.yAdvance = piXfont.cellWidth;
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
            
            if (args == "-ppl") {
                PPL = true;
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
        
        if (!filename.empty()) error();
        filename = argv[n];
    }
    
    if (name.empty()) {
        name = regex_replace(filename, std::regex(R"(\.\w+$)"), "");
        size_t pos = name.rfind("/");
        name = name.substr(pos + 1, name.length() - pos);
    }
    
    if (font.last < font.first) {
        font.first = 0;
        font.last = 255;
    }
    
    info();
    
    if (filename.substr(filename.length() - 2, 2) == ".h") {
        if (PPL) {
            convertAdafruitFontToHpprgm(filename, name);
            return 0;
        }
        convertAdafruitFontToImage(filename, name, columns);
        return 0;
    }
    
    createNewFont(filename, name, font, fixed, leftAlign, piXfont);
    return 0;
}

