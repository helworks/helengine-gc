#include "platform/gamecube/GameCubeKeyboard.hpp"

namespace helengine::gamecube {
    /// Creates the keyboard backend with inactive capture state.
    GameCubeKeyboard::GameCubeKeyboard()
        : IsActive(false) {
    }

    /// Returns the current keyboard snapshot for the bootstrap host.
    KeyboardState GameCubeKeyboard::GetState() {
        if (!IsActive) {
            return KeyboardState();
        }

        return KeyboardState();
    }

    /// Enables or disables keyboard capture for the backend.
    void GameCubeKeyboard::SetActive(bool isActive) {
        IsActive = isActive;
    }
}
