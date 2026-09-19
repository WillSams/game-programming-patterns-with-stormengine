#include <igloo/igloo_alt.h>

#include "../src/vm/bytecodeVm.h"
#include "../src/vm/spells.h"

using namespace igloo;
using namespace bytecode;

// The pattern's core, spec'd directly: no SDL, no engine, no window. Every case
// here is about the MACHINE, because the spells are data and data is what the
// ProgramsAsData block covers.
Describe(BytecodeVmSpec) {

  // A machine holding a given program, already loaded.
  static BytecodeVm With(const Program &program) {
    BytecodeVm vm;
    vm.load(program);
    return vm;
  }

  Describe(PushLiteral) {
    It(pushes_the_operand_onto_the_stack) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 7}});
      vm.step();
      Assert::That(vm.stack().size(), Equals(static_cast<std::size_t>(1)));
      Assert::That(vm.stack().back(), Equals(7));
    };

    It(advances_the_program_counter_by_one) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 7}});
      vm.step();
      Assert::That(vm.pc(), Equals(static_cast<std::size_t>(1)));
    };
  };

  // The chapter's design note made testable: instructions WITH an operand and
  // instructions WITHOUT one both read the stack the same way, so the machine
  // does not need to know which is which.
  Describe(InstructionsThatTakeTheirValueFromTheStack) {
    It(sets_health_from_the_top_of_the_stack) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 42}, {OpCode::SetHealth}});
      vm.run();
      Assert::That(vm.state().health, Equals(42));
    };

    It(adds_to_the_current_health) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 10}, {OpCode::SetHealth},
                            {OpCode::PushLiteral, 5},  {OpCode::AddHealth}});
      vm.run();
      Assert::That(vm.state().health, Equals(15));
    };

    It(sets_and_adds_mana_the_same_way) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 3}, {OpCode::SetMana},
                            {OpCode::PushLiteral, 9}, {OpCode::AddMana}});
      vm.run();
      Assert::That(vm.state().mana, Equals(12));
    };

    It(consumes_the_value_it_used) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 42}, {OpCode::SetHealth}});
      vm.run();
      Assert::That(vm.stack().empty(), IsTrue());
    };

    It(halves_health_and_rounds_down) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 25}, {OpCode::SetHealth},
                            {OpCode::HalveHealth}});
      vm.run();
      Assert::That(vm.state().health, Equals(12));   // integer division, on purpose
    };
  };

  Describe(Halting) {
    It(halts_on_halt) {
      BytecodeVm vm = With({{OpCode::Halt}});
      Assert::That(static_cast<int>(vm.step()), Equals(static_cast<int>(StepResult::Halted)));
      Assert::That(vm.halted(), IsTrue());
    };

    // A program need not end in Halt: requiring one would make every data file
    // carry a redundant line.
    It(halts_when_the_program_runs_off_its_end) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 1}});
      Assert::That(static_cast<int>(vm.step()), Equals(static_cast<int>(StepResult::Advanced)));
      Assert::That(static_cast<int>(vm.step()), Equals(static_cast<int>(StepResult::Halted)));
      Assert::That(vm.halted(), IsTrue());
    };

    It(does_not_move_the_counter_past_a_halt) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 1}, {OpCode::Halt},
                            {OpCode::PushLiteral, 2}});
      vm.run();
      Assert::That(vm.pc(), Equals(static_cast<std::size_t>(1)));
    };

    It(reports_halted_on_every_later_step) {
      BytecodeVm vm = With({{OpCode::Halt}});
      vm.run();
      Assert::That(static_cast<int>(vm.step()), Equals(static_cast<int>(StepResult::Halted)));
    };
  };

  // ⚠️ The part the chapter leaves out, and the reason this machine exists in a
  // demo at all: a program is DATA, so it can be WRONG in a way code cannot --
  // `AddHealth` with nothing pushed. Well-formed bytecode is an assumption a
  // machine running data from outside the program does not get to make.
  Describe(Faults) {
    It(faults_when_an_instruction_finds_an_empty_stack) {
      BytecodeVm vm = With({{OpCode::AddHealth}});
      Assert::That(static_cast<int>(vm.step()), Equals(static_cast<int>(StepResult::Faulted)));
      Assert::That(vm.faulted(), IsTrue());
    };

    // THE PEEK-DON'T-POP RULE. Popping first and faulting after is the natural
    // implementation and it changes state the instruction never ran: a faulting
    // SetHealth would consume a value the next instruction then acts without.
    It(leaves_the_state_it_was_about_to_change_untouched) {
      BytecodeVm vm = With({{OpCode::PushLiteral, 12}, {OpCode::SetHealth},
                            {OpCode::AddHealth}});   // nothing left to pop
      vm.run();
      Assert::That(vm.faulted(), IsTrue());
      Assert::That(vm.state().health, Equals(12));   // NOT 0, and not a garbage read
    };

    It(does_not_advance_the_counter_on_a_fault) {
      BytecodeVm vm = With({{OpCode::AddHealth}});
      vm.step();
      Assert::That(vm.pc(), Equals(static_cast<std::size_t>(0)));
    };

    It(stays_faulted) {
      BytecodeVm vm = With({{OpCode::AddHealth}});
      vm.run();
      Assert::That(static_cast<int>(vm.step()), Equals(static_cast<int>(StepResult::Faulted)));
    };
  };

  // The point of the pattern: behaviour is DATA, so a new spell is a new value
  // and not a new class, a new opcode or a rebuild.
  Describe(ProgramsAsData) {
    It(runs_a_spell_written_as_data) {
      BytecodeVm vm = With(kMinorHeal);
      vm.run();
      Assert::That(vm.state().health, Equals(25));   // 20, then +5
      Assert::That(vm.state().mana, Equals(8));
      Assert::That(vm.halted(), IsTrue());
    };

    It(the_same_opcodes_in_a_different_order_are_a_different_spell) {
      BytecodeVm vm = With(kRitual);
      vm.run();
      Assert::That(vm.state().health, Equals(4));    // (4+4)/2
      Assert::That(vm.state().mana, Equals(14));     // 4+10
    };

    It(drains_life_without_any_new_opcode) {
      BytecodeVm vm = With(kDrainLife);
      vm.run();
      Assert::That(vm.state().health, Equals(30));   // 60/2
    };

    It(survives_a_spell_that_is_wrong) {
      BytecodeVm vm = With(kFaultySpell);
      vm.run();
      Assert::That(vm.faulted(), IsTrue());
      Assert::That(vm.state().health, Equals(12));
    };
  };

  // `load` and `rewind` are separate so re-running a spell does not mean
  // re-supplying it -- the demo's Reset key.
  Describe(Rewinding) {
    // ⚠️ THE REGRESSION THIS PINS: `rewind()` used to clear the counter and the
    // stack and leave health and mana where the last run put them, so the demo's
    // Reset key reset everything except the number on screen. Every spell in
    // spells.h begins by setting health, which is the only reason it looked fine.
    It(returns_to_the_start_state_it_was_loaded_with) {
      BytecodeVm vm;
      vm.load(kMinorHeal, VmState{100, 50});
      vm.run();
      Assert::That(vm.state().health, Equals(25));   // the spell ran

      vm.rewind();
      Assert::That(vm.state().health, Equals(100));  // and Reset undid it
      Assert::That(vm.state().mana, Equals(50));
      Assert::That(vm.pc(), Equals(static_cast<std::size_t>(0)));
      Assert::That(vm.stack().empty(), IsTrue());
      Assert::That(vm.halted(), IsFalse());
    };

    It(returns_to_zero_when_no_start_state_was_given) {
      BytecodeVm vm;
      vm.load(kMinorHeal);
      vm.run();
      vm.rewind();
      Assert::That(vm.state().health, Equals(0));
      Assert::That(vm.state().mana, Equals(0));
    };

    It(keeps_the_loaded_program_across_a_rewind) {
      BytecodeVm vm;
      vm.load(kDrainLife);
      vm.run();
      vm.rewind();
      Assert::That(vm.program().size(), Equals(kDrainLife.size()));
      vm.run();
      Assert::That(vm.state().health, Equals(30));   // the program still ran
    };

    It(clears_a_fault_on_rewind) {
      BytecodeVm vm;
      vm.load(kFaultySpell);
      vm.run();
      Assert::That(vm.faulted(), IsTrue());
      vm.rewind();
      Assert::That(vm.faulted(), IsFalse());
    };
  };

  // A table with a hole in it fails silently -- an opcode with no name would be
  // a blank row on screen. Walking the enum makes adding one without a name a
  // failing test instead.
  Describe(OpCodeCoverage) {
    It(gives_every_opcode_a_name) {
      const OpCode *all = AllOpcodes();
      for (int i = 0; i < kOpCodeCount; ++i) {
        const char *name = OpCodeName(all[i]);
        Assert::That(name != nullptr, IsTrue());
        Assert::That(std::string(name).empty(), IsFalse());
        Assert::That(std::string(name) == "?", IsFalse());
      }
    };
  };
};
