#include <igloo/igloo_alt.h>

#include <cstddef>
#include <vector>

#include "../src/dirty/transformTree.h"

using namespace igloo;
using namespace dirty;

// ── THE GROUND TRUTH IS WALKED, NOT ASKED FOR ────────────────────────────────
//
// The whole danger of this pattern is a SILENT stale answer: a missed MarkDirty
// gives a wrong number with no crash and no warning. So the specs do not compare the
// cached value against another cached value -- they walk the parent chain and sum the
// LOCAL offsets by hand, using only public accessors. If the cache and the walk
// disagree, the cache is stale, and that is the failure this pattern actually has.
static float WalkedX(const TransformTree &tree, int node) {
    float sum = 0.0f;
    for (int i = node; i >= 0; i = tree.Parent(i))
        sum += tree.LocalX(i);
    return sum;
}
static float WalkedY(const TransformTree &tree, int node) {
    float sum = 0.0f;
    for (int i = node; i >= 0; i = tree.Parent(i))
        sum += tree.LocalY(i);
    return sum;
}

// A tree of a known shape: one root, three children, two grandchildren each.
// 1 + 3 + 6 = 10 nodes. Every spec that counts work uses this so the arithmetic in
// the assertion is the arithmetic of the fixture.
namespace {
constexpr int kRoot = 0;

struct Tree {
    static int Build(TransformTree &tree) {
        const int root = tree.AddRoot(0.0f, 0.0f);
        for (int c = 0; c < 3; ++c) {
            const int child = tree.AddChild(root, 10.0f * (c + 1), 1.0f * c);
            for (int g = 0; g < 2; ++g)
                tree.AddChild(child, 1.0f + static_cast<float>(g), 2.0f);
        }
        return root;
    }
};

// Every node's cached value must equal the hand-walked one.
//
// ⚠️ NOT `const`, AND THAT IS THE PATTERN RATHER THAN AN OVERSIGHT: a read of a
// deferred value IS a mutation -- it brings the cache up to date and counts the
// work. A `const WorldX` would have to either resolve nothing (returning a stale
// value, the exact bug this pattern has) or mutate through the const.
static void AssertWholeTreeIsFresh(TransformTree &tree) {
    for (std::size_t i = 0; i < tree.size(); ++i) {
        Assert::That(tree.WorldX(static_cast<int>(i)),
                     Equals(WalkedX(tree, static_cast<int>(i))));
        Assert::That(tree.WorldY(static_cast<int>(i)),
                     Equals(WalkedY(tree, static_cast<int>(i))));
    }
}
} // namespace

