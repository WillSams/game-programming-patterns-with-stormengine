#pragma once

#include "instruction.h"

// ── Spells, authored as DATA ─────────────────────────────────────────────────
//
// Each of these is a program a designer could have written in a text file and
// the game could have loaded. Adding one is adding data -- no new C++, no new
// opcode, no rebuild of the machine.
//
// ⚠️ The two shapes here are deliberate:
//
//   * `MinorHeal` sets health first and then adds to it, which is what makes it
//     a *program* rather than a constant: the same two opcodes in a different
//     order are a different spell.
//   * `DrainLife` uses an instruction with NO operand next to ones with, and
//     reads the stack the same way -- the machine does not care which.
//
// A `Program` that runs off its end HALTS implicitly (see BytecodeVm::step), so
// a trailing Halt is a choice rather than a requirement -- kept here because a
// reader of the data should not have to know that rule to see where it ends.

namespace bytecode {

inline const Program kMinorHeal = {
    {OpCode::PushLiteral, 20}, {OpCode::SetHealth},
    {OpCode::PushLiteral, 5},  {OpCode::AddHealth},
    {OpCode::PushLiteral, 8},  {OpCode::SetMana},
    {OpCode::Halt},
};

inline const Program kDrainLife = {
    {OpCode::PushLiteral, 60}, {OpCode::SetHealth},
    {OpCode::PushLiteral, 30}, {OpCode::SetMana},
    {OpCode::HalveHealth},
    {OpCode::Halt},
};

inline const Program kRitual = {
    {OpCode::PushLiteral, 4},  {OpCode::SetHealth},
    {OpCode::PushLiteral, 4},  {OpCode::AddHealth},
    {OpCode::PushLiteral, 4},  {OpCode::AddMana},
    {OpCode::PushLiteral, 10}, {OpCode::AddMana},
    {OpCode::HalveHealth},
    {OpCode::Halt},
};

// ⚠️ A DELIBERATELY BROKEN program, and it is part of the demo rather than a
// test fixture: a machine that runs data from outside the program has to survive
// data that is wrong, and the interesting failure is the stack underflow --
// `AddHealth` with nothing pushed. It is what the chapter leaves out and what a
// real implementation cannot.
inline const Program kFaultySpell = {
    {OpCode::PushLiteral, 12}, {OpCode::SetHealth},
    {OpCode::AddHealth},   // pops an EMPTY stack -> fault
    {OpCode::Halt},
};

} // namespace bytecode
