#pragma once

#include "IInputBackend.hpp"
#include "InputFrameState.hpp"

namespace helengine::gamecube {
    /// Implements the generated input backend contract for the bootstrap GameCube host.
    class GameCubeInputManager : public IInputBackend {
    public:
        /// Creates the GameCube input backend with background input disabled.
        GameCubeInputManager();

        /// Releases the GameCube input backend.
        ~GameCubeInputManager();

        /// Captures one bootstrap input frame with default keyboard, mouse, and pointer state.
        InputFrameState CaptureFrame() override;
    };
}
