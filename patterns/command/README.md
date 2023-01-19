# Command Pattern

The command pattern is a way to handle input in a game by encapsulating input events as command objects. These command objects can then be processed by the game's state and logic, allowing for a clear separation of input handling and game logic.

In this example, the InputHandler class is responsible for handling input events from SDL and mapping them to the appropriate Command objects. The main game loop polls for input events, and if an event corresponds to a mapped command, it is executed.
