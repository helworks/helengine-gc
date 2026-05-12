#include "platform/gamecube/GameCubeInputManager.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube input backend with background input disabled.
    GameCubeInputManager::GameCubeInputManager()
        : ReceiveInputInBackgroundValue(false) {
    }

    /// Releases the GameCube input backend.
    GameCubeInputManager::~GameCubeInputManager() {
    }

    /// Returns whether the backend should continue reporting input while unfocused.
    bool GameCubeInputManager::get_ReceiveInputInBackground() {
        return ReceiveInputInBackgroundValue;
    }

    /// Updates whether the backend should continue reporting input while unfocused.
    void GameCubeInputManager::set_ReceiveInputInBackground(bool value) {
        ReceiveInputInBackgroundValue = value;
    }

    /// Captures one bootstrap input frame with default keyboard, mouse, and pointer state.
    InputFrameState GameCubeInputManager::CaptureFrame() {
        InputFrameState frame;
        frame.set_GamepadCount(0);
        return frame;
    }
}
