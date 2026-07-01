#include "src/game.h"

// Entry point. The engine owns the main loop; the Singleton pattern demo lives
// in PlayState under src/states/.
int main(int argc, char *argv[]) {
    Game game;
    game.Run();
    game.Destroy();
    return 0;
}
