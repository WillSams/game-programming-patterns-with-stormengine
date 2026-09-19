#include "playState.h"

#include <cmath>

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

namespace {
constexpr float kWorldSpan = 80.0f;    // -40..40 maps onto the field box
constexpr float kPi = 3.14159265358979323846f;
} // namespace

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    Build();
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

// One root, four branches, three grandchildren each: 17 nodes. The local offsets make
// a real fan, so a parent's move visibly carries its descendants with it.
void PlayState::Build() {
    tree_ = dirty::TransformTree{};
    root_ = tree_.AddRoot(0.0f, 0.0f);
    for (int c = 0; c < 4; ++c) {
        const float angle = kPi * 0.5f * static_cast<float>(c);
        const int child = tree_.AddChild(root_, 20.0f * std::cos(angle),
                                        20.0f * std::sin(angle));
        if (c == 0)
            branch_ = child;              // recorded, not re-derived at the key
        for (int g = 0; g < 3; ++g) {
            const float ga = angle - 0.45f + 0.45f * static_cast<float>(g);
            tree_.AddChild(child, 12.0f * std::cos(ga), 12.0f * std::sin(ga));
        }
    }
    tree_.ResetCounters();
    phase_ = 0.0f;
    lastDirty_ = tree_.DirtyCount();
    status_ = autoMove_ ? "AUTO MOVE ON" : "AUTO MOVE OFF";
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();

    if (autoMove_) {
        phase_ += 0.05f;
        // ⚠️ THE WHOLE COST OF A CHANGE. One call, no arithmetic, seventeen nodes
        // marked stale -- and none of them rebuilt until something reads them.
        //
        // ⚠️ AND THE AMPLITUDE IS BOUNDED BY THE WORLD SPAN, which is not decorative:
        // world = root + child(20) + grandchild(12), so a root travel of 14 put the
        // furthest node at 46 units in an 80-unit span and DREW IT OVER THE COLUMN
        // BESIDE THE FIELD. Six keeps the worst case at 38 with the offsets this demo
        // builds. An offset set anywhere in this file has to respect that sum.
        tree_.SetLocal(root_, 6.0f * std::cos(phase_), 6.0f * std::sin(phase_));
    }
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
        case SDLK_SPACE:                       // move the root: dirties all 17
            tree_.SetLocal(root_, 0.0f, 0.0f);
            status_ = "ROOT MOVED -- 17 NODES MARKED, NONE REBUILT";
            break;
        case SDLK_c:                           // move ONE branch: dirties 4 of 17
            // ⚠️ THE BRANCH IS RECORDED BY `Build`, NOT RE-DERIVED HERE. An earlier
            // version computed it as `AddChild(-1, ...) < 0 ? 1 : 1` -- a nonsense
            // expression that returned -1 only to become the literal 1, and that also
            // attempted to add a node on every keypress. It read as arithmetic and
            // was a constant.
            // ⚠️ THE OFFSET MUST KEEP THE NODE INSIDE THE WORLD SPAN, and 30 did
            // not: world = root + child + grandchild, so root's own +/-14 travel plus
            // a local 30 put the branch past the field's edge and drew its nodes OVER
            // THE RIGHT COLUMN. The span is 80 units (+/-40) and this stays well
            // inside it at every root position. A node outside the box is a drawing
            // bug, not a simulation result.
            tree_.SetLocal(branch_, 12.0f, 4.0f);
            status_ = "ONE BRANCH MOVED -- 4 OF 17 MARKED";
            break;
        case SDLK_a:
            autoMove_ = !autoMove_;
            status_ = autoMove_ ? "AUTO MOVE ON" : "AUTO MOVE OFF -- WATCH RECOMPUTES";
            break;
        case SDLK_r:
            tree_.ResetCounters();
            status_ = "COUNTERS CLEARED -- THE SCENE IS UNCHANGED";
            break;
        case SDLK_0:
            Build();
            status_ = "SCENE REBUILT";
            break;
        default: break;
        }
    }
}

// ── The field ────────────────────────────────────────────────────────────────
//
// ⚠️ DRAWING IS THE READ THAT RESOLVES. A dirty node is brought up to date HERE,
// because this is the first thing that asks for its world position -- which is the
// deferral working, and also why the count of stale nodes has to be captured BEFORE
// this loop runs: afterwards there are none.
int PlayState::FieldX(float worldX, int x, int w) const {
    return x + 1 + static_cast<int>((worldX + kWorldSpan * 0.5f) / kWorldSpan * (w - 2));
}
int PlayState::FieldY(float worldY, int y, int h) const {
    return y + 1 + static_cast<int>((worldY + kWorldSpan * 0.5f) / kWorldSpan * (h - 2));
}

