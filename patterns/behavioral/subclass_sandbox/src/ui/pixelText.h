#pragma once

#include <SDL2/SDL.h>

#include <string>

#include "glyphs.h"

// Draw text with the 3x5 font in glyphs.h. This half is the presentation -- it
// needs SDL and is deliberately not spec'd; the glyph DATA it draws is pure and
// is (see glyphs.h).
inline void DrawPixelText(SDL_Renderer *renderer, const std::string &text,
                          int x, int y, int scale) {
    int cursor = x;
    for (char ch : text) {
        const glyphs::Glyph glyph = glyphs::PixelGlyph(ch);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 3; ++col)
                if (glyph[row][col] == '#') {
                    SDL_Rect px = {cursor + col * scale, y + row * scale,
                                   scale, scale};
                    SDL_RenderFillRect(renderer, &px);
                }
        cursor += 4 * scale;   // 3 columns + 1 spacing
    }
}

// Width of `text` in pixels at `scale`, for centring and for laying out a column.
inline int PixelTextWidth(const std::string &text, int scale) {
    return static_cast<int>(text.size()) * 4 * scale;
}
