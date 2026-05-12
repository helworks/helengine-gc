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

        /// Returns whether the backend should continue reporting input while unfocused.
        bool get_ReceiveInputInBackground() override;

        /// Updates whether the backend should continue reporting input while unfocused.
        void set_ReceiveInputInBackground(bool value) override;

        /// Captures one bootstrap input frame with default keyboard, mouse, and pointer state.
        InputFrameState CaptureFrame() override;

    private:
        /// Tracks whether the backend should continue reporting input while unfocused.
        bool ReceiveInputInBackgroundValue;
    };
}
