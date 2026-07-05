#include <igloo/igloo_alt.h>
#include <memory>

#include "../src/commands/Command.h"
#include "../src/commands/MoveLeftCommand.h"
#include "../src/commands/MoveRightCommand.h"
#include "../src/commands/JumpCommand.h"

using namespace igloo;

// The Command pattern: each concrete command encapsulates an action (here, a
// Direction). The same code can invoke any command through a Command*.
Describe(CommandSpec) {

    It(should_resolve_a_move_left_command_to_left) {
        MoveLeftCommand command;
        Assert::That(command.getDirection(), Equals(LEFT));
    };

    It(should_resolve_a_move_right_command_to_right) {
        MoveRightCommand command;
        Assert::That(command.getDirection(), Equals(RIGHT));
    };

    It(should_resolve_a_jump_command_to_up) {
        JumpCommand command;
        Assert::That(command.getDirection(), Equals(UP));
    };

    It(should_dispatch_through_a_base_command_pointer) {
        std::unique_ptr<Command> command = std::make_unique<MoveRightCommand>();
        Assert::That(command->getDirection(), Equals(RIGHT));
    };
};
