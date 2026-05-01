#pragma once

#include "InputManager.hpp"

namespace helengine::gamecube {
    class GameCubeKeyboard;
    class GameCubeMouse;

    /// Wires the generated input manager to the minimal GameCube keyboard and mouse bootstrap backends.
    class GameCubeInputManager : public InputManager {
    public:
        /// Creates the generated input bridge with owned bootstrap keyboard and mouse backends.
        GameCubeInputManager();

        /// Releases the owned bootstrap keyboard and mouse backends.
        ~GameCubeInputManager();

    private:
        /// Stores the owned keyboard backend.
        GameCubeKeyboard* NativeKeyboard;

        /// Stores the owned mouse backend.
        GameCubeMouse* NativeMouse;
    };
}
