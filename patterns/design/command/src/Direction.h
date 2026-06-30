#pragma once

// The action a command encapsulates. Kept SDL-free so commands stay pure and
// easily testable.
enum Direction { LEFT, RIGHT, UP, DOWN };
