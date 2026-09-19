#pragma once

#include <cstddef>
#include <functional>
#include <string>

// ── Event Queue pattern: the sender does not know who listens ────────────────
//
// The chapter's problem: a system that wants to announce something has to know
// every system that cares. The combat code must call the audio code, the
// achievement code, the UI code -- and the more it announces, the more it knows,
// until nothing can be added or removed without touching it.
//
// The pattern breaks that with a QUEUE: the sender puts an event in and knows
// nothing more; whoever cares takes it out. The chapter's own framing is
// "decouple the sender from the receiver", and the queue is just the seam.
//
// ⚠️ PURE. `std::function` is standard C++, not SDL, so the whole pattern is
// spec-able with no window (CODING.md tenet 9).
//
// THREE DECISIONS THIS IMPLEMENTATION MAKES, because the chapter raises all three
// and neither answer is the only one:
//
//   1. FIXED CAPACITY, AND A FULL QUEUE REFUSES. The chapter discusses a ring
//      buffer that wraps and a queue that grows; a bounded queue refuses and says
//      so, which is the honest version for something that must not allocate in the
//      middle of a frame. `send` returns false; the existing events are untouched.
//   2. SENT DURING DISPATCH IS QUEUED, NOT RECURSED. A handler that sends inside
//      `dispatch` appends to the same queue and is handled by the SAME drain, in
//      order. Re-entering the handlers from inside a handler is how the chapter's
//      feedback loop -- and a stack that never unwinds -- happens.
//   3. DISPATCH DRAINS WHAT IS QUEUED, INCLUDING WHAT IT CAUSES, and stops. It does
//      not run forever: a handler that sends on every event is a real feedback
//      loop, and the cap is what stops it (see kDrainLimit).
namespace events {

enum class EventType {
    Scored,       // a = points, b = the scorer's id
    TookDamage,   // a = amount, b = the victim's id
    Jumped,       // a = height
    Sound,        // tag = which sound -- what a listener turns into audio
    Achievement,  // tag = which achievement -- what a listener unlocks
};

struct Event {
    EventType   type = EventType::Scored;
    int         a = 0;
    int         b = 0;
    std::string tag;

    bool operator==(const Event &o) const {
        return type == o.type && a == o.a && b == o.b && tag == o.tag;
    }
};

// A name for each type, for the demo's screen and for readable spec failures --
// one implementation, so the two cannot disagree. A table with a hole in it (an
// event with no name) is the kind of thing that fails silently, so the spec walks
// the enum.
inline const char *EventName(EventType t) {
    switch (t) {
    case EventType::Scored:      return "SCORED";
    case EventType::TookDamage:  return "TOOK_DAMAGE";
    case EventType::Jumped:      return "JUMPED";
    case EventType::Sound:       return "SOUND";
    case EventType::Achievement: return "ACHIEVEMENT";
    }
    return "?";
}

inline std::string DescribeEvent(const Event &e) {
    std::string s = EventName(e.type);
    if (!e.tag.empty())
        return s + " " + e.tag;
    if (e.type == EventType::Scored)
        return s + " " + std::to_string(e.a) + " BY " + std::to_string(e.b);
    if (e.type == EventType::Jumped)
        return s + " " + std::to_string(e.a);
    return s + " " + std::to_string(e.a) + " ID " + std::to_string(e.b);
}

class EventQueue {
public:
    static constexpr std::size_t kCapacity = 8;
    // A drain runs at most this many events, so a handler that sends on every
    // event terminates instead of spinning. Nothing legitimate needs more.
    static constexpr std::size_t kDrainLimit = 64;

    using Handler = std::function<void(const Event &)>;

    // Put an event in. FALSE WHEN FULL -- the event is dropped and the queue is
    // left exactly as it was, which is the documented policy rather than an
    // overflow the caller discovers later.
    bool send(const Event &e) {
        if (count_ == kCapacity)
            return false;
        items_[tail_] = e;
        tail_ = (tail_ + 1) % kCapacity;
        ++count_;
        return true;
    }

    // Take one out. Only valid when non-empty; the demo and the drain are the
    // only callers, and both check.
    Event next() {
        const Event e = items_[head_];
        head_ = (head_ + 1) % kCapacity;
        --count_;
        return e;
    }

    std::size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    bool full() const { return count_ == kCapacity; }

    // Hand every queued event to `handler`, in order, INCLUDING anything the
    // handler itself sends. That is decision 2: sending from inside a handler is
    // how a listener reacts to one event with another, and it is queued rather
    // than recursed so the order stays the order of sending.
    //
    // Returns how many were handled, so a caller (and a spec) can see a drain
    // happen rather than infer it.
    std::size_t dispatch(const Handler &handler) {
        std::size_t handled = 0;
        while (!empty() && handled < kDrainLimit) {
            handler(next());
            ++handled;
        }
        return handled;
    }

private:
    Event       items_[kCapacity];
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t count_ = 0;
};

} // namespace events
