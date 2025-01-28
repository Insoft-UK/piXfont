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

using namespace image;

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
    std::cout << "Usage: " << _basename << " <input-file> -w <value> -h <value> [-n <name>] [-f <value>] [-l <value>] [-s <value>] [-O x-offset y-offset] [-d value]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -w <0...32>             Max width of the bitmap in pixels.\n";
    std::cout << "  -h <0...32>             Font height in pixels.\n";
    std::cout << "  -n <name>               Font name.\n";
    std::cout << "  -f <0...255>            First ASCII value of your first character.\n";
    std::cout << "  -a                      Auto left-align glyphs.\n";
    std::cout << "  -O <x-offset y-offset>  X-axis and Y-axis offset to the start of the glyphs.\n";
    std::cout << "  -d <0...32>             Distance to advance cursor in the x-axis from\n";
    std::cout << "                          the right edge of the glyph, default is 1.\n";
    std::cout << "  -D                      Direction layout of glyphs.\n";
    std::cout << "  -l <0...255>            Last ASCII value of your last character.\n";
    std::cout << "  -s <0...32>             Distance to advance cursor in the x-axis for\n";
    std::cout << "                          ascii char code 32 if not fixed char width.\n";
    std::cout << "  -H <0...32>             Horizontal spacing in pixels between each glyph.\n";
    std::cout << "  -V <0...32>             Vertical spacing in pixels between each glyph.\n";
    std::cout << "  -F <0...32>             Fixed char width.\n";
    std::cout << "  -ppl                    Generate PPL code, not required for .h files.\n";
    std::cout << "  -v                      Display detailed processing information.\n";
    std::cout << "\n";
    std::cout << "  Verbose Flags:\n";
    std::cout << "     f                    Font\n";
    std::cout << "     g                    Glyph\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  ansiart {--version | --help}\n";
    std::cout << "    --version              Display the version information.\n";
    std::cout << "    --help                 Show this help message.\n";
}


static bool verbose = false;
static bool PPL = false;

//enum class MessageType {
//    Warning,
//    Error,
//    Verbose
//};

enum class Direction {
    Horizontal,
    Vertical
};

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


GFXglyph autoGFXglyphSettings(TBitmap &image)
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

void concatenateImageData(TBitmap &image, std::vector<uint8_t> &data)
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

int asciiExtents(const TBitmap &bitmap, GFXfont &font, int hs, int vs, Direction direction)
{
    if (bitmap.bytes.empty()) return 0;
    int x,y;
    int prevFirst = (int)font.first;
    
    for (uint16_t n = 0; n < 256; n++) {
        indexToXY(direction==Direction::Horizontal ? n : -n, x, y, bitmap.width, bitmap.height, font.width + hs, font.yAdvance + vs);
        
        if (containsImage(bitmap, x, y, font.width, font.yAdvance)) {
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

void createImageFile(std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::vector<GFXglyph> &glyphs,  std::string &name)
{
    TBitmap bitmap;
    
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
    }
    bitmap = createBitmap(w, font.yAdvance * (font.last - font.first + 1), 8);
    
    int y = 0;
    char c = adafruitFont.first;
    for (auto it = glyphs.begin(); it < glyphs.end(); it++) {
        font::glyph(0, y, c, (uint8_t)1, adafruitFont, bitmap);
        y += adafruitFont.yAdvance;
        c++;
    }
    std::string path;
    
    size_t pos = filename.rfind("/");
    path = filename.substr(0, pos + 1);
    
    saveBitmap((path + name + ".bmp").c_str(), bitmap);
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

std::string loadAdafruitFont(std::string &filename)
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

bool extractAdafruitFont(std::string &filename, GFXfont &font, std::vector<uint8_t> &data, std::vector<GFXglyph> &glyphs)
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
    while (std::regex_search(s, match, std::regex(R"(\{ *((?:0x)?[\d[a-fA-F]+) *, *(\d+) *, *(\d+) *, *(\d+) *, *(\d+) *, *(-?\d+) *\})"))) {
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

void convertAdafruitFontToImage(std::string &filename, std::string &name)
{
    std::vector<uint8_t> data;
    std::vector<GFXglyph> glyphs;
    GFXfont font;
    
    if (!extractAdafruitFont(filename, font, data, glyphs)) {
        std::cout << "Failed to find valid Adafruit Font data.\n";
        exit(2);
    }
    
    createImageFile(filename, font, data, glyphs, name);
}

// TODO: Add extended support for none monochrome glyphs.

void createNewFont(std::string &filename, std::string &name, GFXfont &font, int hs, int vs, bool fixed, bool leftAlign, int distance, Direction direction)
{
    TBitmap bitmap;
    bitmap = loadBitmap(filename.c_str());
    
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
    
    if ((font.width + hs) * (font.yAdvance + vs) * (font.last - font.first + 1) > bitmap.width * bitmap.height) {
        std::cout << "The extraction of glyphs from the provided bitmap image exceeds what is possible based on the image dimensions.\n";
        return;
    }
    

    std::vector<uint8_t> data;
    std::vector<GFXglyph> glyphs;
    uint16_t offset = 0;
    
    TBitmap image = {
        .width = font.width,
        .height = font.yAdvance
    };
    image.bytes.reserve((size_t)image.width * (size_t)image.height);
    image.bytes.resize((size_t)image.width * (size_t)image.height);
    
    int x, y;
    int indexOffset = asciiExtents(bitmap, font, hs, vs, direction);
    for (int index = 0; index < font.last - font.first + 1; index++) {
        int n = direction == Direction::Horizontal ? index + indexOffset : -(index + indexOffset);
        indexToXY(n, x, y, bitmap.width, bitmap.height, font.width + hs, font.yAdvance + vs);
        copyBitmap(image, 0, 0, bitmap, x + font.xOffset, y + font.yOffset, image.width, image.height);
        
        GFXglyph glyph = {
            offset, 0, 0, 0, 0, 0
        };
        glyph = autoGFXglyphSettings(image);
        
        TBitmap extractedImage = extractImageSection(image);
        if (extractedImage.bytes.empty()) {
            glyph.xAdvance = font.width;
            glyphs.push_back(glyph);
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
    
    std::string filename, name, prefix, sufix;
    
    GFXfont font = { 0, 0, .first=0, .last=0, .yAdvance=8, .width=8, .xSpace=8, .xOffset=0, .yOffset=0 };
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
            
            
            if (args == "-O") {
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
        filename = argv[n];
    }
    
    if (name.empty()) {
        name = regex_replace(filename, std::regex(R"(\.\w+$)"), "");
        size_t pos = name.rfind("/");
        name = name.substr(pos + 1, name.length() - pos);
    }
    
    info();
    
    if (filename.substr(filename.length() - 2, 2) == ".h") {
        if (PPL) {
            convertAdafruitFontToHpprgm(filename, name);
            return 0;
        }
        convertAdafruitFontToImage(filename, name);
        return 0;
    }
    
    createNewFont(filename, name, font, hs, vs, fixed, leftAlign, distance, direction);
    return 0;
}