Describe(DirtyFlagSpec) {

  // ── 1. A VALUE THAT WAS NEVER COMPUTED IS NOT CLEAN ────────────────────────
  Describe(ANewNodeIsAlreadyDirty) {
    It(reports_dirty_before_anything_reads_it) {
      TransformTree tree;
      Tree::Build(tree);
      Assert::That(tree.DirtyCount(), Equals(static_cast<std::size_t>(10)));
      Assert::That(tree.IsDirty(kRoot), IsTrue());
    };

    It(does_the_work_on_the_first_read_and_no_more) {
      TransformTree tree;
      Tree::Build(tree);
      tree.ResetCounters();

      Assert::That(tree.WorldX(kRoot), Equals(0.0f));
      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(1)));

      tree.WorldX(kRoot);
      tree.WorldX(kRoot);
      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(1)));
      Assert::That(tree.Reads(), Equals(static_cast<std::size_t>(3)));
    };
  };

  // ── 2. A CHANGE COSTS A MARK, NOT A WALK ───────────────────────────────────
  Describe(AChangeOnlyMarks) {
    It(makes_a_value_stale_without_computing_anything) {
      TransformTree tree;
      Tree::Build(tree);
      for (std::size_t i = 0; i < tree.size(); ++i)
        tree.WorldX(static_cast<int>(i));
      tree.ResetCounters();

      tree.SetLocal(kRoot, 5.0f, 0.0f);
      // ⚠️ NO ARITHMETIC YET. The cost of a change does not scale with how much
      // depends on it -- that is the entire pattern, and it is why the counter is
      // the way to see it.
      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(0)));
      Assert::That(tree.Sets(), Equals(static_cast<std::size_t>(1)));
      Assert::That(tree.DirtyCount(), Equals(static_cast<std::size_t>(10)));

      Assert::That(tree.WorldX(kRoot), Equals(5.0f));
      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(1)));
    };

    It(is_idempotent_so_marking_twice_does_not_double_the_work) {
      TransformTree tree;
      Tree::Build(tree);
      for (std::size_t i = 0; i < tree.size(); ++i)
        tree.WorldX(static_cast<int>(i));
      tree.ResetCounters();

      tree.SetLocal(kRoot, 1.0f, 0.0f);
      tree.SetLocal(kRoot, 2.0f, 0.0f);
      tree.SetLocal(kRoot, 3.0f, 0.0f);
      Assert::That(tree.DirtyCount(), Equals(static_cast<std::size_t>(10)));

      Assert::That(tree.WorldX(kRoot), Equals(3.0f));
      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(1)));
      Assert::That(tree.Sets(), Equals(static_cast<std::size_t>(3)));
    };
  };

  // ── 3. A PARENT'S MOVE REACHES EVERY DESCENDANT, LAZILY ────────────────────
  Describe(DirtinessFlowsDown) {
    It(moves_the_whole_tree_when_the_root_moves) {
      TransformTree tree;
      Tree::Build(tree);
      AssertWholeTreeIsFresh(tree);

      tree.SetLocal(kRoot, 100.0f, 50.0f);

      // Every node's cached value was invalidated by ONE mark, and every node now
      // agrees with a fresh walk.
      AssertWholeTreeIsFresh(tree);
      for (std::size_t i = 1; i < tree.size(); ++i)
        Assert::That(tree.WorldX(static_cast<int>(i)),
                     Equals(WalkedX(tree, static_cast<int>(i))));
    };

    // ⚠️ THE OTHER DIRECTION IS A BUG WAITING TO HAPPEN: a leaf's own move must NOT
    // invalidate its ancestors, or every edit dirties the world.
    It(does_not_dirty_the_ancestors_of_a_changed_leaf) {
      TransformTree tree;
      Tree::Build(tree);
      for (std::size_t i = 0; i < tree.size(); ++i)
        tree.WorldX(static_cast<int>(i));

      const int leaf = 9;                       // a grandchild
      tree.SetLocal(leaf, 7.0f, 7.0f);

      Assert::That(tree.IsDirty(leaf), IsTrue());
      Assert::That(tree.IsDirty(kRoot), IsFalse());       // ancestors stay clean
      Assert::That(tree.IsDirty(tree.Parent(leaf)), IsFalse());
      Assert::That(tree.DirtyCount(), Equals(static_cast<std::size_t>(1)));
    };
  };

  // ── 4. THE PAYOFF, MEASURED: ONLY THE STALE PART IS REDONE ─────────────────
  Describe(OnlyTheStalePartIsRedone) {
    It(recomputes_one_branch_and_not_the_other) {
      TransformTree tree;
      Tree::Build(tree);
      for (std::size_t i = 0; i < tree.size(); ++i)
        tree.WorldX(static_cast<int>(i));
      tree.ResetCounters();

      // Branch 1 = child 1 and its two grandchildren (3 of 10 nodes).
      const int child1 = 1;
      tree.SetLocal(child1, 42.0f, 0.0f);

      for (std::size_t i = 0; i < tree.size(); ++i)
        tree.WorldX(static_cast<int>(i));

      // ⚠️ THREE, NOT TEN. This is the pattern working: nine nodes were read and
      // only the three under the change were rebuilt.
      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(3)));
      Assert::That(tree.Reads(), Equals(static_cast<std::size_t>(10)));
      AssertWholeTreeIsFresh(tree);
    };

    // ⚠️ AND THIS IS THE DEMO'S PUNCHLINE AS A SPEC: when nothing changes, reading
    // the world costs nothing but the reads. The flag is what stands between "the
    // scene was drawn" and "the scene was recomputed".
    It(costs_nothing_at_all_when_nothing_changed) {
      TransformTree tree;
      Tree::Build(tree);
      for (std::size_t i = 0; i < tree.size(); ++i)
        tree.WorldX(static_cast<int>(i));
      tree.ResetCounters();

      for (int frame = 0; frame < 100; ++frame)
        for (std::size_t i = 0; i < tree.size(); ++i)
          tree.WorldX(static_cast<int>(i));

      Assert::That(tree.Recomputes(), Equals(static_cast<std::size_t>(0)));
      Assert::That(tree.Reads(), Equals(static_cast<std::size_t>(1000)));
    };
  };

  // ── 5. THE SILENT-WRONG-ANSWER SPEC ───────────────────────────────────────
  // A missed mark is invisible, so this case mutates, reads everything, mutates
  // again, and reads everything again -- comparing against the walked truth each
  // time. It is the case that would catch a `SetLocal` that forgot to propagate.
  Describe(TheCacheNeverLies) {
    It(agrees_with_the_walked_truth_through_many_mutations) {
      TransformTree tree;
      Tree::Build(tree);

      for (int round = 0; round < 12; ++round) {
        const int victim = round % static_cast<int>(tree.size());
        tree.SetLocal(victim, static_cast<float>(round) * 1.5f, -1.0f * static_cast<float>(round));
        AssertWholeTreeIsFresh(tree);
      }
      Assert::That(tree.Recomputes() > 0, IsTrue());
    };
  };

  // ── 6. THE TRUST BOUNDARY AT THE INDEX ────────────────────────────────────
  Describe(IndicesAreGuarded) {
    It(refuses_a_child_whose_parent_does_not_exist) {
      TransformTree tree;
      const int root = tree.AddRoot(0.0f, 0.0f);
      Assert::That(tree.AddChild(-1, 1.0f, 1.0f), Equals(-1));
      Assert::That(tree.AddChild(99, 1.0f, 1.0f), Equals(-1));
      Assert::That(tree.size(), Equals(static_cast<std::size_t>(1)));
      Assert::That(tree.AddChild(root, 1.0f, 1.0f) >= 0, IsTrue());
    };

    It(answers_a_read_past_the_end_without_crashing) {
      TransformTree tree;
      Tree::Build(tree);
      Assert::That(tree.WorldX(99), Equals(0.0f));
      Assert::That(tree.WorldY(-1), Equals(0.0f));
      Assert::That(tree.IsDirty(99), IsFalse());
      tree.SetLocal(99, 1.0f, 1.0f);            // ignored, and does not count
      Assert::That(tree.Sets(), Equals(static_cast<std::size_t>(0)));
    };
  };
};
