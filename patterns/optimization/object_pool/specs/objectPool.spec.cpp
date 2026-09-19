#include <igloo/igloo_alt.h>

#include <cstddef>

#include "../src/pool/particlePool.h"

using namespace igloo;
using namespace pool;

// ── WHAT THESE SPECS PIN, AND WHY ─────────────────────────────────────────────
//
// A pool's failure modes are all quiet. It can grow when it was supposed to be
// fixed, overwrite a live object when it runs dry, or hand the same slot to two
// callers at once -- and every one of those produces a program that runs and draws
// something. So the cases below check the CONTRACT rather than the arithmetic:
// capacity never moves, exhaustion refuses instead of clobbering, and the free
// list survives a double release.
//
// The last group is the one the pattern is actually about: the bytes of a returned
// object are still there, so a spec proves that what comes back is the NEW particle
// and not the ghost of the old one.

Describe(ObjectPoolSpec) {

  // ── 1. THE POOL IS FIXED ────────────────────────────────────────────────────
  Describe(TheCapacityIsACommitment) {
    It(has_the_capacity_it_was_built_with) {
      ParticlePool pool(8);
      Assert::That(pool.Capacity(), Equals(std::size_t{8}));
    };

    It(does_not_grow_when_a_create_is_refused) {
      ParticlePool pool(2);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);   // refused
      Assert::That(pool.Capacity(), Equals(std::size_t{2}));
      Assert::That(pool.AliveCount(), Equals(std::size_t{2}));
    };

    It(starts_empty_with_every_slot_free) {
      ParticlePool pool(5);
      Assert::That(pool.AliveCount(), Equals(std::size_t{0}));
      Assert::That(pool.FreeCount(), Equals(std::size_t{5}));
    };
  };

  // ── 2. EXHAUSTION IS REFUSED, NEVER OVERWRITTEN ─────────────────────────────
  Describe(AFullPoolRefuses) {
    It(returns_minus_one_and_counts_the_refusal) {
      ParticlePool pool(1);
      Assert::That(pool.Create(1.0f, 2.0f, 3.0f, 4.0f, 5.0f), Equals(0));
      Assert::That(pool.Create(9.0f, 9.0f, 9.0f, 9.0f, 9.0f), Equals(-1));
      Assert::That(pool.Refused(), Equals(std::size_t{1}));
    };

    // ⚠️ THE REFUSED SPAWN MUST NOT TOUCH THE LIVE ONE. A pool that recycles the
    // oldest object to make room looks like it is working -- bursts keep appearing
    // -- while it silently cuts the life of something on screen.
    It(leaves_the_live_particle_exactly_as_it_was) {
      ParticlePool pool(1);
      const int slot = pool.Create(1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
      pool.Create(9.0f, 9.0f, 9.0f, 9.0f, 9.0f);
      const Particle &p = pool.At(slot);
      Assert::That(p.x, Equals(1.0f));
      Assert::That(p.y, Equals(2.0f));
      Assert::That(p.vx, Equals(3.0f));
      Assert::That(p.vy, Equals(4.0f));
      Assert::That(p.life, Equals(5.0f));
      Assert::That(p.maxLife, Equals(5.0f));
      Assert::That(p.alive, IsTrue());
    };
  };

  // ── 3. ANIMATE ──────────────────────────────────────────────────────────────
  Describe(Animate) {
    It(moves_a_live_particle_by_its_velocity_times_dt) {
      ParticlePool pool(1);
      const int slot = pool.Create(0.0f, 0.0f, 10.0f, -4.0f, 1.0f);
      pool.Animate(0.5f);
      Assert::That(pool.At(slot).x, Equals(5.0f));
      Assert::That(pool.At(slot).y, Equals(-2.0f));
    };

    It(skips_a_slot_that_is_not_alive) {
      ParticlePool pool(2);
      const int slot = pool.Create(0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
      pool.Release(slot);
      pool.Animate(0.5f);
      // Released, so its position is whatever it had -- never advanced again.
      Assert::That(pool.At(slot).x, Equals(0.0f));
    };

    It(releases_a_particle_whose_life_has_run_out) {
      ParticlePool pool(1);
      const int slot = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
      pool.Animate(0.2f);
      Assert::That(pool.IsAlive(slot), IsFalse());
      Assert::That(pool.AliveCount(), Equals(std::size_t{0}));
      Assert::That(pool.Released(), Equals(std::size_t{1}));
    };

    It(keeps_a_particle_alive_until_its_life_is_actually_gone) {
      ParticlePool pool(1);
      const int slot = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Animate(0.4f);
      pool.Animate(0.4f);
      Assert::That(pool.IsAlive(slot), IsTrue());   // 0.2 left
      pool.Animate(0.3f);
      Assert::That(pool.IsAlive(slot), IsFalse());
    };

    It(never_drives_life_negative) {
      ParticlePool pool(1);
      const int slot = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
      pool.Animate(100.0f);
      Assert::That(pool.At(slot).life, Equals(0.0f));
    };
  };

  // ── 4. THE POINT: A RETURNED SLOT COMES BACK ────────────────────────────────
  Describe(Reuse) {
    It(gives_a_freed_slot_to_the_next_create_and_says_so) {
      ParticlePool pool(1);
      const int first = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
      pool.Animate(0.2f);                       // dies, slot returns to the list
      const int second = pool.Create(5.0f, 5.0f, 0.0f, 0.0f, 1.0f);

      Assert::That(second, Equals(first));      // ⚠️ the same address, reused
      Assert::That(pool.Created(), Equals(std::size_t{2}));
      Assert::That(pool.Reused(), Equals(std::size_t{1}));
      Assert::That(pool.Capacity(), Equals(std::size_t{1}));
    };

    // ⚠️ THE GHOST. A live slot's bytes are overwritten by the new particle's -- if
    // they were not, the demo would draw the old particle with a new particle's
    // velocity, and nothing would report an error.
    It(leaves_no_trace_of_the_previous_particle) {
      ParticlePool pool(1);
      pool.Create(1.0f, 2.0f, 3.0f, 4.0f, 0.1f);
      pool.Animate(0.2f);
      const int slot = pool.Create(-7.0f, -8.0f, -9.0f, -10.0f, 20.0f);

      Assert::That(pool.At(slot).x, Equals(-7.0f));
      Assert::That(pool.At(slot).y, Equals(-8.0f));
      Assert::That(pool.At(slot).vx, Equals(-9.0f));
      Assert::That(pool.At(slot).maxLife, Equals(20.0f));
    };

    It(does_not_call_a_first_take_a_reuse) {
      ParticlePool pool(3);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      Assert::That(pool.Reused(), Equals(std::size_t{0}));
    };
  };

  // ── 5. THE FREE LIST SURVIVES MISTAKES ──────────────────────────────────────
  //
  // ⚠️ A DOUBLE RELEASE IS THE WAY TO CORRUPT A FREE LIST: pushing one slot twice
  // makes it its own successor, and the next creates hand the SAME slot to two
  // callers. The pool ignores the second release, so the list stays a set.
  Describe(Release) {
    It(is_ignored_when_the_slot_is_already_free) {
      ParticlePool pool(2);
      const int slot = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Release(slot);
      pool.Release(slot);                       // ignored
      Assert::That(pool.Released(), Equals(std::size_t{1}));
      Assert::That(pool.FreeCount(), Equals(std::size_t{2}));
    };

    It(still_hands_out_distinct_slots_after_a_double_release) {
      ParticlePool pool(2);
      const int a = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      const int b = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Release(a);
      pool.Release(a);                          // ignored, so `a` is on the list once
      const int c = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      const int d = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      Assert::That(c, Equals(a));               // ONE free slot, and `c` gets it
      Assert::That(c == b, IsFalse());          // ...not a second alias of `b`
      Assert::That(d, Equals(-1));              // nothing left
      Assert::That(pool.AliveCount(), Equals(std::size_t{2}));
    };
  };

  // ── 6. CLEAR, AND THE EMPTY POOL ────────────────────────────────────────────
  Describe(Clear) {
    It(frees_every_slot_without_changing_the_capacity) {
      ParticlePool pool(3);
      for (int i = 0; i < 3; ++i)
        pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Clear();
      Assert::That(pool.AliveCount(), Equals(std::size_t{0}));
      Assert::That(pool.FreeCount(), Equals(std::size_t{3}));
      Assert::That(pool.Capacity(), Equals(std::size_t{3}));
    };

    It(lets_every_slot_be_taken_again) {
      ParticlePool pool(2);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Clear();
      Assert::That(pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f), IsGreaterThan(-1));
      Assert::That(pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f), IsGreaterThan(-1));
      Assert::That(pool.AliveCount(), Equals(std::size_t{2}));
    };

    // ⚠️ Clear is about what is alive, not about history: a slot that has held a
    // particle is still a reused slot the next time it is taken.
    It(does_not_erase_the_reuse_history) {
      ParticlePool pool(1);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      pool.Clear();
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      Assert::That(pool.Reused(), Equals(std::size_t{1}));
    };
  };

  // ⚠️ A PARTICLE WITH NO LIFE IS NOT CREATABLE. Accepting it would store a slot that
  // is `alive` while already expired, which only `Animate` would ever clean up.
  Describe(ANonPositiveLife) {
    It(is_refused_and_touches_nothing) {
      ParticlePool pool(1);
      Assert::That(pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 0.0f), Equals(-1));
      Assert::That(pool.Create(0.0f, 0.0f, 0.0f, 0.0f, -1.0f), Equals(-1));
      Assert::That(pool.AliveCount(), Equals(std::size_t{0}));
      Assert::That(pool.FreeCount(), Equals(std::size_t{1}));
    };

    // It is a caller bug, not a full pool, so it must not be counted as exhaustion.
    It(is_not_counted_as_exhaustion) {
      ParticlePool pool(1);
      pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
      Assert::That(pool.Refused(), Equals(std::size_t{0}));
    };
  };

  // ⚠️ A ZERO-CAPACITY POOL IS THE EDGE THE HEAD/TAIL LOGIC GETS WRONG. It must
  // refuse everything rather than index a slot that is not there.
  Describe(AnEmptyPool) {
    It(refuses_every_create_and_reports_itself_empty) {
      ParticlePool pool(0);
      Assert::That(pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f), Equals(-1));
      Assert::That(pool.Capacity(), Equals(std::size_t{0}));
      Assert::That(pool.FreeCount(), Equals(std::size_t{0}));
      Assert::That(pool.Refused(), Equals(std::size_t{1}));
    };
  };

  // ── 7. THE BOOKKEEPING HOLDS UNDER CHURN ────────────────────────────────────
  //
  // A free-list bug does not usually show up on the first take and release -- it
  // shows up after a thousand, as a slot that is on the list twice or on it never.
  // These two invariants are cheap to check every step and are what a long run of
  // the fountain is actually relying on.
  Describe(Invariants) {
    It(keeps_alive_plus_free_equal_to_capacity_through_churn) {
      ParticlePool pool(5);
      for (int step = 0; step < 200; ++step) {
        if (step % 3 == 0)
          pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        else
          pool.Animate(0.5f);                 // expires whatever has run out
        Assert::That(pool.AliveCount() + pool.FreeCount(), Equals(pool.Capacity()));
      }
    };

    It(accounts_for_every_take_as_alive_or_released) {
      ParticlePool pool(4);
      for (int i = 0; i < 4; ++i)
        pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 0.1f);
      pool.Animate(0.2f);                     // all four return
      for (int i = 0; i < 4; ++i)
        pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 5.0f);   // all four reused

      Assert::That(pool.Created(), Equals(std::size_t{8}));
      Assert::That(pool.Reused(), Equals(std::size_t{4}));
      Assert::That(pool.Created(), Equals(pool.AliveCount() + pool.Released()));
    };

    It(counts_every_live_slot_exactly_once) {
      ParticlePool pool(6);
      int taken[6];
      for (int i = 0; i < 6; ++i)
        taken[i] = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
      for (int i = 0; i < 6; i += 2)
        pool.Release(taken[i]);
      for (int i = 0; i < 3; ++i)
        pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

      std::size_t live = 0;
      for (std::size_t i = 0; i < pool.Capacity(); ++i)
        if (pool.IsAlive(static_cast<int>(i)))
          ++live;
      Assert::That(live, Equals(pool.AliveCount()));
    };
  };

  // ── 8. THE CALLER OWNS ACCELERATION ────────────────────────────────────────
  //
  // ⚠️ THE POOL IS NOT A PARTICLE SYSTEM. It integrates the velocity it is handed and
  // owns life and reuse; a demo adds gravity. The mutable `At` is that seam, and this
  // is the contract it promises: whatever the caller does to the velocity is what
  // `Animate` then integrates.
  Describe(TheMutableAccessor) {
    It(lets_the_caller_apply_acceleration_before_the_integration) {
      ParticlePool pool(1);
      const int slot = pool.Create(0.0f, 0.0f, 0.0f, 0.0f, 10.0f);
      pool.At(slot).vy += 2.0f;        // the caller's gravity for one second
      pool.Animate(0.5f);
      Assert::That(pool.At(slot).y, Equals(1.0f));
    };
  };
};