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

        /// Selects Nintendont's virtual-controller transport when available, or initializes libogc PAD on physical GameCube hardware.
        static void InitializePlatformInput(GameCubeMemoryCardDiagnosticJournal* diagnosticJournal);

        /// Captures one bootstrap input frame with default keyboard, mouse, and pointer state.
        InputFrameState CaptureFrame() override;

    private:
        /// Stores the optional durable boot journal used to bracket the first Nintendont PadStub invocation.
        GameCubeMemoryCardDiagnosticJournal* DiagnosticJournal;

        /// Stores whether the optional journal has already recorded a completed Nintendont PadStub invocation.
        bool HasRecordedNintendontPadRead;
    };
}
