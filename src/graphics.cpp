//
//  graphics.cpp
//  piXfont
//
//  Created by Richie on 03/03/2025.
//  Copyright © 2025 Insoft. All rights reserved.
//

#include "graphics.hpp"

void graphics::drawFillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint8_t color, const image::TImage &image)
{
    uint8_t *dst = (uint8_t *)image.bytes.data();
    while (h--) {
        if (y >= image.height) return;
        for (int i = 0; i < w; i++) {
            if (x + i >= image.width) break;
            dst[y * image.height + x + i] = color;
        }
        y++;
    }
}

void graphics::drawFillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint32_t color, const image::TImage &image)
{
    uint32_t *dst = (uint32_t *)image.bytes.data();
    while (h--) {
        if (y >= image.height) return;
        for (int i = 0; i < w; i++) {
            if (x + i >= image.width) break;
            dst[y * image.height + x + i] = color;
        }
        y++;
    }
}

void graphics::setPixel(int16_t x, int16_t y, const uint8_t color, const image::TImage &image)
{
    if (x < 0 || y < 0) return;
    if (x >= image.width || y >= image.height) return;
    uint8_t *dst = (uint8_t *)image.bytes.data();
    dst[x + y * image.width] = color;
}

void graphics::setPixel(int16_t x, int16_t y, const uint32_t color, const image::TImage &image)
{
    if (x < 0 || y < 0) return;
    if (x >= image.width || y >= image.height) return;
    uint32_t *dst = (uint32_t *)image.bytes.data();
    dst[x + y * image.width] = color;
}
