#include "platform/gamecube/GameCubeInputManager.hpp"

#include "platform/gamecube/GameCubeKeyboard.hpp"
#include "platform/gamecube/GameCubeMouse.hpp"

namespace helengine::gamecube {
    /// Creates the generated input bridge with owned bootstrap keyboard and mouse backends.
    GameCubeInputManager::GameCubeInputManager()
        : InputManager()
        , NativeKeyboard(new GameCubeKeyboard())
        , NativeMouse(new GameCubeMouse()) {
        Keyboard = NativeKeyboard;
        Mouse = NativeMouse;
    }

    /// Releases the owned bootstrap keyboard and mouse backends.
    GameCubeInputManager::~GameCubeInputManager() {
        Keyboard = nullptr;
        Mouse = nullptr;
        delete NativeMouse;
        delete NativeKeyboard;
    }
}