void PlayState::DrawTree(int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer_, 60, 70, 90, 255);
    SDL_Rect box{x, y, w, h};
    SDL_RenderDrawRect(renderer_, &box);

    dirty::TransformTree &tree = tree_;   // no cast: this method is not const

    // Lines first, so the boxes sit on top of their connections.
    for (std::size_t i = 0; i < tree.size(); ++i) {
        const int parent = tree.Parent(static_cast<int>(i));
        if (parent < 0)
            continue;
        SDL_SetRenderDrawColor(renderer_, 45, 52, 68, 255);
        SDL_RenderDrawLine(renderer_, FieldX(tree.WorldX(parent), x, w),
                                      FieldY(tree.WorldY(parent), y, h),
                                      FieldX(tree.WorldX(static_cast<int>(i)), x, w),
                                      FieldY(tree.WorldY(static_cast<int>(i)), y, h));
    }

    for (std::size_t i = 0; i < tree.size(); ++i) {
        const int idx = static_cast<int>(i);
        const int px = FieldX(tree.WorldX(idx), x, w);
        const int py = FieldY(tree.WorldY(idx), y, h);
        const bool isRoot = (idx == root_);
        SDL_SetRenderDrawColor(renderer_, isRoot ? 255 : 120, isRoot ? 200 : 210,
                               isRoot ? 80 : 255, 255);
        SDL_Rect node{px - (isRoot ? 4 : 2), py - (isRoot ? 4 : 2),
                      isRoot ? 9 : 5, isRoot ? 9 : 5};
        SDL_RenderFillRect(renderer_, &node);
    }
}

void PlayState::DrawNumbers(int x, int y, std::size_t dirtyBeforeDraw) const {
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "WORK", x, y, 4);

    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "NODES " + std::to_string(tree_.size()), x, y + 28, 3);
    DrawPixelText(renderer_, "READS " + std::to_string(tree_.Reads()), x, y + 46, 3);

    // ⚠️ "READS", NOT "DRAWS". The counter counts READS -- the line loop reads both
    // ends of every edge, so 17 nodes give 98 reads, and calling that "draws" invited
    // the reader to wonder why 98 frames drew 17 nodes. It now names what the header
    // counts and what a spec asserts on.

    // The two that matter. The gap between them is the work the flag deferred.
    SDL_SetRenderDrawColor(renderer_, 120, 210, 255, 255);
    DrawPixelText(renderer_, "REBUILT " + std::to_string(tree_.Recomputes()), x, y + 70, 3);

    SDL_SetRenderDrawColor(renderer_, 255, 200, 80, 255);
    DrawPixelText(renderer_, "MARKS " + std::to_string(tree_.Sets()), x, y + 88, 3);

    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "STALE BEFORE DRAW " + std::to_string(dirtyBeforeDraw), x, y + 112, 3);

    // Stated as a share, because "was any work avoided" is the question the reader
    // actually has. Integer math on purpose: a percent with a decimal invites
    // reading precision into a counter ratio.
    const std::size_t draws = tree_.Reads();
    const int pct = draws == 0 ? 100
                               : static_cast<int>(100 - (tree_.Recomputes() * 100) / draws);
    SDL_SetRenderDrawColor(renderer_, pct > 50 ? 120 : 255, pct > 50 ? 210 : 170,
                           pct > 50 ? 255 : 100, 255);
    DrawPixelText(renderer_, "AVOIDED " + std::to_string(pct) + " PCT", x, y + 136, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, 140, 150, 165, 255);
    DrawPixelText(renderer_, "SPACE ROOT  C BRANCH  A AUTO  R CLEAR  0 REBUILD  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "DIRTY FLAG", pad, pad, 4);

    SDL_SetRenderDrawColor(renderer_, 140, 150, 165, 255);
    DrawPixelText(renderer_, "MARK IT STALE, REBUILD ON DEMAND", pad, pad + 28, 4);

    // ⚠️ CAPTURED BEFORE THE DRAW LOOP, because drawing is what resolves the stale
    // nodes. Read after, and this number would always be zero.
    const std::size_t stale = tree_.DirtyCount();

    DrawTree(pad, 96, 380, 240);
    DrawNumbers(416, 96, stale);

    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "STATUS " + status_, pad, 360, 3);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
