#pragma once

#include <cstdint>
#include <vector>

// ── The instruction set, as DATA ─────────────────────────────────────────────
//
// This header is the whole point of the Bytecode pattern: behaviour that a
// designer can author, ship and change WITHOUT a compiler. A spell is a
// `Program` -- a vector of instructions -- and the machine that runs it
// (bytecodeVm.h) knows nothing about spells.
//
// The chapter's own framing: the pattern is for when "your game's behaviour is
// not something you can express in data" is FALSE -- when the interesting part
// IS data, but the ways of combining it are code. Here the combinations are the
// opcodes and the content is the program.
//
// ⚠️ An instruction MAY carry an operand and most do not, which is the chapter's
// own design note: `PushLiteral` needs a value, `HalveHealth` needs nothing. One
// struct with an operand that some opcodes ignore is simpler than two
// instruction types, and the alternative -- a separate `Literal` instruction
// that mutates the *next* instruction -- is the trap the chapter warns about
// because it makes a program unreadable and unseekable.

namespace bytecode {

enum class OpCode : std::uint8_t {
    PushLiteral,  // operand: push the value
    SetHealth,    // pop -> health
    SetMana,      // pop -> mana
    AddHealth,    // pop -> health += value
    AddMana,      // pop -> mana += value
    HalveHealth,  // health /= 2, no operand (rounds down; health is an int)
    Halt,
};

struct Instruction {
    OpCode op = OpCode::Halt;
    int    operand = 0;   // read by PushLiteral, ignored by everything else
};

using Program = std::vector<Instruction>;

// A name for each opcode, for the demo's listing and for error messages.
//
// Pure and total: every opcode has a name, which `specs/bytecodeVm.spec.cpp`
// asserts by walking the enum -- so an opcode added without a name is a failing
// test rather than a blank row on screen. That is the same coverage contract the
// glyph table has, for the same reason: a table with a hole in it fails silently.
inline const char *OpCodeName(OpCode op) {
    switch (op) {
    case OpCode::PushLiteral: return "PUSH";
    case OpCode::SetHealth:   return "SET_HP";
    case OpCode::SetMana:     return "SET_MP";
    case OpCode::AddHealth:   return "ADD_HP";
    case OpCode::AddMana:     return "ADD_MP";
    case OpCode::HalveHealth: return "HALVE_HP";
    case OpCode::Halt:        return "HALT";
    }
    return "?";
}

// Every opcode, in order -- the list a coverage test walks. Kept beside the enum
// so adding a case is a visible edit in one file.
inline const OpCode *AllOpcodes() {
    static const OpCode all[] = {
        OpCode::PushLiteral, OpCode::SetHealth,   OpCode::SetMana,
        OpCode::AddHealth,   OpCode::AddMana,     OpCode::HalveHealth,
        OpCode::Halt,
    };
    return all;
}
inline constexpr int kOpCodeCount = 7;

} // namespace bytecode
