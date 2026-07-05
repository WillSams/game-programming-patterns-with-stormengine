#pragma once

#include "grid.h"

// Live cells in the 8-neighborhood of (x,y). Out-of-bounds neighbors are dead.
inline int LiveNeighbors(const Grid &g, int x, int y) {
    int n = 0;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            if (g.alive(x + dx, y + dy)) ++n;
        }
    return n;
}

// Conway's rules for a single cell:
//   live + 2 or 3 neighbors  -> survives
//   dead + exactly 3         -> born
//   otherwise                -> dead
inline bool NextCellState(bool alive, int neighbors) {
    if (alive) return neighbors == 2 || neighbors == 3;
    return neighbors == 3;
}

// Advance one generation, reading `in` and writing `out`. `out` is resized to
// match `in` if needed. This is the crux of the Double Buffer pattern: because
// every cell reads the *current* generation, the results must be written to a
// separate buffer — updating `in` in place would corrupt neighbor reads that
// haven't happened yet.
inline void Step(const Grid &in, Grid &out) {
    if (out.width != in.width || out.height != in.height)
        out = Grid(in.width, in.height);

    for (int y = 0; y < in.height; ++y)
        for (int x = 0; x < in.width; ++x)
            out.set(x, y, NextCellState(in.alive(x, y), LiveNeighbors(in, x, y)));
}
