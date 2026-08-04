#pragma once

#include <cstdint>

#include "IInputBackend.hpp"
#include "InputFrameState.hpp"

namespace helengine::gamecube {
    class GameCubeMemoryCardDiagnosticJournal;

    /// Implements the generated input backend contract for the bootstrap GameCube host.
    class GameCubeInputManager : public IInputBackend {
    public:
        /// Creates the GameCube input backend with background input disabled.
        explicit GameCubeInputManager(GameCubeMemoryCardDiagnosticJournal* diagnosticJournal);

        /// Releases the GameCube input backend.
        ~GameCubeInputManager();

        /// Initializes the standard GameCube controller transport used by retail games and Nintendont's normal patch path.
        static void InitializePlatformInput(GameCubeMemoryCardDiagnosticJournal* diagnosticJournal);

        /// Captures one bootstrap input frame with default keyboard, mouse, and pointer state.
        InputFrameState CaptureFrame() override;

    };
}
