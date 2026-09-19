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
}

PlayState::~PlayState() { onExit(); }

bool PlayState::onEnter() {
    m_loadingComplete = true;
    return true;
}

bool PlayState::onExit() {
    assetStore_->ClearAssets();
    m_exiting = true;
    return true;
}

// The sender's whole part in this: put an event in and know nothing else. It does
// not name a listener, count them, or check whether any exists.
void PlayState::Send(const events::Event &event) {
    if (!queue_.send(event)) {          // decision 1: a full queue says so
        status_ = "QUEUE FULL -- EVENT DROPPED";
        return;
    }
    sent_.push_back(events::DescribeEvent(event));

    // Whoever cares, takes it out. The handlers below are the "systems"; they are
    // wired here and nowhere near the sender above.
    lastDrained_ = queue_.dispatch([&](const events::Event &e) {
        reacted_.push_back("SAW " + events::DescribeEvent(e));
        // ⚠️ A listener REACTING by sending another event -- the chapter's own
        // example, and the queue delivers it in order rather than recursing.
        if (e.type == events::EventType::Scored && soundListener_)
            queue_.send({events::EventType::Sound, 0, 0, "cheer"});
        if (e.type == events::EventType::Sound && achievementListener_)
            queue_.send({events::EventType::Achievement, 0, 0, "first-goal"});
    });
    status_ = "DRAINED " + std::to_string(lastDrained_);
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
        case SDLK_s: Send({events::EventType::Scored, 3, 7, {}}); break;
        case SDLK_d: Send({events::EventType::TookDamage, 5, 7, {}}); break;
        case SDLK_j: Send({events::EventType::Jumped, 10, 0, {}}); break;
        case SDLK_1:
            soundListener_ = !soundListener_;
            status_ = soundListener_ ? "SOUND LISTENER ON" : "SOUND LISTENER OFF";
            break;
        case SDLK_2:
            achievementListener_ = !achievementListener_;
            status_ = achievementListener_ ? "ACH LISTENER ON" : "ACH LISTENER OFF";
            break;
        case SDLK_r:
            sent_.clear();
            reacted_.clear();
            lastDrained_ = 0;
            status_ = "READY";
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

// ── The screen ───────────────────────────────────────────────────────────────
//
// Same visual language as the other demos: background, ink, 16px padding, 4x
// font, hint anchored to the window height.
namespace {
const SDL_Color kInk    = {230, 230, 235, 255};
const SDL_Color kDim    = {140, 150, 165, 255};
const SDL_Color kRow    = {170, 180, 195, 255};
const SDL_Color kCursor = {255, 200,  80, 255};
} // namespace

// The sender's view: events, and nothing about who saw them.
void PlayState::DrawSent(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "SENT", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "BY THE SENDER, WHO KNOWS NO ONE", x, y + 24, 3);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    if (sent_.empty()) {
        DrawPixelText(renderer_, "NOTHING YET", x, y + 48, 3);
        return;
    }
    const std::size_t shown = std::min<std::size_t>(sent_.size(), 6);
    for (std::size_t i = 0; i < shown; ++i)
        DrawPixelText(renderer_, sent_[sent_.size() - shown + i], x,
                      y + 48 + static_cast<int>(i) * 18, 3);
}

// Listeners are switchable, and the sender above is not told. Two columns produced
// by different code is the pattern; this is the second column's switchboard.
void PlayState::DrawListeners(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "LISTENERS", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, soundListener_ ? kCursor.r : kDim.r,
                           soundListener_ ? kCursor.g : kDim.g,
                           soundListener_ ? kCursor.b : kDim.b, 255);
    DrawPixelText(renderer_, std::string("1 SOUND ") + (soundListener_ ? "ON" : "OFF"),
                  x, y + 28, 3);
    SDL_SetRenderDrawColor(renderer_, achievementListener_ ? kCursor.r : kDim.r,
                           achievementListener_ ? kCursor.g : kDim.g,
                           achievementListener_ ? kCursor.b : kDim.b, 255);
    DrawPixelText(renderer_,
                  std::string("2 ACHIEVEMENT ") + (achievementListener_ ? "ON" : "OFF"),
                  x, y + 48, 3);
}

void PlayState::DrawReacted(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "REACTED", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    if (reacted_.empty()) {
        DrawPixelText(renderer_, "NOTHING YET", x, y + 24, 3);
        return;
    }
    const std::size_t shown = std::min<std::size_t>(reacted_.size(), 8);
    for (std::size_t i = 0; i < shown; ++i)
        DrawPixelText(renderer_, reacted_[reacted_.size() - shown + i], x,
                      y + 24 + static_cast<int>(i) * 18, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "S SCORE  D DAMAGE  J JUMP  1-2 LISTENERS  R RESET  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    const int scale = 4;
    const int colR = windowWidth_ / 2 + pad;

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "EVENT QUEUE", pad, pad, scale);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "ONE SEND, NO RECEIVERS NAMED", pad, pad + 28, scale);

    DrawSent(pad, 96, scale);

    DrawListeners(colR, 96, scale);
    DrawReacted(colR, 200, scale);

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "STATUS " + status_, pad, 348, scale);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
