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

#include "hpprgm.hpp"

#include <fstream>
#include <regex>

#include "common.hpp"

using namespace cmn;

static uint64_t parse_hex(const std::string str) {
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

std::string hpprgm::load(const std::string &filename)
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

bool hpprgm::decodeAdafruitFont(const std::string &utf8, font::TAdafruitFont &font)
{
    std::string str;
    
    // Check if the file is successfully opened
    if (utf8.empty()) {
        std::cerr << "Error no UTF8 data.\n";
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
        uint64_t number = parse_hex(it->str(1));
        for (int n = 0; n < 8; n++) {
            uint8_t byte = number & 255;
            font.data.push_back(mirror_byte(byte));
            number >>= 8;
        }
    }

    if (font.data.empty()) {
        std::cout << "Failed to find <Bitmap Data>.\n";
        return false;
    }
    
    str = match.str(2);
    for (std::sregex_iterator it(str.begin(), str.end(), re), end; it != end; ++it) {
        uint64_t number = parse_hex(it->str(1));
        
        font::TGlyph glyph;
        
        glyph.bitmapOffset = number & 0xFFFFF;
        glyph.width = (number >> 16) & 255;
        glyph.height = (number >> 24) & 255;
        glyph.xAdvance = (number >> 32) & 255;
        glyph.dX = (number >> 40) & 255;
        glyph.dY = (number >> 48) & 255;
        
        font.glyphs.push_back(glyph);
    }
    if (font.glyphs.empty()) {
        std::cout << "Failed to find <Glyph Table>.\n";
        return false;
    }
    
    font.first = parse_number(match.str(3));
    font.last = parse_number(match.str(4));
    font.yAdvance = parse_number(match.str(5));
    
    return true;
}
