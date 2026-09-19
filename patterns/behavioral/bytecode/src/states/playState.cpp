#include "playState.h"

#include <algorithm>

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

// The demo's spells, in the order the number keys index them. DATA, all of it --
// this screen adds no behaviour, only a listing and some keys.
//
// ⚠️ ONE LIST, NOT THREE. The first version had a vector of programs, a parallel
// array of names and a separate kSpellCount -- so adding a spell meant editing
// three places, and forgetting the third made the new one unselectable while the
// second made a name read off the end. A name and its program belong together.
const std::vector<PlayState::Demo> &PlayState::Spells() {
    static const std::vector<Demo> demos = {
        {"MINOR HEAL", bytecode::kMinorHeal},
        {"DRAIN LIFE", bytecode::kDrainLife},
        {"RITUAL", bytecode::kRitual},
        {"FAULTY SPELL", bytecode::kFaultySpell},
    };
    return demos;
}

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    vm_.load(Spells()[0].program);
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

void PlayState::SelectSpell(std::size_t index) {
    if (index >= Spells().size())
        return;
    spellIndex_ = index;
    vm_.load(Spells()[index].program);
    status_ = "READY";
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

        case SDLK_SPACE: {   // one instruction, so the machine can be watched
            switch (vm_.step()) {
            case bytecode::StepResult::Advanced: status_ = "STEPPED"; break;
            case bytecode::StepResult::Halted:   status_ = "HALTED";  break;
            case bytecode::StepResult::Faulted:  status_ = "FAULTED"; break;
            }
            break;
        }

        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            vm_.run();
            status_ = vm_.faulted() ? "FAULTED" : "HALTED";
            break;

        case SDLK_r:
            vm_.rewind();
            status_ = "READY";
            break;

        case SDLK_1: SelectSpell(0); break;
        case SDLK_2: SelectSpell(1); break;
        case SDLK_3: SelectSpell(2); break;
        case SDLK_4: SelectSpell(3); break;
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
// Layout follows the other patterns: the same background, the same text colour,
// the same 16px padding and the same 4x font scale, so the demos read as one set.
// The one addition is that the hint line is anchored to windowHeight_ rather than
// a literal, so it stays at the bottom of whatever window it is given.
namespace {
const SDL_Color kInk    = {230, 230, 235, 255};
const SDL_Color kDim    = {140, 150, 165, 255};
const SDL_Color kRow    = {170, 180, 195, 255};
const SDL_Color kCursor = {255, 200,  80, 255};
const SDL_Color kBad    = {255, 120, 120, 255};
} // namespace

void PlayState::DrawProgram(int x, int y, int scale) const {
    const bytecode::Program &program = vm_.program();
    for (std::size_t i = 0; i < program.size(); ++i) {
        const bytecode::Instruction &in = program[i];
        const bool here = (i == vm_.pc() && !vm_.halted() && !vm_.faulted());

        SDL_SetRenderDrawColor(renderer_, here ? kCursor.r : kRow.r,
                               here ? kCursor.g : kRow.g,
                               here ? kCursor.b : kRow.b, 255);

        // The program counter, then the instruction. An operand is shown only
        // for the opcode that reads one -- a row reading "HALVE_HP 0" would
        // suggest the 0 means something.
        std::string row = std::string(here ? "> " : "  ") + std::to_string(i) + " " +
                          bytecode::OpCodeName(in.op);
        if (in.op == bytecode::OpCode::PushLiteral)
            row += " " + std::to_string(in.operand);
        DrawPixelText(renderer_, row, x, y + static_cast<int>(i) * 20, scale);
    }
}

void PlayState::DrawState(int x, int y, int scale) const {
    const bytecode::VmState &st = vm_.state();
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "HP " + std::to_string(st.health), x, y, scale);
    DrawPixelText(renderer_, "MP " + std::to_string(st.mana), x, y + 20, scale);

    // Two bars, so a spell's effect is visible as a shape and not only a number.
    const int barW = std::min(160, windowWidth_ / 4);
    const int barH = 8;
    const int hp = std::clamp(st.health, 0, 100);
    SDL_SetRenderDrawColor(renderer_, 70, 80, 95, 255);
    SDL_Rect hpTrack = {x, y + 44, barW, barH};
    SDL_RenderFillRect(renderer_, &hpTrack);
    SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255);
    SDL_Rect hpFill = {x, y + 44, barW * hp / 100, barH};
    SDL_RenderFillRect(renderer_, &hpFill);
}

void PlayState::DrawStack(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "STACK", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    if (vm_.stack().empty()) {
        DrawPixelText(renderer_, "EMPTY", x, y + 24, scale);
        return;
    }
    // Top of stack first -- the value the next instruction will take.
    for (std::size_t i = 0; i < vm_.stack().size() && i < 6; ++i) {
        const int v = vm_.stack()[vm_.stack().size() - 1 - i];
        DrawPixelText(renderer_, (i == 0 ? "> " : "  ") + std::to_string(v), x,
                      y + 24 + static_cast<int>(i) * 20, scale);
    }
}

void PlayState::DrawSpells(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "SPELLS", x, y, scale);
    for (std::size_t i = 0; i < Spells().size(); ++i) {
        const bool selected = (i == spellIndex_);
        SDL_SetRenderDrawColor(renderer_, selected ? kCursor.r : kRow.r,
                               selected ? kCursor.g : kRow.g,
                               selected ? kCursor.b : kRow.b, 255);
        DrawPixelText(renderer_,
                      (selected ? "> " : "  ") + std::to_string(i + 1) + " " +
                          Spells()[i].name,
                      x, y + 24 + static_cast<int>(i) * 20, 3);
    }
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "SPACE STEP  ENTER RUN  R RESET  1-4 SPELL  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    const int scale = 4;
    const int colR = windowWidth_ / 2 + pad;

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "BYTECODE", pad, pad, scale);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_,
                  "SPELL " + std::to_string(spellIndex_ + 1) + " " +
                      Spells()[spellIndex_].name,
                  pad, pad + 28, scale);

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "PROGRAM", pad, 96, scale);
    DrawProgram(pad, 124, 3);

    // The spell list, which is also the key legend: without it the hint says
    // "1-4 SPELL" and the screen never says which is which. It sits below the
    // longest program (10 rows from 124 end at 324) so it cannot collide with a
    // longer one, and it fills what was otherwise ~190px of empty screen.
    DrawSpells(pad, 348, scale);

    DrawState(colR, 96, scale);
    DrawStack(colR, 200, scale);

    SDL_SetRenderDrawColor(renderer_,
                           vm_.faulted() ? kBad.r : kInk.r,
                           vm_.faulted() ? kBad.g : kInk.g,
                           vm_.faulted() ? kBad.b : kInk.b, 255);
    DrawPixelText(renderer_, "STATUS " + status_, colR, 348, scale);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
