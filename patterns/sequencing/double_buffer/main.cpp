#include "src/game.h"

// Entry point. The engine owns the main loop; the pattern demo lives in the
// game states under src/states/.
int main(int argc, char *argv[]) {
    Game game;
    game.Run();
    game.Destroy();
    return 0;
}
