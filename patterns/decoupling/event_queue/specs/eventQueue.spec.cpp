#include <igloo/igloo_alt.h>

#include <string>
#include <vector>

#include "../src/events/eventQueue.h"

using namespace igloo;
using namespace events;

// The pattern's core, spec'd with no window. Three things are being pinned: that
// the queue preserves order, what it does when it is FULL, and what happens when a
// handler SENDS WHILE DISPATCHING -- which is the chapter's feedback loop and the
// only part of this that can go wrong invisibly.
Describe(EventQueueSpec) {

  static Event Scored(int pts, int id) { return {EventType::Scored, pts, id, {}}; }
  static Event Sound(const std::string &tag) {
      return {EventType::Sound, 0, 0, tag};
  }

  Describe(OrderIsPreserved) {
    It(delivers_events_in_the_order_they_were_sent) {
      EventQueue q;
      q.send(Scored(1, 10));
      q.send(Scored(2, 11));
      q.send(Sound("whistle"));

      std::vector<std::string> seen;
      const std::size_t n = q.dispatch([&](const Event &e) {
          seen.push_back(DescribeEvent(e));
      });

      Assert::That(n, Equals(static_cast<std::size_t>(3)));
      Assert::That(seen.size(), Equals(static_cast<std::size_t>(3)));
      Assert::That(seen[0], Equals("SCORED 1 BY 10"));
      Assert::That(seen[1], Equals("SCORED 2 BY 11"));
      Assert::That(seen[2], Equals("SOUND whistle"));
    };

    It(leaves_the_queue_empty_after_a_drain) {
      EventQueue q;
      q.send(Scored(1, 10));
      q.dispatch([](const Event &) {});
      Assert::That(q.empty(), IsTrue());
      Assert::That(q.size(), Equals(static_cast<std::size_t>(0)));
    };

    It(does_nothing_at_all_when_there_is_nothing_queued) {
      EventQueue q;
      bool called = false;
      Assert::That(q.dispatch([&](const Event &) { called = true; }),
                   Equals(static_cast<std::size_t>(0)));
      Assert::That(called, IsFalse());
    };
  };

  // ⚠️ DECISION 1: FIXED CAPACITY, AND A FULL QUEUE REFUSES. The chapter leaves
  // this open (drop, grow, or wrap); a bounded queue that says NO is what something
  // running inside a frame should do, and the point of pinning it is that the
  // refusal leaves the queue EXACTLY as it was -- no half-written event, no lost
  // oldest one.
  Describe(AFullQueueRefuses) {
    It(sends_up_to_capacity_then_says_no) {
      EventQueue q;
      for (std::size_t i = 0; i < EventQueue::kCapacity; ++i)
        Assert::That(q.send(Scored(static_cast<int>(i), 1)), IsTrue());
      Assert::That(q.full(), IsTrue());
      Assert::That(q.send(Scored(99, 1)), IsFalse());     // refused
      Assert::That(q.size(), Equals(EventQueue::kCapacity));
    };

    It(leaves_the_queued_events_untouched_when_it_refuses) {
      EventQueue q;
      for (std::size_t i = 0; i < EventQueue::kCapacity; ++i)
        q.send(Scored(static_cast<int>(i), 1));
      q.send(Scored(99, 1));                              // refused

      std::vector<int> seen;
      q.dispatch([&](const Event &e) { seen.push_back(e.a); });
      Assert::That(seen.size(), Equals(EventQueue::kCapacity));
      for (std::size_t i = 0; i < seen.size(); ++i)
        Assert::That(seen[i], Equals(static_cast<int>(i)));  // 0..7, no 99
    };

    // The ring WRAPS: after draining, the free space is at the front, and the
    // second fill must come back in order rather than walking off the end.
    It(wraps_and_still_delivers_in_order) {
      EventQueue q;
      for (std::size_t i = 0; i < EventQueue::kCapacity; ++i)
        q.send(Scored(static_cast<int>(i), 1));
      q.dispatch([](const Event &) {});

      for (std::size_t i = 100; i < 100 + EventQueue::kCapacity; ++i)
        q.send(Scored(static_cast<int>(i), 1));

      std::vector<int> seen;
      q.dispatch([&](const Event &e) { seen.push_back(e.a); });
      Assert::That(seen.size(), Equals(EventQueue::kCapacity));
      for (std::size_t i = 0; i < seen.size(); ++i)
        Assert::That(seen[i], Equals(100 + static_cast<int>(i)));
    };
  };

  // ⚠️ DECISION 2, AND THE CHAPTER'S OWN WARNING: an event handler that sends
  // another event. If dispatch RECURSED, the new event would be handled before the
  // ones already queued -- which is not the order of sending and is how a feedback
  // loop unwinds a stack. It is queued instead, so the order stays the order.
  Describe(SendingDuringDispatchIsQueuedNotRecursed) {
    It(handles_the_new_event_after_the_ones_already_queued) {
      EventQueue q;
      q.send(Scored(1, 1));          // A
      q.send(Scored(2, 1));          // B

      std::vector<std::string> seen;
      q.dispatch([&](const Event &e) {
          seen.push_back(DescribeEvent(e));
          if (e.a == 1)
              q.send(Sound("cheer"));   // C, sent while A is being handled
      });

      // A, B, C -- NOT A, C, B. Recursion would give the second.
      Assert::That(seen.size(), Equals(static_cast<std::size_t>(3)));
      Assert::That(seen[0], Equals("SCORED 1 BY 1"));
      Assert::That(seen[1], Equals("SCORED 2 BY 1"));
      Assert::That(seen[2], Equals("SOUND cheer"));
    };

    It(drains_a_chain_of_events_caused_by_each_other) {
      EventQueue q;
      q.send(Scored(1, 1));
      int handled = 0;
      q.dispatch([&](const Event &e) {
          ++handled;
          if (e.type == EventType::Scored)
              q.send(Sound("goal-horn"));
          else if (e.type == EventType::Sound)
              q.send({EventType::Achievement, 0, 0, "first-horn"});
      });
      Assert::That(handled, Equals(3));   // scored -> sound -> achievement
      Assert::That(q.empty(), IsTrue());
    };
  };

  // ⚠️ DECISION 3, AND THE REASON IT EXISTS: a handler that sends on EVERY event is
  // a genuine feedback loop -- scored -> sound -> scored -> ... -- and an unbounded
  // drain would never return. It stops at kDrainLimit and leaves the rest queued.
  Describe(AnEndlessHandlerStillTerminates) {
    It(stops_at_the_drain_limit_and_leaves_the_queue_non_empty) {
      EventQueue q;
      q.send(Scored(1, 1));
      const std::size_t n = q.dispatch([&](const Event &) {
          q.send(Scored(1, 1));      // forever
      });
      Assert::That(n, Equals(EventQueue::kDrainLimit));
      Assert::That(q.empty(), IsFalse());   // the loop did not run away
      Assert::That(q.size() <= EventQueue::kCapacity, IsTrue());
    };
  };

  // A table with a hole in it fails silently, exactly as the font's does.
  Describe(EventCoverage) {
    It(names_every_event_type) {
      const EventType all[] = {EventType::Scored, EventType::TookDamage,
                              EventType::Jumped, EventType::Sound,
                              EventType::Achievement};
      for (EventType t : all) {
        const std::string n = EventName(t);
        Assert::That(n.empty(), IsFalse());
        Assert::That(n == "?", IsFalse());
      }
    };
  };
};
