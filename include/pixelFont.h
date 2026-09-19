#pragma once

#include <array>
#include <cctype>

// ── A 3x5 pixel font, as DATA, with no SDL in sight ──────────────────────────
//
// ⚠️ SHARED, AND ITS SPEC IS TOO, and getting that wrong is why this header is
// here rather than copied into each pattern. It was copied twice -- byte-identical,
// 234 lines with its drawer and its spec -- and eight patterns remained, which
// would have been ten copies and ten copies of a spec. CODING.md's first tenet:
// "when the same logic appears twice, promote the shared DECISION to one named
// inline function in the header that owns the behavior", and its meta-principle:
// "fix the boundary, not the copy". A glyph fix must not be an N-place edit.
//
// The pattern folders still repeat their SHELL deliberately -- `game.h` and the
// `playState` skeleton are a template a reader copies and makes their own, which
// is an affordance. A glyph table is not a template; it is one decision.
//
// The spec lives in `patterns/__shared__/specs/pixelFont.spec.cpp`, because the
// per-pattern Makefile only globs `specs/*.cpp` and a shared spec had no home.
// The folder exists for that reason and its `main.cpp` is a stub so the existing
// CI loop -- "any folder with a Makefile" -- runs it like any other.
//
// ⚠️ THAT SPEC IS NOT DECORATION. A missing glyph draws BLANK -- the character
// is simply not there, and nothing anywhere says so. That failure mode shipped
// in the sibling game repo: the UI font had no em dash, so `"CONNECTED — WAITING"`
// drew a hole where the dash should be, invisible on the developer's machine and
// obvious on a player's. A font is a coverage contract, so it gets a test.
//
// ⚠️ 3x5 IS A COMPROMISE FOR M, N AND W, and it is worth knowing rather than
// discovering: three columns cannot draw a convincing diagonal, so M and N
// differ only by their middle row and W is M upside down.
//
// ⚠️ M AND N ARE THE HARD PAIR AND BOTH EARLIER ATTEMPTS WERE WRONG, each in a
// way ONLY LOOKING CATCHES:
//
//   * N as {"# #","## ","# #","# #","# #"} draws a K -- the demo's own label read
//     "SPELL 1 MIKOR HEAL".
//   * N as {"# #","###","# #","# #","# #"} then drew an H, because H's crossbar
//     is one row lower and three columns cannot separate them by one row.
//
// What is left is the pair that CAN be told apart: M carries the crossbar on row
// 1 and N on rows 1 AND 2, so N reads as the diagonal it is meant to be. That is
// the same N the sibling pattern's font uses.
//
// No test can see any of this; rendering the alphabet and reading it can. The
// glyphs are drawn, so they get looked at. The demo's labels avoid
// relying on the distinction; the glyphs exist so no character silently blanks.
namespace glyphs {

using Glyph = std::array<const char *, 5>;

// Unknown characters return a blank glyph. `HasVisibleGlyph` is how a caller
// finds that out rather than drawing nothing and wondering.
inline Glyph PixelGlyph(char c) {
    switch (std::toupper(static_cast<unsigned char>(c))) {
    case 'A': return {"###", "# #", "###", "# #", "# #"};
    case 'B': return {"## ", "# #", "## ", "# #", "## "};
    case 'C': return {"###", "#  ", "#  ", "#  ", "###"};
    case 'D': return {"## ", "# #", "# #", "# #", "## "};
    case 'E': return {"###", "#  ", "###", "#  ", "###"};
    case 'F': return {"###", "#  ", "###", "#  ", "#  "};
    case 'G': return {"###", "#  ", "# #", "# #", "###"};
    case 'H': return {"# #", "# #", "###", "# #", "# #"};
    case 'I': return {"###", " # ", " # ", " # ", "###"};
    case 'J': return {"###", "  #", "  #", "# #", "###"};
    case 'K': return {"# #", "## ", "#  ", "## ", "# #"};
    case 'L': return {"#  ", "#  ", "#  ", "#  ", "###"};
    case 'M': return {"# #", "###", "# #", "# #", "# #"};
    case 'N': return {"# #", "###", "###", "# #", "# #"};
    case 'O': return {"###", "# #", "# #", "# #", "###"};
    case 'P': return {"###", "# #", "###", "#  ", "#  "};
    case 'Q': return {"###", "# #", "# #", "###", "  #"};
    case 'R': return {"###", "# #", "###", "## ", "# #"};
    case 'S': return {"###", "#  ", "###", "  #", "###"};
    case 'T': return {"###", " # ", " # ", " # ", " # "};
    case 'U': return {"# #", "# #", "# #", "# #", "###"};
    case 'V': return {"# #", "# #", "# #", "# #", " # "};
    case 'W': return {"# #", "# #", "###", "###", "# #"};
    case 'X': return {"# #", "# #", " # ", "# #", "# #"};
    case 'Y': return {"# #", "# #", "###", " # ", " # "};
    case 'Z': return {"###", "  #", " # ", "#  ", "###"};
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
    case ':': return {"   ", " # ", "   ", " # ", "   "};
    case '.': return {"   ", "   ", "   ", "   ", " # "};
    case '-': return {"   ", "   ", "###", "   ", "   "};
    case '/': return {"  #", "  #", " # ", "#  ", "#  "};
    case '>': return {"#  ", " # ", "  #", " # ", "#  "};
    case '<': return {"  #", " # ", "#  ", " # ", "  #"};
    case '[': return {"## ", "#  ", "#  ", "#  ", "## "};
    case ']': return {" ##", "  #", "  #", "  #", " ##"};
    case '=': return {"   ", "###", "   ", "###", "   "};
    default:  return {"   ", "   ", "   ", "   ", "   "};  // space / unknown
    }
}

// Does this character draw anything? False for space and for anything the font
// does not cover -- the difference matters, and a caller printing a label can
// check it instead of shipping a hole.
inline bool HasVisibleGlyph(char c) {
    if (c == ' ')
        return false;
    for (const char *row : PixelGlyph(c))
        for (int i = 0; i < 3; ++i)
            if (row[i] == '#')
                return true;
    return false;
}

} // namespace glyphs
