#include <igloo/igloo_alt.h>

#include <cstring>
#include <string>

#include "pixelFont.h"

using namespace igloo;

// The font is a COVERAGE CONTRACT, so it gets a test.
//
// ⚠️ A missing glyph draws BLANK -- the character is simply absent and nothing
// says so. That failure shipped in the sibling game repo: the UI font had no em
// dash, so `"CONNECTED — WAITING"` drew a hole where the dash belongs. It is
// invisible on the machine that has the font and obvious on a player's.
Describe(GlyphCoverageSpec) {

  It(covers_every_letter_and_digit) {
    for (char c = 'A'; c <= 'Z'; ++c)
      Assert::That(glyphs::HasVisibleGlyph(c), IsTrue());
    for (char c = '0'; c <= '9'; ++c)
      Assert::That(glyphs::HasVisibleGlyph(c), IsTrue());
  };

  It(covers_the_punctuation_the_demo_prints) {
    for (char c : std::string(":.-/<>[]="))
      Assert::That(glyphs::HasVisibleGlyph(c), IsTrue());
  };

  It(draws_nothing_for_a_space) {
    Assert::That(glyphs::HasVisibleGlyph(' '), IsFalse());
  };

  It(reports_a_character_it_does_not_cover_as_blank) {
    Assert::That(glyphs::HasVisibleGlyph('~'), IsFalse());
    Assert::That(glyphs::HasVisibleGlyph('@'), IsFalse());
  };

  It(is_case_insensitive) {
    Assert::That(glyphs::PixelGlyph('a'), Equals(glyphs::PixelGlyph('A')));
  };

  // ⚠️ STRUCTURAL, AND NOT PEDANTRY: the drawer indexes row[0..2] of every row.
  // A row that is not exactly three characters is an out-of-bounds read, and a
  // short one is easy to type by accident in a table this size.
  It(gives_every_glyph_five_rows_of_exactly_three_columns) {
    const std::string all =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.-/<>[]= ";
    for (char c : all) {
      const glyphs::Glyph g = glyphs::PixelGlyph(c);
      Assert::That(g.size(), Equals(static_cast<std::size_t>(5)));
      for (const char *row : g)
        Assert::That(std::strlen(row), Equals(static_cast<std::size_t>(3)));
    }
  };

  It(only_ever_uses_hash_and_space_inside_a_glyph) {
    const std::string all =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.-/<>[]=";
    for (char c : all)
      for (const char *row : glyphs::PixelGlyph(c))
        for (int i = 0; i < 3; ++i)
          Assert::That(row[i] == '#' || row[i] == ' ', IsTrue());
  };

  // ⚠️ NO TWO CHARACTERS MAY RENDER IDENTICALLY, EXCEPT THE TWO CONVENTIONS.
  //
  // This found a real collision the moment it was written: O and 0 were the same
  // glyph, and so were S and 5 -- both conventional in a font this small, and
  // both readable from context ("HP 0", "SPELL 1"). Forcing them apart is worse
  // than allowing them: a slashed zero at 3x5 is a blob, and a squared-off 5
  // stops reading as a 5.
  //
  // So the two accepted pairs are NAMED here rather than the check being
  // weakened, which keeps it able to catch an accidental copy-paste -- the
  // failure this exists for -- while recording that these two are deliberate.
  It(renders_no_two_characters_the_same_except_the_named_conventions) {
    const std::pair<char, char> kAccepted[] = {{'O', '0'}, {'S', '5'}};
    const auto accepted = [&](char a, char b) {
      for (const auto &p : kAccepted)
        if ((p.first == a && p.second == b) || (p.first == b && p.second == a))
          return true;
      return false;
    };

    const std::string all =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.-/<>[]=";
    for (std::size_t i = 0; i < all.size(); ++i)
      for (std::size_t j = i + 1; j < all.size(); ++j) {
        if (accepted(all[i], all[j]))
          continue;
        Assert::That(glyphs::PixelGlyph(all[i]) == glyphs::PixelGlyph(all[j]),
                     IsFalse());
      }
  };
};
