#pragma once

#include <cstddef>
#include <vector>

#include "instruction.h"

// ── The machine ──────────────────────────────────────────────────────────────
//
// Pure C++, no SDL, no engine: this is the pattern's testable core, and
// `specs/bytecodeVm.spec.cpp` reaches it directly. The demo state
// (states/playState.cpp) only *draws* what this does.
//
// What it owns: a program counter, a stack, and the small amount of game state
// the instructions act on. What it does NOT own: any knowledge of spells,
// which are data (spells.h).
//
// ⚠️ THE STACK IS WHERE THE PATTERN PAYS OFF AND WHERE IT BITES. Instructions
// with no operand take their input from the stack, so a program is a sequence of
// small composable pieces rather than one opcode per combination -- but a
// program can then be WRONG in a way code cannot: `AddHealth` with nothing
// pushed. The chapter assumes well-formed bytecode; a machine that runs data
// from outside the program cannot, so a stack underflow FAULTS rather than
// reading whatever is there.
//
// TWO LIMITS, deliberately not implemented, named so they are decisions rather
// than surprises: the STACK IS UNBOUNDED (a program of a million PushLiterals
// grows the vector; a real machine caps the depth and faults on overflow), and
// `HalveHealth` uses integer division, so it rounds toward zero and a negative
// health is truncated rather than floored.
//
// ⚠️ AND A FAULT DOES NOT CHANGE THE STATE IT WAS ABOUT TO CHANGE. The operand
// is popped before the instruction runs, so the natural implementation faults
// after popping and leaves health untouched only by luck. `step()` reads without
// popping, and pops only once the instruction has what it needs -- otherwise a
// faulting `SetHealth` would consume a value and the next instruction would act
// on a stack the program did not describe.

namespace bytecode {

struct VmState {
    int health = 0;
    int mana = 0;
};

enum class StepResult {
    Advanced,   // one instruction ran
    Halted,     // a Halt, or the program ran off its end
    Faulted,    // the instruction could not run (stack underflow)
};

class BytecodeVm {
public:
    BytecodeVm() = default;
    explicit BytecodeVm(VmState start) : state_(start) {}

    // Load a program and remember the state a rewind returns to.
    void load(const Program &program, VmState start = {}) {
        program_ = program;
        start_ = start;
        rewind();
    }

    // Rewind to the start of the loaded program -- the demo's Reset key, and why
    // `load` and `rewind` are separate: re-running a spell must not require
    // re-supplying it.
    //
    // ⚠️ IT RESTORES THE START STATE, which the first version did not: that one
    // cleared the counter and the stack but left health and mana where the last
    // run put them, so "reset" reset everything except the thing on screen.
    // Nothing caught it because every spell in spells.h happens to begin by
    // setting health -- the defect was hidden by the data, which is the worst
    // way for a defect to hide. `start_` is the one definition of where a
    // program begins.
    void rewind() {
        state_ = start_;
        pc_ = 0;
        stack_.clear();
        halted_ = false;
        faulted_ = false;
    }

    StepResult step();
    void run() {
        while (step() == StepResult::Advanced) { }
    }

    bool        halted() const { return halted_; }
    bool        faulted() const { return faulted_; }
    std::size_t pc() const { return pc_; }
    const VmState &state() const { return state_; }
    const std::vector<int> &stack() const { return stack_; }
    const Program &program() const { return program_; }

private:
    Program              program_;
    VmState              start_{};      // where rewind() returns to
    std::size_t          pc_ = 0;
    std::vector<int>     stack_;
    VmState              state_{};
    bool                 halted_ = false;
    bool                 faulted_ = false;
};

inline StepResult BytecodeVm::step() {
    if (halted_ || faulted_)
        return halted_ ? StepResult::Halted : StepResult::Faulted;

    // Running off the end is an implicit halt: a program need not end in Halt,
    // and requiring one would make every data file carry a redundant line.
    if (pc_ >= program_.size()) {
        halted_ = true;
        return StepResult::Halted;
    }

    const Instruction in = program_[pc_];

    // ⚠️ PEEK, DO NOT POP. See the note at the top: the value is taken only once
    // the instruction has everything it needs, so a fault cannot consume it.
    const auto peek = [this](int &out) {
        if (stack_.empty())
            return false;
        out = stack_.back();
        return true;
    };
    const auto drop = [this] { stack_.pop_back(); };

    int v = 0;
    switch (in.op) {
    case OpCode::PushLiteral:
        stack_.push_back(in.operand);
        break;

    case OpCode::SetHealth:
        if (!peek(v)) { faulted_ = true; return StepResult::Faulted; }
        drop();
        state_.health = v;
        break;

    case OpCode::SetMana:
        if (!peek(v)) { faulted_ = true; return StepResult::Faulted; }
        drop();
        state_.mana = v;
        break;

    case OpCode::AddHealth:
        if (!peek(v)) { faulted_ = true; return StepResult::Faulted; }
        drop();
        state_.health += v;
        break;

    case OpCode::AddMana:
        if (!peek(v)) { faulted_ = true; return StepResult::Faulted; }
        drop();
        state_.mana += v;
        break;

    case OpCode::HalveHealth:
        state_.health /= 2;   // integer division: documented, not accidental
        break;

    case OpCode::Halt:
        halted_ = true;
        return StepResult::Halted;
    }

    ++pc_;   // only a completed instruction advances the counter
    return StepResult::Advanced;
}

} // namespace bytecode
