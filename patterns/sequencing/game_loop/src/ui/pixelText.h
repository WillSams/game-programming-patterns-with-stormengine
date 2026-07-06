#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <cctype>
#include <string>

// Minimal 3x5 pixel font rendered as filled rects — no font asset or SDL_ttf
// needed, just enough glyphs to label the demo (the letters used in "RATE nn HZ"
// and "INTERP ON/OFF", plus digits, space, and colon). Unknown chars draw blank.
inline std::array<const char *, 5> PixelGlyph(char c) {
    switch (std::toupper(static_cast<unsigned char>(c))) {
    case '0': return {"###", "# #", "# #", "# #", "###"};
    case '1': return {" # ", "## ", " # ", " # ", "###"};
    case '2': return {"###", "  #", "###", "#  ", "###"};
    case '3': return {"###", "  #", "###", "  #", "###"};
    case '4': return {"# #", "# #", "###", "  #", "  #"};
    case '5': return {"###", "#  ", "###", "  #", "###"};
    case '6': return {"###", "#  ", "###", "# #", "###"};
    case '7': return {"###", "  #", "  #", "  #", "  #"};
    case '8': return {"###", "# #", "###", "# #", "###"};
    case '9': return {"###", "# #", "###", "  #", "###"};
    case 'A': return {"###", "# #", "###", "# #", "# #"};
    case 'E': return {"###", "#  ", "###", "#  ", "###"};
    case 'F': return {"###", "#  ", "###", "#  ", "#  "};
    case 'H': return {"# #", "# #", "###", "# #", "# #"};
    case 'I': return {"###", " # ", " # ", " # ", "###"};
    case 'N': return {"# #", "###", "###", "# #", "# #"};
    case 'O': return {"###", "# #", "# #", "# #", "###"};
    case 'P': return {"###", "# #", "###", "#  ", "#  "};
    case 'R': return {"###", "# #", "###", "## ", "# #"};
    case 'T': return {"###", " # ", " # ", " # ", " # "};
    case 'Z': return {"###", "  #", " # ", "#  ", "###"};
    case ':': return {"   ", " # ", "   ", " # ", "   "};
    default:  return {"   ", "   ", "   ", "   ", "   "}; // space / unknown
    }
}

// Draw text at (x, y) using the current render draw color. Each font pixel is a
// `scale`×`scale` block; glyphs are 3 wide with 1 column of spacing.
inline void DrawPixelText(SDL_Renderer *renderer, const std::string &text,
                          int x, int y, int scale) {
    int cursor = x;
    for (char ch : text) {
        std::array<const char *, 5> glyph = PixelGlyph(ch);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 3; ++col)
                if (glyph[row][col] == '#') {
                    SDL_Rect px = { cursor + col * scale, y + row * scale,
                                    scale, scale };
                    SDL_RenderFillRect(renderer, &px);
                }
        cursor += 4 * scale; // 3 columns + 1 spacing
    }
}
