#pragma once

// ── Double Buffer pattern ────────────────────────────────────────────────────
// (Game Programming Patterns, Robert Nystrom — Sequencing Patterns)
//
// Encapsulates a pair of buffers. Readers only ever see the stable `current()`
// buffer while a writer builds the `next()` one; a single `swap()` publishes the
// freshly written buffer atomically. Nobody ever observes a half-finished state.
//
// The textbook use is a graphics framebuffer (draw into the back buffer, flip),
// but the pattern applies to *any* state whose update reads its own previous
// values — a cellular automaton like Conway's Game of Life is the demo here:
// every cell must read the current generation, so the next generation has to be
// written somewhere else. That "somewhere else" is the second buffer.
template <typename T>
class DoubleBuffer {
public:
    DoubleBuffer() = default;

    // Seed both buffers so `current()` and `next()` start from the same state.
    explicit DoubleBuffer(const T &seed) {
        buffers_[0] = seed;
        buffers_[1] = seed;
    }

    // The stable buffer everything reads this frame.
    const T &current() const { return buffers_[front_]; }
    T       &current()       { return buffers_[front_]; }

    // The hidden buffer the next frame is written into.
    T &next() { return buffers_[front_ ^ 1]; }

    // Publish the written buffer, hiding the old one.
    void swap() { front_ ^= 1; }

private:
    T   buffers_[2]{};
    int front_ = 0;
};
