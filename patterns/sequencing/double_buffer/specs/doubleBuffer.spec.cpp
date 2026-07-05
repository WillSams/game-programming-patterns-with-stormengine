#include <igloo/igloo_alt.h>

#include "../src/util/doubleBuffer.h"

using namespace igloo;

Describe(DoubleBufferSpec) {

    It(reads_the_seed_as_the_current_value) {
        DoubleBuffer<int> b(7);
        Assert::That(b.current(), Equals(7));
    };

    It(hides_writes_to_next_until_a_swap) {
        DoubleBuffer<int> b(1);
        b.next() = 99;
        Assert::That(b.current(), Equals(1)); // still stable — reader unaffected
        b.swap();
        Assert::That(b.current(), Equals(99)); // now published
    };

    It(swaps_back_and_forth) {
        DoubleBuffer<int> b(0);
        b.next() = 10; b.swap();
        b.next() = 20; b.swap();
        Assert::That(b.current(), Equals(20));
    };

    It(keeps_the_two_buffers_independent) {
        DoubleBuffer<int> b(5);
        b.next() = 42;
        b.swap();                     // 42 is now current; 5 is now the back buffer
        b.next() = 100;               // scribble on the back buffer
        Assert::That(b.current(), Equals(42)); // visible buffer untouched
    };
};
