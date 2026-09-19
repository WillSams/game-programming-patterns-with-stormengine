// This folder has no pattern example -- it holds the specs for headers SHARED by
// every pattern (`include/pixelFont.h`). The build needs a main.cpp for its
// example target, and that target is never run; the specs are what matter:
//
//     make test && make run-test
int main() { return 0; }
