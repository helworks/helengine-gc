#pragma once

#include "Keyboard.hpp"
#include "KeyboardState.hpp"

namespace helengine::gamecube {
    /// Implements the generated keyboard contract for the current GameCube bootstrap with an empty key state.
    class GameCubeKeyboard : public Keyboard {
    public:
        /// Creates the keyboard backend with inactive capture state.
        GameCubeKeyboard();

        /// Returns the current keyboard snapshot for the bootstrap host.
        KeyboardState GetState() override;

        /// Enables or disables keyboard capture for the backend.
        void SetActive(bool isActive) override;

    private:
        /// Tracks whether keyboard capture is currently enabled for the backend.
        bool IsActive;
    };
}
