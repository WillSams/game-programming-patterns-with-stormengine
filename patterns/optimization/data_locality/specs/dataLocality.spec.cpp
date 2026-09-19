#include <igloo/igloo_alt.h>

#include <cstring>
#include <string>
#include <vector>

#include "../src/locality/particles.h"

using namespace igloo;
using namespace locality;

// ── WHAT IS BEING PINNED HERE, AND WHY IT IS NOT THE SPEED ───────────────────
//
// A test cannot assert that memory is faster: that belongs to a machine, not to a
// suite, and a timing assertion is a flake generator. So these specs pin the two
// deterministic halves of the pattern --
//
//   * the optimization DOES NOT CHANGE THE WORLD (both layouts agree), and
//   * the LAYOUT IS WHAT THE PATTERN CLAIMS (the hot struct is exactly its hot
//     fields, elements are one struct apart, and a hot update leaves the cold array
//     byte-identical).
//
// The last of those is the actual claim of the pattern -- "the hot loop did not drag
// cold data in" -- checked rather than asserted. The demo times the two; it does not
// decide correctness.
namespace {

// The same set of particles, laid out both ways. One builder, so the two worlds
// cannot be given different data by a typo -- which would make the comparison spec
// pass or fail for the wrong reason.
struct Fixture {
    static constexpr std::size_t kCount = 32;

    // ⚠️ THE COUNT COMES FROM THE WORLD, NOT FROM kCount, AND THAT IS A FIX.
    // The first version wrote its own kCount (32) elements into whatever it was
    // handed, so a case that allocated four-element worlds got a 32-element WRITE --
    // out of bounds, heap corruption, and a segfault with no failing assertion to
    // point at it. Deriving the count means the two can no longer disagree, and the
    // assertion makes a disagreement loud if a future case builds them unevenly.
    static void Build(MixedWorld &mixed, SplitWorld &split) {
        Assert::That(mixed.size(), Equals(split.size()));
        const std::size_t count = mixed.size();
        for (std::size_t i = 0; i < count; ++i) {
            const float f = static_cast<float>(i);
            const float x = 4.0f * (f - static_cast<float>(count) / 2.0f);
            const float y = 0.5f * f;
            const float vx = 0.25f * f - 1.0f;
            const float vy = -0.125f * f;
            const std::string name = "P" + std::to_string(i);
            mixed.Set(i, x, y, vx, vy, name);
            split.Set(i, x, y, vx, vy, name);
        }
    }
};

} // namespace

