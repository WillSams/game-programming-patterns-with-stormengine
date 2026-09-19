#pragma once

#include <string>
#include <vector>

// ── The sandbox: the world a power may act on, and the ONLY way to act on it ──
//
// Pure C++, no SDL, no engine. Every operation is RECORDED, which is what makes
// the pattern testable at all: a power's effect is a list, and a spec can assert
// the list rather than opening a window and watching particles.
//
// The chapter's point is that these are the *deliberate* primitives -- the base
// class chooses them, and a subclass can say nothing else. Recording them makes
// that vocabulary visible, which is also what the demo screen shows.
namespace powers {

enum class EffectKind { Move, Sound, Particles, AddHealth, SetHealth };

struct Effect {
    EffectKind  kind;
    int         a = 0;    // Move: dx · Particles: count · Health: amount
    int         b = 0;    // Move: dy
    std::string tag;      // Sound: its name

    bool operator==(const Effect &o) const {
        return kind == o.kind && a == o.a && b == o.b && tag == o.tag;
    }
};

class Sandbox {
public:
    // ── the vocabulary ──────────────────────────────────────────────────────
    void move(int dx, int dy) {
        x_ += dx;
        y_ += dy;
        log_.push_back({EffectKind::Move, dx, dy, {}});
    }
    void playSound(const std::string &name) {
        log_.push_back({EffectKind::Sound, 0, 0, name});
    }
    void spawnParticles(int count) {
        log_.push_back({EffectKind::Particles, count, 0, {}});
    }
    void addHealth(int amount) {
        health_ += amount;
        log_.push_back({EffectKind::AddHealth, amount, 0, {}});
    }
    void setHealth(int value) {
        health_ = value;
        log_.push_back({EffectKind::SetHealth, value, 0, {}});
    }

    // ── what a caller may read back ─────────────────────────────────────────
    const std::vector<Effect> &log() const { return log_; }
    int health() const { return health_; }
    int x() const { return x_; }
    int y() const { return y_; }

    // Rewind the whole world. The demo's Reset key; a spec's next case.
    void reset() {
        log_.clear();
        health_ = 100;
        x_ = 0;
        y_ = 0;
    }

private:
    std::vector<Effect> log_;
    int health_ = 100;
    int x_ = 0;
    int y_ = 0;
};

// ── Naming an effect, ONCE ───────────────────────────────────────────────────
//
// The demo's EFFECTS panel and the specs both print these, so they print them the
// same way: `DescribeEffect` is the single implementation of "what does this
// effect read as". A spec that built its own strings would be asserting a second
// spelling of the same fact.
inline const char *EffectName(EffectKind kind) {
    switch (kind) {
    case EffectKind::Move:      return "MOVE";
    case EffectKind::Sound:     return "SOUND";
    case EffectKind::Particles: return "PARTICLES";
    case EffectKind::AddHealth: return "ADD_HP";
    case EffectKind::SetHealth: return "SET_HP";
    }
    return "?";
}

inline std::string DescribeEffect(const Effect &e) {
    switch (e.kind) {
    case EffectKind::Move:
        return std::string("MOVE ") + std::to_string(e.a) + " " + std::to_string(e.b);
    case EffectKind::Sound:
        return std::string("SOUND ") + e.tag;
    case EffectKind::Particles:
        return std::string("PARTICLES ") + std::to_string(e.a);
    case EffectKind::AddHealth:
        return std::string("ADD_HP ") + std::to_string(e.a);
    case EffectKind::SetHealth:
        return std::string("SET_HP ") + std::to_string(e.a);
    }
    return "?";
}

// A whole log on one line, " | " separated. Used by the demo and by the specs.
inline std::string DescribeEffects(const std::vector<Effect> &log) {
    std::string out;
    for (const Effect &e : log) {
        if (!out.empty())
            out += " | ";
        out += DescribeEffect(e);
    }
    return out;
}

} // namespace powers
