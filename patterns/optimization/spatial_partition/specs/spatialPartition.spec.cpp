#include <igloo/igloo_alt.h>

#include <algorithm>
#include <cstddef>
#include <vector>

#include "../src/partition/grid.h"

using namespace igloo;
using namespace partition;

// ── WHAT THESE SPECS PIN, AND WHY ─────────────────────────────────────────────
//
// The partition's silent failure is a STALE BUCKET: an entity that crossed into a
// new cell but was never taken out of the old one is still found where it used to
// be, and not found where it is. Nothing crashes. So the cases below are almost all
// about the CROSSING -- placement after a move, not only after an insert -- plus the
// invariant that makes the answers trustworthy: every entity is in exactly one
// bucket, ever.
//
// `Contains` is deliberately a linear search of a bucket through the public API, so a
// spec states what a caller would see rather than reaching into storage.
static bool Contains(const std::vector<int> &bucket, int id) {
    return std::find(bucket.begin(), bucket.end(), id) != bucket.end();
}

Describe(SpatialPartitionSpec) {

  // ── 1. THE LATTICE ──────────────────────────────────────────────────────────
  Describe(TheGrid) {
    It(has_the_cells_per_side_it_was_built_with) {
      Grid grid(10, 0);
      Assert::That(grid.CellsPerSide(), Equals(10));
      Assert::That(grid.CellCount(), Equals(std::size_t{100}));
    };

    It(maps_the_corners_to_the_corner_cells) {
      Grid grid(10, 0);
      Assert::That(grid.CellIndex(0.0f, 0.0f), Equals(0));
      Assert::That(grid.CellIndex(0.99f, 0.99f), Equals(99));
    };

    It(honours_the_cell_boundaries) {
      Grid grid(10, 0);
      Assert::That(grid.CellIndex(0.05f, 0.05f), Equals(0));
      Assert::That(grid.CellIndex(0.15f, 0.05f), Equals(1));
      Assert::That(grid.CellIndex(0.05f, 0.15f), Equals(10));
    };

    // ⚠️ OUT OF THE WORLD IS NOT CLAMPED. A position outside [0,1) belongs to no cell.
    It(refuses_a_position_outside_the_world) {
      Grid grid(10, 0);
      Assert::That(grid.CellIndex(1.0f, 0.5f), Equals(-1));
      Assert::That(grid.CellIndex(-0.01f, 0.5f), Equals(-1));
      Assert::That(grid.CellIndex(0.5f, 1.5f), Equals(-1));
    };

    It(answers_an_empty_query_for_an_out_of_world_position) {
      Grid grid(10, 4);
      grid.Insert(0, 0.5f, 0.5f);
      Assert::That(grid.CandidatesAt(2.0f, 0.5f).empty(), IsTrue());
    };
  };

  // ── 2. INSERT AND FIND ──────────────────────────────────────────────────────
  Describe(Insert) {
    It(makes_the_entity_findable_at_its_position) {
      Grid grid(10, 4);
      Assert::That(grid.Insert(2, 0.55f, 0.65f), IsTrue());
      Assert::That(Contains(grid.CandidatesAt(0.55f, 0.65f), 2), IsTrue());
      Assert::That(grid.CellOf(2), Equals(grid.CellIndex(0.55f, 0.65f)));
    };

    It(refuses_an_out_of_world_position) {
      Grid grid(10, 4);
      Assert::That(grid.Insert(1, 1.2f, 0.5f), IsFalse());
      Assert::That(grid.CellOf(1), Equals(-1));
    };

    It(refuses_an_unknown_entity_id) {
      Grid grid(10, 4);
      Assert::That(grid.Insert(9, 0.5f, 0.5f), IsFalse());
    };
  };

  // ── 3. THE CROSSING: THE OPERATION THIS PATTERN GETS WRONG ──────────────────
  Describe(Move) {
    It(leaves_the_old_bucket_when_the_entity_crosses_into_a_new_one) {
      Grid grid(10, 4);
      grid.Insert(0, 0.05f, 0.05f);          // cell 0
      const int oldCell = grid.CellOf(0);
      grid.Move(0, 0.25f, 0.05f);            // cell 2 -- a different bucket

      Assert::That(grid.CellOf(0), IsGreaterThan(oldCell));
      Assert::That(Contains(grid.CandidatesAt(0.05f, 0.05f), 0), IsFalse());  // ⚠️ not here
      Assert::That(Contains(grid.CandidatesAt(0.25f, 0.05f), 0), IsTrue());   // ...but here
    };

    It(reports_a_crossing_and_counts_it) {
      Grid grid(10, 4);
      grid.Insert(0, 0.05f, 0.05f);
      Assert::That(grid.Move(0, 0.25f, 0.05f), IsTrue());
      Assert::That(grid.Crossings(), Equals(std::size_t{1}));
      Assert::That(grid.Moves(), Equals(std::size_t{1}));
    };

    // ⚠️ THE COMMON CASE, AND IT MUST BE FREE. An entity that stays in its cell does no
    // bucket bookkeeping at all: the call counts, the crossing does not.
    It(costs_no_crossing_when_the_entity_stays_in_its_cell) {
      Grid grid(10, 4);
      grid.Insert(0, 0.02f, 0.02f);
      Assert::That(grid.Move(0, 0.06f, 0.06f), IsFalse());
      Assert::That(grid.Crossings(), Equals(std::size_t{0}));
      Assert::That(grid.Moves(), Equals(std::size_t{1}));
      Assert::That(Contains(grid.CandidatesAt(0.02f, 0.02f), 0), IsTrue());
    };

    It(refuses_a_move_out_of_the_world_and_leaves_the_entity_where_it_was) {
      Grid grid(10, 4);
      grid.Insert(0, 0.05f, 0.05f);
      Assert::That(grid.Move(0, 5.0f, 0.05f), IsFalse());
      Assert::That(Contains(grid.CandidatesAt(0.05f, 0.05f), 0), IsTrue());
    };

    It(places_an_entity_that_moves_before_it_was_inserted) {
      Grid grid(10, 4);
      Assert::That(grid.Move(0, 0.25f, 0.25f), IsTrue());     // move into the world
      Assert::That(grid.CellOf(0), Equals(grid.CellIndex(0.25f, 0.25f)));
      Assert::That(Contains(grid.CandidatesAt(0.25f, 0.25f), 0), IsTrue());
    };
  };

  // ── 4. REMOVE ───────────────────────────────────────────────────────────────
  Describe(Remove) {
    It(takes_the_entity_out_of_its_bucket) {
      Grid grid(10, 4);
      grid.Insert(1, 0.5f, 0.5f);
      Assert::That(grid.Remove(1), IsTrue());
      Assert::That(Contains(grid.CandidatesAt(0.5f, 0.5f), 1), IsFalse());
      Assert::That(grid.CellOf(1), Equals(-1));
    };

    It(is_idempotent) {
      Grid grid(10, 4);
      grid.Insert(1, 0.5f, 0.5f);
      grid.Remove(1);
      Assert::That(grid.Remove(1), IsFalse());
      Assert::That(grid.CandidatesAt(0.5f, 0.5f).empty(), IsTrue());
    };

    It(frees_the_slot_for_a_later_insert) {
      Grid grid(10, 4);
      grid.Insert(1, 0.5f, 0.5f);
      grid.Remove(1);
      Assert::That(grid.Insert(1, 0.2f, 0.2f), IsTrue());
      Assert::That(Contains(grid.CandidatesAt(0.2f, 0.2f), 1), IsTrue());
    };
  };

  // ── 5. THE NEIGHBOR QUERY ───────────────────────────────────────────────────
  Describe(CandidatesAround) {
    It(returns_everything_in_the_surrounding_block) {
      Grid grid(10, 16);
      grid.Insert(0, 0.45f, 0.45f);   // centre cell (4,4)
      grid.Insert(1, 0.55f, 0.45f);   // its right neighbour
      grid.Insert(2, 0.45f, 0.55f);   // its lower neighbour
      grid.Insert(3, 0.05f, 0.05f);   // far away

      std::vector<int> out;
      grid.CandidatesAround(0.45f, 0.45f, 1, out);
      Assert::That(out.size(), Equals(std::size_t{3}));
      Assert::That(Contains(out, 0), IsTrue());
      Assert::That(Contains(out, 1), IsTrue());
      Assert::That(Contains(out, 2), IsTrue());
      Assert::That(Contains(out, 3), IsFalse());
    };

    // ⚠️ NO DUPLICATES, BY CONSTRUCTION: an entity is in one bucket, and the block
    // visits each bucket once. A duplicate here would mean the placement is broken.
    It(never_returns_the_same_entity_twice) {
      Grid grid(10, 32);
      for (int i = 0; i < 32; ++i)
        grid.Insert(i, 0.5f, 0.5f);          // all in ONE cell, the worst case
      std::vector<int> out;
      grid.CandidatesAround(0.5f, 0.5f, 1, out);
      std::sort(out.begin(), out.end());
      Assert::That(std::adjacent_find(out.begin(), out.end()), Equals(out.end()));
      Assert::That(out.size(), Equals(std::size_t{32}));
    };

    It(clamps_its_block_at_the_world_edge) {
      Grid grid(10, 4);
      grid.Insert(0, 0.01f, 0.01f);          // corner cell (0,0)
      std::vector<int> out;
      grid.CandidatesAround(0.01f, 0.01f, 1, out);   // 2x2 of the block is off-world
      Assert::That(out.size(), Equals(std::size_t{1}));
      Assert::That(out[0], Equals(0));
    };

    It(returns_nothing_for_an_out_of_world_probe) {
      Grid grid(10, 4);
      grid.Insert(0, 0.5f, 0.5f);
      std::vector<int> out;
      out.push_back(99);
      grid.CandidatesAround(-1.0f, 0.5f, 1, out);
      Assert::That(out.empty(), IsTrue());
    };
  };

  // ── 6. THE INVARIANT EVERYTHING ELSE RELIES ON ──────────────────────────────
  //
  // ⚠️ ONE ENTITY, ONE BUCKET, THROUGH ANY SEQUENCE OF MOVES. This is the property
  // that makes a duplicate impossible and a missing entity a bug rather than a
  // question. If a stale-bucket bug existed, this is the case that would eventually
  // catch it: the bucket sizes would not add up to the entities in the grid.
  Describe(ThePlacementInvariant) {
    It(sums_to_the_number_of_entities_in_the_grid_after_churn) {
      const int kEntities = 24;
      Grid grid(6, kEntities);
      for (int i = 0; i < kEntities; ++i)
        grid.Insert(i, 0.01f * i, 0.01f * i);

      // 200 moves along a bouncing path, so many but not all cross a boundary.
      std::vector<float> x(kEntities, 0.0f), y(kEntities, 0.0f), vx(kEntities, 0.013f), vy(kEntities, 0.017f);
      for (int e = 0; e < kEntities; ++e) { x[e] = 0.05f + 0.01f * e; y[e] = 0.5f; }
      for (int step = 0; step < 200; ++step) {
        for (int e = 0; e < kEntities; ++e) {
          x[e] += vx[e]; y[e] += vy[e];
          if (x[e] < 0.0f || x[e] >= 1.0f) { vx[e] = -vx[e]; x[e] += vx[e]; }
          if (y[e] < 0.0f || y[e] >= 1.0f) { vy[e] = -vy[e]; y[e] += vy[e]; }
          grid.Move(e, x[e], y[e]);
        }
      }

      std::size_t inBuckets = 0;
      for (int cell = 0; cell < static_cast<int>(grid.CellCount()); ++cell)
        inBuckets += grid.BucketSize(cell);
      Assert::That(inBuckets, Equals(std::size_t{kEntities}));

      // And every live entity is findable at its current position.
      for (int e = 0; e < kEntities; ++e)
        Assert::That(Contains(grid.CandidatesAt(x[e], y[e]), e), IsTrue());
    };

    It(sum_of_buckets_still_holds_after_removals) {
      Grid grid(4, 8);
      for (int i = 0; i < 8; ++i)
        grid.Insert(i, 0.1f, 0.1f);          // all in cell 0
      grid.Remove(0);
      grid.Remove(3);
      grid.Remove(7);
      std::size_t inBuckets = 0;
      for (int cell = 0; cell < static_cast<int>(grid.CellCount()); ++cell)
        inBuckets += grid.BucketSize(cell);
      Assert::That(inBuckets, Equals(std::size_t{5}));
    };
  };

  // ⚠️ A GRID WITH NO CELLS IS THE EDGE THE INDEX ARITHMETIC GETS WRONG. It must
  // answer "no cell" rather than divide by zero.
  Describe(ADegenerateGrid) {
    It(refuses_every_position_and_never_counts_a_cell) {
      Grid grid(0, 4);
      Assert::That(grid.CellIndex(0.5f, 0.5f), Equals(-1));
      Assert::That(grid.CellCount(), Equals(std::size_t{0}));
      Assert::That(grid.Insert(0, 0.5f, 0.5f), IsFalse());
      Assert::That(grid.CandidatesAt(0.5f, 0.5f).empty(), IsTrue());
    };
  };
};