Describe(DataLocalitySpec) {

  // ── 1. THE OPTIMIZATION DOES NOT CHANGE THE RESULT ─────────────────────────
  Describe(TheLayoutDoesNotChangeTheWorld) {
    It(agrees_after_stepping_both_layouts) {
      MixedWorld mixed{Fixture::kCount};
      SplitWorld split{Fixture::kCount};
      Fixture::Build(mixed, split);

      const View view;
      for (int step = 0; step < 10; ++step) {
        mixed.Step(1.0f / 60.0f, view);
        split.Step(1.0f / 60.0f, view);
      }
      Assert::That(SameWorld(mixed, split, 1e-4f), IsTrue());
    };

    // The bounce is a branch on the hot data, and it must behave the same way in
    // both layouts -- this is where a "packed" version most easily diverges.
    It(agrees_through_a_bounce_off_the_view_edge) {
      MixedWorld mixed{2};
      SplitWorld split{2};
      const View view;                       // minX -50, maxX 50
      mixed.Set(0, 49.0f, 0.0f, 40.0f, 0.0f, "A");
      split.Set(0, 49.0f, 0.0f, 40.0f, 0.0f, "A");
      mixed.Set(1, -49.0f, 0.0f, -40.0f, 0.0f, "B");
      split.Set(1, -49.0f, 0.0f, -40.0f, 0.0f, "B");

      mixed.Step(0.1f, view);                // 49 + 4.0 -> past 50, must clamp
      split.Step(0.1f, view);

      Assert::That(mixed.at(0).x, Equals(50.0f));
      Assert::That(split.hotAt(0).x, Equals(50.0f));
      Assert::That(mixed.at(0).vx, Equals(-40.0f));
      Assert::That(split.hotAt(0).vx, Equals(-40.0f));
      Assert::That(SameWorld(mixed, split, 1e-4f), IsTrue());
    };

    // ⚠️ AND THE COMPARISON ITSELF IS NOT VACUOUS. A `SameWorld` that always
    // returned true would make every case above meaningless, so the comparator is
    // required to REJECT a world that differs -- by count, and by position.
    It(the_comparison_rejects_different_worlds) {
      MixedWorld mixed{Fixture::kCount};
      SplitWorld split{Fixture::kCount};
      Fixture::Build(mixed, split);
      Assert::That(SameWorld(mixed, split, 1e-4f), IsTrue());   // agrees first

      split.hotAt(2).x += 1.0f;
      Assert::That(SameWorld(mixed, split, 1e-4f), IsFalse());  // rejects a move

      SplitWorld shorter{Fixture::kCount - 1};
      Assert::That(SameWorld(mixed, shorter, 1e-4f), IsFalse()); // rejects a count
    };
  };

  // ── 2. THE LAYOUT IS WHAT THE PATTERN CLAIMS ──────────────────────────────
  Describe(TheLayoutIsWhatItClaims) {
    // Every byte here is a byte fetched for EVERY element, so a pad byte or a
    // stray field is paid for across the whole array.
    It(the_hot_struct_is_exactly_its_four_floats) {
      Assert::That(sizeof(ParticleHot), Equals(4 * sizeof(float)));
    };

    // The pattern's claim in bytes: the mixed struct carries its cold fields in
    // the same memory the hot loop walks, so a cache line of the mixed array holds
    // mostly data the loop will not read.
    It(the_hot_struct_is_much_smaller_than_the_mixed_one) {
      Assert::That(sizeof(ParticleHot) < sizeof(ParticleMixed), IsTrue());
    };

    // The contiguity claim, measured rather than assumed: consecutive elements are
    // exactly one struct apart -- no padding, no indirection, no gap.
    It(elements_are_exactly_one_struct_apart) {
      SplitWorld split{4};
      const char *first = reinterpret_cast<const char *>(&split.hotAt(0));
      const char *second = reinterpret_cast<const char *>(&split.hotAt(1));
      Assert::That(second - first, Equals(static_cast<std::ptrdiff_t>(sizeof(ParticleHot))));
    };

    // ⚠️ THE ACTUAL CLAIM OF THE PATTERN, CHECKED RATHER THAN TRUSTED: the hot loop
    // must not touch the cold side at all. If it did, the split would have bought
    // nothing -- and it would show up here as a changed byte, not as a slower timer.
    It(leaves_the_cold_array_byte_identical_after_a_hot_update) {
      SplitWorld split{Fixture::kCount};
      MixedWorld mixed{Fixture::kCount};
      Fixture::Build(mixed, split);

      const std::vector<unsigned char> before(
          split.ColdBytes(), split.ColdBytes() + split.ColdBytesSize());

      const View view;
      for (int step = 0; step < 20; ++step)
        split.Step(1.0f / 60.0f, view);

      Assert::That(split.ColdBytesSize(), Equals(before.size()));
      Assert::That(std::memcmp(before.data(), split.ColdBytes(), before.size()),
                   Equals(0));
    };

    // A cold flag says nothing about the simulation, so flipping it must not move
    // anything -- the split must not have made hot and cold secretly interdependent.
    It(is_unaffected_by_a_cold_only_flag) {
      MixedWorld mixed{Fixture::kCount};
      SplitWorld splitA{Fixture::kCount}, splitB{Fixture::kCount};
      Fixture::Build(mixed, splitA);
      Fixture::Build(mixed, splitB);

      // The ONLY way to change a cold field: a named setter. There is no non-const
      // `coldAt`, which is what keeps the hot loop from acquiring a stray write.
      splitB.SetVisible(0, false);
      Assert::That(splitB.coldAt(0).visible, IsFalse());
      Assert::That(splitA.coldAt(0).visible, IsTrue());

      const View view;
      for (int step = 0; step < 5; ++step) {
        mixed.Step(0.5f, view);     // ⚠️ EVERY world under comparison must be
        splitA.Step(0.5f, view);    // stepped. The first version stepped the two
        splitB.Step(0.5f, view);    // split worlds and compared against a `mixed`
      }                             // that had never moved -- a spec bug, and the
                                    // failure was "the layouts disagree".
      // Same motion, and still the same motion as the MIXED layout.
      Assert::That(SameWorld(mixed, splitB, 1e-4f), IsTrue(),
                   "a cold flag must not move a particle");
      Assert::That(SameWorld(mixed, splitA, 1e-4f), IsTrue());
    };
  };

  // ── 3. THE HOT LOOP VISITS EVERY ELEMENT, EXACTLY ONCE ────────────────────
  // The guard for the classic layout bug: a stride that skips elements, or an
  // index range that double-steps the last one. Both would show up in a timing
  // benchmark as a FASTER loop, which is the worst possible way to find it.
  Describe(EveryElementIsSteppedExactlyOnce) {
    It(counts_one_visit_per_element_per_step) {
      constexpr std::size_t kCount = 50;
      SplitWorld split{kCount};
      for (std::size_t i = 0; i < kCount; ++i)
        split.Set(i, 0.0f, 0.0f, 1.0f, 0.0f, "P");

      const View view;
      for (int step = 0; step < 7; ++step)
        split.Step(1.0f, view);

      for (std::size_t i = 0; i < kCount; ++i)
        Assert::That(split.visitsAt(i), Equals(7));
    };

    It(does_not_disturb_elements_beyond_the_end_of_a_short_world) {
      SplitWorld split{1};
      split.Set(0, 0.0f, 0.0f, 1.0f, 0.0f, "ONLY");
      const View view;
      split.Step(1.0f, view);
      Assert::That(split.size(), Equals(static_cast<std::size_t>(1)));
      Assert::That(split.visitsAt(0), Equals(1));
    };
  };

  // ── 4. THE COST OF THE OPTIMIZATION, NOT HIDDEN ───────────────────────────
  // The two layouts are not interchangeable: a split world's cold data must be
  // reunited with its hot data through a SHARED INDEX. The pattern buys locality
  // with an index and a split, and the demo draws from the cold array to show it.
  Describe(ReunitingTheLayoutsNeedsTheIndex) {
    It(copies_the_hot_fields_and_leaves_the_cold_ones_behind) {
      MixedWorld mixed{2};
      SplitWorld split{2};
      split.Set(0, 3.0f, 4.0f, 5.0f, 6.0f, "NAMED");
      split.Set(1, 7.0f, 8.0f, 9.0f, 10.0f, "OTHER");

      CopyHotFromSplit(split, 0, mixed);

      Assert::That(mixed.at(0).x, Equals(3.0f));
      Assert::That(mixed.at(0).vy, Equals(6.0f));
      Assert::That(mixed.at(0).name, Equals(std::string("")));   // cold, NOT copied
      Assert::That(split.coldAt(0).name, Equals(std::string("NAMED")));
      Assert::That(mixed.at(1).x, Equals(0.0f));                 // only index 0
    };
  };
};
