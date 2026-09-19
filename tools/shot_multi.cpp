// Software-render a pattern demo's render() to a BMP, so layout can be looked at
// instead of reasoned about.
//
//   ./shot <key-sequence> <out.bmp> <title-bar-hidden?>
//
// One character = one keypress. `_` is SPACE, `.` is one update(), `|` is a frame
// that is rendered and thrown away. Every other printable character is its own
// keycode (letters and digits).
#include <SDL2/SDL.h>

#include <cctype>
#include <memory>
#include <string>

#include <stormengine2/assetStore.h>

#include "src/states/playState.h"

static SDL_Keycode KeyFor(char c) {
    if (c >= 'a' && c <= 'z') return SDLK_a + (c - 'a');
    if (c >= 'A' && c <= 'Z') return SDLK_a + (c - 'A');
    if (c >= '0' && c <= '9') return SDLK_0 + (c - '0');
    if (c == ' ') return SDLK_SPACE;
    return SDLK_UNKNOWN;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        SDL_Log("usage: %s <key-sequence> <out.bmp>", argv[0]);
        return 1;
    }
    const std::string sequence = argv[1];
    const std::string output   = argv[2];

    SDL_Init(SDL_INIT_VIDEO);

    const int width = 800, height = 600;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
        0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);

    bool running = true;
    auto assetStore = std::make_unique<AssetStore>();
    PlayState state(renderer, width, height, false, std::move(assetStore), running);
    state.onEnter();

    auto press = [&](SDL_Keycode key) {
        SDL_Event event{};
        event.type = SDL_KEYDOWN;
        event.key.keysym.sym = key;
        SDL_PushEvent(&event);
        state.processInput();
    };

    for (char c : sequence) {
        if (c == '_') { press(SDLK_SPACE); continue; }
        if (c == '.') { state.update(); continue; }
        if (c == '|') { state.render(); continue; }
        const SDL_Keycode key = KeyFor(c);
        if (key != SDLK_UNKNOWN)
            press(key);
    }

    state.render();
    SDL_SaveBMP(surface, output.c_str());

    state.onExit();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_Quit();
    return 0;
}