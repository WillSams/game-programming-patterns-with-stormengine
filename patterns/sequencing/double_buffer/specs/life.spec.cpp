#include <igloo/igloo_alt.h>

#include "../src/life/life.h"
#include "../src/util/doubleBuffer.h"

using namespace igloo;

Describe(GameOfLifeSpec) {

    Describe(NeighborCounting) {
        It(should_count_all_eight_surrounding_cells) {
            Grid g(3, 3);
            g.set(0, 0, true); g.set(1, 0, true); g.set(2, 0, true);
            g.set(0, 1, true);                     g.set(2, 1, true);
            g.set(0, 2, true); g.set(1, 2, true); g.set(2, 2, true);
            Assert::That(LiveNeighbors(g, 1, 1), Equals(8));
        };
        It(should_treat_out_of_bounds_as_dead) {
            Grid g(3, 3);
            g.set(0, 0, true); // a corner cell has no in-bounds live neighbors
            Assert::That(LiveNeighbors(g, 0, 0), Equals(0));
        };
    };

    Describe(ConwayRules) {
        It(should_kill_a_lonely_cell)                { Assert::That(NextCellState(true, 1),  Equals(false)); };
        It(should_keep_a_cell_with_two_neighbors)    { Assert::That(NextCellState(true, 2),  Equals(true));  };
        It(should_keep_a_cell_with_three_neighbors)  { Assert::That(NextCellState(true, 3),  Equals(true));  };
        It(should_kill_an_overcrowded_cell)          { Assert::That(NextCellState(true, 4),  Equals(false)); };
        It(should_birth_a_dead_cell_with_three_neighbors) { Assert::That(NextCellState(false, 3), Equals(true));  };
        It(should_leave_a_dead_cell_dead_otherwise)  { Assert::That(NextCellState(false, 2), Equals(false)); };
    };

    Describe(StillLife) {
        It(should_keep_a_block_stable) {
            Grid g(4, 4);
            g.set(1, 1, true); g.set(2, 1, true);
            g.set(1, 2, true); g.set(2, 2, true);

            Grid next;
            Step(g, next);
            Assert::That(next.alive(1, 1) && next.alive(2, 1) &&
                         next.alive(1, 2) && next.alive(2, 2), Equals(true));
            Assert::That(next.liveCount(), Equals(4));
        };
    };

    Describe(Oscillator) {
        // A blinker demonstrates why the second buffer matters: the whole row is
        // read as generation N, then a perpendicular row is written as N+1.
        It(should_flip_a_blinker_each_generation_through_the_double_buffer) {
            DoubleBuffer<Grid> board(Grid(5, 5));
            board.current().set(2, 1, true); // vertical bar
            board.current().set(2, 2, true);
            board.current().set(2, 3, true);

            Step(board.current(), board.next());
            board.swap();
            Assert::That(board.current().alive(1, 2) &&
                         board.current().alive(2, 2) &&
                         board.current().alive(3, 2), Equals(true)); // now horizontal
            Assert::That(board.current().liveCount(), Equals(3));

            Step(board.current(), board.next());
            board.swap();
            Assert::That(board.current().alive(2, 1) &&
                         board.current().alive(2, 2) &&
                         board.current().alive(2, 3), Equals(true)); // vertical again
        };
    };
};
