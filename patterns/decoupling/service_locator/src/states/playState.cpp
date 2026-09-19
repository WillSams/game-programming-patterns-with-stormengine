#include "playState.h"

#include <algorithm>

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    services::ServiceLocator::Provide(&consoleAudio_);
}

PlayState::~PlayState() { onExit(); }

bool PlayState::onEnter() {
    m_loadingComplete = true;
    return true;
}

// ⚠️ AND IT PUTS THE LOCATOR BACK. The locator is a global, so a screen that
// registers a provider and exits leaving it registered is the state-leak the specs
// pin. A real game would register a platform provider at startup and leave it; a
// demo that swaps providers as its subject must not leak one into the next thing
// that runs.
bool PlayState::onExit() {
    services::ServiceLocator::Provide(nullptr);
    assetStore_->ClearAssets();
    m_exiting = true;
    return true;
}

// ── The game's calls. Not one of them names a provider ───────────────────────
void PlayState::CallSound(const std::string &tag) {
    called_.push_back("CALL SOUND " + tag);
    services::ServiceLocator::Audio().PlaySound(tag);
}
void PlayState::CallMusic(const std::string &tag) {
    called_.push_back("CALL MUSIC " + tag);
    services::ServiceLocator::Audio().PlayMusic(tag);
}
void PlayState::CallStopAll() {
    called_.push_back("CALL STOP ALL");
    services::ServiceLocator::Audio().StopAll();
}

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type != SDL_KEYDOWN)
            continue;

        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            isRunning_ = false;
            return;
        case SDLK_p: CallSound("goal"); break;
        case SDLK_m: CallMusic("arena"); break;
        case SDLK_x: CallStopAll(); break;
        case SDLK_1:
            services::ServiceLocator::Provide(&consoleAudio_);
            status_ = "CONSOLE AUDIO REGISTERED";
            break;
        case SDLK_2:
            services::ServiceLocator::Provide(nullptr);   // the null service
            status_ = "NULL AUDIO REGISTERED -- THE GAME KEEPS CALLING";
            break;
        case SDLK_r:
            called_.clear();
            consoleAudio_.Clear();
            status_ = "CLEARED";
            break;
        default: break;
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();
}

namespace {
const SDL_Color kInk    = {230, 230, 235, 255};
const SDL_Color kDim    = {140, 150, 165, 255};
const SDL_Color kRow    = {170, 180, 195, 255};
const SDL_Color kCursor = {255, 200,  80, 255};
} // namespace

void PlayState::DrawCalled(int x, int y) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "CALLED BY THE GAME", x, y, 4);
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "IT NAMES NO PROVIDER", x, y + 24, 3);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    if (called_.empty()) {
        DrawPixelText(renderer_, "NOTHING YET", x, y + 48, 3);
        return;
    }
    const std::size_t shown = std::min<std::size_t>(called_.size(), 7);
    for (std::size_t i = 0; i < shown; ++i)
        DrawPixelText(renderer_, called_[called_.size() - shown + i], x,
                      y + 48 + static_cast<int>(i) * 18, 3);
}

void PlayState::DrawRegistered(int x, int y) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "REGISTERED", x, y, 4);

    const bool haveProvider = services::ServiceLocator::HasProvider();
    SDL_SetRenderDrawColor(renderer_, haveProvider ? kCursor.r : kDim.r,
                           haveProvider ? kCursor.g : kDim.g,
                           haveProvider ? kCursor.b : kDim.b, 255);
    // ⚠️ NO ARROW MARKER. An earlier version appended ` <` to the active row, which
    // reads as "less than" and is not the convention the sibling decoupling demos
    // use -- they mark the active row by COLOUR and say the state outright, which is
    // what the `CURRENT` line below does.
    DrawPixelText(renderer_, "1 CONSOLE AUDIO", x, y + 28, 3);
    SDL_SetRenderDrawColor(renderer_, !haveProvider ? kCursor.r : kDim.r,
                           !haveProvider ? kCursor.g : kDim.g,
                           !haveProvider ? kCursor.b : kDim.b, 255);
    DrawPixelText(renderer_, "2 NULL AUDIO", x, y + 48, 3);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, std::string("CURRENT ") + services::ServiceLocator::CurrentName(),
                  x, y + 72, 3);
}

void PlayState::DrawHeard(int x, int y) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "HEARD BY THE SERVICE", x, y, 4);
    const std::vector<std::string> &log = consoleAudio_.Log();
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    if (log.empty()) {
        // The null service, doing its job: the game calls and nothing arrives.
        DrawPixelText(renderer_, "NOTHING YET", x, y + 24, 3);
        return;
    }
    const std::size_t shown = std::min<std::size_t>(log.size(), 7);
    for (std::size_t i = 0; i < shown; ++i)
        DrawPixelText(renderer_, log[log.size() - shown + i], x,
                      y + 24 + static_cast<int>(i) * 18, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "P SOUND  M MUSIC  X STOP  1-2 PROVIDER  R CLEAR  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    const int colR = windowWidth_ / 2 + pad;

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "SERVICE LOCATOR", pad, pad, 4);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "THE GAME ASKS, SOMETHING ELSE ANSWERS", pad, pad + 28, 4);

    DrawCalled(pad, 96);
    DrawRegistered(colR, 96);
    DrawHeard(colR, 216);

    // The pattern's payoff in one line: two counters that can disagree.
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "CALLS " + std::to_string(called_.size()) +
                              "   HEARD " + std::to_string(consoleAudio_.Log().size()),
                  pad, 352, 4);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
