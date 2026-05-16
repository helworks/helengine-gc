#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <gccore.h>

#include "platform/gamecube/GameCubeBootPhase.hpp"

class Core;
class PlatformInfo;

namespace helengine::gamecube {
    class GameCubeInputManager;
    class GameCubeRenderManager2D;
    class GameCubeRenderManager3D;

    /// Owns GameCube host startup, optional generated-core boot, and the steady-state frame loop.
    class GameCubeApplication {
    public:
        /// Creates the GameCube application with no initialized native or engine state.
        GameCubeApplication();

        /// Releases generated-core bridge objects after the application loop finishes.
        ~GameCubeApplication();

        /// Initializes the native host and enters the steady-state frame loop.
        int Run();

    private:
        /// Initializes the VI display state and allocates the first framebuffer.
        bool InitializeVideo();

        /// Initializes GX for the host clear-and-present loop.
        bool InitializeGraphics();

        /// Initializes packaged-disc access without starting the generated engine core.
        bool InitializePackagedDisc();

        /// Initializes the minimal-sample triangle state used to validate the host loop with a known-good GX sample.
        bool InitializeMinimalSample();

        /// Draws one known-good libogc sample triangle through the application-owned host loop.
        void DrawMinimalSample();

        /// Reads one byte range from the mounted GameCube disc using aligned sector transfers.
        bool ReadDiscRange(void* destination, std::size_t offset, std::size_t length);

        /// Validates the mounted GameCube retail disc header and packaged FST header fields.
        bool ValidatePackagedDiscLayout();

        /// Reads the packaged FST body and verifies it contains the expected startup-scene asset entry.
        bool ValidatePackagedDiscFileSystemTable();

        /// Reads one packaged scene asset from disc after resolving its file entry from the FST.
        bool ValidatePackagedSceneAssetRead();

        /// Reads the UTF-8 name stored for one FST entry.
        std::string ReadPackagedFstEntryName(const std::vector<uint8_t>& fstBytes, std::size_t entryIndex);

        /// Recursively resolves one packaged file path to its disc offset and byte length from the FST bytes.
        bool TryResolvePackagedFstFile(
            const std::vector<uint8_t>& fstBytes,
            std::size_t directoryEntryIndex,
            const std::string& directoryPath,
            const std::string& expectedPath,
            std::size_t& discOffset,
            std::size_t& fileSize);

        /// Initializes the generated engine core when generated sources are present in the build.
        bool InitializeEngineCore();

        /// Advances one engine frame when the generated core was initialized successfully.
        bool UpdateEngineCore();

        /// Draws one engine frame when the generated core was initialized successfully.
        bool DrawEngineCore();

        /// Presents one fallback frame to the active framebuffer.
        void PresentFrame();

        /// Resolves the currently visible diagnostic color for the next presented frame.
        GXColor ResolvePresentedClearColor();

        /// Updates the currently presented clear color used for boot-state diagnostics.
        void SetClearColor(GXColor color);

        /// Sets the current boot phase and visible clear color.
        void SetBootPhase(GameCubeBootPhase phase, GXColor color);

        /// Marks the current boot phase as failed and updates the visible clear color.
        void FailBootPhase(GameCubeBootPhase phase, GXColor color);

        /// Returns whether the current build was configured to stop after enough verified rendered frames.
        bool HasSatisfiedVerificationExitCondition();

        /// Returns whether the current build should convert runtime success and failure into deterministic process exits.
        bool IsVerificationBuild();

        /// Returns the process exit code that represents the current boot phase during verification runs.
        int GetVerificationExitCode();

        /// Stores the preferred video mode selected for the current console or emulator.
        GXRModeObj* RenderMode;

        /// Stores the two allocated external framebuffers used for display output.
        void* FrameBuffers[2];

        /// Stores the index of the next external framebuffer that will receive the copied display image.
        uint32_t FrameBufferIndex;

        /// Stores the current fallback clear color for crash-phase diagnostics.
        GXColor ClearColor;

        /// Stores the current host boot phase.
        GameCubeBootPhase BootPhase;

        /// Tracks whether the generated engine core finished initialization.
        bool EngineInitialized;

        /// Counts the number of frames presented after generated-core initialization succeeds.
        uint32_t PresentedFrameCount;

        /// Counts the number of generated-core frames that completed both update and draw.
        uint32_t VerifiedFrameCount;

        /// Tracks whether the current frame completed the generated update step before presentation.
        bool UpdateCompletedSincePresent;

        /// Tracks whether the current frame completed the generated draw step before presentation.
        bool DrawCompletedSincePresent;

        /// Tracks whether the first generated update boundary report has already been emitted.
        bool FirstUpdateBeginReported;

        /// Tracks whether the first generated update completion report has already been emitted.
        bool FirstUpdateCompletedReported;

        /// Tracks whether the first generated draw boundary report has already been emitted.
        bool FirstDrawBeginReported;

        /// Tracks whether the first generated draw completion report has already been emitted.
        bool FirstDrawCompletedReported;

        /// Tracks whether the automated probe detected a shared triangle disappearance during verification.
        bool VerificationProbeFailed;

        /// Tracks whether the center probe pixel was ever observed as visible during verification sampling.
        bool VerificationCenterVisibleOnce;

        /// Tracks whether the marker probe pixel was ever observed as visible during verification sampling.
        bool VerificationMarkerVisibleOnce;

        /// Counts consecutive verification samples where both probe pixels were absent after first visibility.
        uint32_t VerificationMissingSampleCount;

        /// Stores the position array used by the packaged minimal-sample control triangle.
        s16 MinimalSampleVertices[9];

        /// Stores the color array used by the packaged minimal-sample control triangle.
        u8 MinimalSampleColors[12];

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        /// Stores the generated engine core instance when the build includes generated sources.
        Core* EngineCore;

        /// Stores the generated 3D render manager bridge.
        GameCubeRenderManager3D* EngineRenderManager3D;

        /// Stores the generated 2D render manager bridge.
        GameCubeRenderManager2D* EngineRenderManager2D;

        /// Stores the generated input manager bridge.
        GameCubeInputManager* EngineInputManager;

        /// Stores the platform descriptor passed into the generated core initialization contract.
        PlatformInfo* EnginePlatformInfo;
#endif
    };
}
