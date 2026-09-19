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
    breeds_ = {&types::kCritter, &types::kBrute, &types::kDragon};
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

void PlayState::Spawn() {
    // A new monster is a new INSTANCE of the shared type -- the breed had to be
    // written once, whether this is the first dragon or the fiftieth.
    spawned_.emplace_back(*breeds_[index_], nextId_++);
    status_ = "SPAWNED " + spawned_.back().name();
}

void PlayState::DamageAll(int amount) {
    for (types::Monster &m : spawned_)
        m.takeDamage(amount);
    status_ = "DAMAGED ALL BY " + std::to_string(amount);
}

void PlayState::HealAll(int amount) {
    for (types::Monster &m : spawned_)
        m.heal(amount);
    status_ = "HEALED ALL BY " + std::to_string(amount);
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
        case SDLK_1: if (breeds_.size() > 0) { index_ = 0; status_ = "BREED " + breeds_[0]->name; } break;
        case SDLK_2: if (breeds_.size() > 1) { index_ = 1; status_ = "BREED " + breeds_[1]->name; } break;
        case SDLK_3: if (breeds_.size() > 2) { index_ = 2; status_ = "BREED " + breeds_[2]->name; } break;
        case SDLK_SPACE: Spawn(); break;
        case SDLK_d: DamageAll(7); break;
        case SDLK_h: HealAll(7); break;
        case SDLK_r:
            spawned_.clear();
            nextId_ = 1;
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
const SDL_Color kBad    = {255, 120, 120, 255};
} // namespace

// Each row shows the VALUES THE BREED ANSWERS, which is where inheritance becomes
// visible: BRUTE says "ATK 12" and shows a health it never stated.
void PlayState::DrawBreeds(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "BREEDS", x, y, scale);
    for (std::size_t i = 0; i < breeds_.size(); ++i) {
        const bool on = (i == index_);
        SDL_SetRenderDrawColor(renderer_, on ? kCursor.r : kRow.r,
                               on ? kCursor.g : kRow.g,
                               on ? kCursor.b : kRow.b, 255);
        const types::Breed &b = *breeds_[i];
        DrawPixelText(renderer_,
                      (on ? "> " : "  ") + std::to_string(i + 1) + " " + b.name +
                          "  HP " + std::to_string(b.healthValue()) +
                          " ATK " + std::to_string(b.attackValue()),
                      x, y + 24 + static_cast<int>(i) * 20, 3);
    }
}

void PlayState::DrawSpawned(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "MONSTERS", x, y, scale);
    if (spawned_.empty()) {
        SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
        DrawPixelText(renderer_, "PRESS SPACE", x, y + 24, 3);
        return;
    }
    // Newest first, and dead ones in a different colour -- the only per-instance
    // thing on this screen.
    for (std::size_t i = 0; i < spawned_.size() && i < 7; ++i) {
        const types::Monster &m = spawned_[spawned_.size() - 1 - i];
        SDL_SetRenderDrawColor(renderer_, m.alive() ? kRow.r : kBad.r,
                               m.alive() ? kRow.g : kBad.g,
                               m.alive() ? kRow.b : kBad.b, 255);
        DrawPixelText(renderer_, m.describe(), x, y + 24 + static_cast<int>(i) * 18, 3);
    }
}

// The pattern's second claim, COMPUTED FROM THE SPAWNED MONSTERS rather than
// stated.
//
// ⚠️ THE FIRST VERSION REPORTED THE WRONG THING: "N MONSTERS / 3 BREEDS" counted
// the BESTIARY, so the two numbers had nothing to do with each other and the
// panel demonstrated no sharing at all -- the one thing it exists to show. The
// count below is the distinct breeds the SPAWNED monsters actually point at, by
// address, which is the same equality `specs/` pins.
void PlayState::DrawSharing(int x, int y, int scale) const {
    std::vector<const types::Breed *> inUse;
    for (const types::Monster &m : spawned_) {
        const types::Breed *b = &m.breed();
        if (std::find(inUse.begin(), inUse.end(), b) == inUse.end())
            inUse.push_back(b);
    }

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "SHARED TYPE", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    DrawPixelText(renderer_,
                  std::to_string(spawned_.size()) + " MONSTERS",
                  x, y + 32, scale);
    // "1 BREEDS" was on screen in the first render of this panel -- a copy bug,
    // and the kind only looking catches.
    DrawPixelText(renderer_,
                  std::to_string(inUse.size()) + (inUse.size() == 1 ? " BREED IN USE"
                                                                    : " BREEDS IN USE"),
                  x, y + 56, scale);

    // The sentence the panel is for, and it is derived: N instances over 1 type
    // object is the pattern, and "ONE EACH" is what the absence of it looks like.
    const std::string verdict =
        spawned_.empty()            ? "NOTHING SPAWNED"
        : (inUse.size() == 1 && spawned_.size() > 1) ? "ALL SHARE ONE"
        : (inUse.size() == spawned_.size())          ? "ONE EACH"
                                                     : "SOME SHARE";
    SDL_SetRenderDrawColor(renderer_, kCursor.r, kCursor.g, kCursor.b, 255);
    DrawPixelText(renderer_, verdict, x, y + 88, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "1-3 BREED  SPACE SPAWN  D DAMAGE  H HEAL  R RESET  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    const int scale = 4;
    const int colR = windowWidth_ / 2 + pad;

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "TYPE OBJECT", pad, pad, scale);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    // ⚠️ `Breed::name` is a FIELD while `Monster::name()` is a METHOD -- the
    // aggregate holds its data, the instance derives a display name from it.
    // The asymmetry cost one compile error here; it is noted rather than
    // papered over, because the next reader will make the same guess.
    DrawPixelText(renderer_, "SELECTED " + breeds_[index_]->name, pad, pad + 28, scale);

    DrawBreeds(pad, 96, scale);
    DrawSpawned(pad, 260, scale);

    DrawSharing(colR, 96, scale);

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "STATUS " + status_, colR, 348, scale);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
