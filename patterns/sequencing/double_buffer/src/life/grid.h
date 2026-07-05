#pragma once

#include <cstdint>
#include <vector>

// A fixed-size grid of live/dead cells for Conway's Game of Life. Row-major,
// 0 = dead, 1 = live. Out-of-bounds coordinates read as dead and ignore writes,
// so the edges behave as permanently dead borders (a bounded, non-toroidal world).
struct Grid {
    int                  width  = 0;
    int                  height = 0;
    std::vector<uint8_t> cells;

    Grid() = default;
    Grid(int w, int h)
        : width(w), height(h), cells(static_cast<size_t>(w) * h, 0) {}

    bool alive(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) return false;
        return cells[index(x, y)] != 0;
    }

    void set(int x, int y, bool live) {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        cells[index(x, y)] = live ? 1 : 0;
    }

    int liveCount() const {
        int n = 0;
        for (uint8_t c : cells) n += c;
        return n;
    }

private:
    size_t index(int x, int y) const {
        return static_cast<size_t>(y) * width + x;
    }
};
