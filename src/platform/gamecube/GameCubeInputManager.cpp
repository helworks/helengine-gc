#include "platform/gamecube/GameCubeInputManager.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube input backend with background input disabled.
    GameCubeInputManager::GameCubeInputManager() {
    }

    /// Releases the GameCube input backend.
    GameCubeInputManager::~GameCubeInputManager() {
    }

    /// Captures one bootstrap input frame with default keyboard, mouse, and pointer state.
    InputFrameState GameCubeInputManager::CaptureFrame() {
        InputFrameState frame;
        frame.set_GamepadCount(0);
        return frame;
    }
}
