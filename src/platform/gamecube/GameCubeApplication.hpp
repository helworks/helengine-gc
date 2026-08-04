#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <gccore.h>

#include "platform/gamecube/GameCubeBootPhase.hpp"

class Core;
class PlatformInfo;
class IAudioBackend;
struct PPCContext;

namespace helengine::gamecube {
    class GameCubeInputManager;
    enum class GameCubeMemoryCardDiagnosticFailure : uint32_t;
    class GameCubeMemoryCardDiagnosticJournal;
    class GameCubeRenderManager2D;
    class GameCubeRenderManager3D;
    class GameCubeExceptionDiagnostics;
    class GameCubeSceneTransitionTraceDiagnostics;

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
    /// Updates the active direct-frame checkpoint from a GameCube native rendering boundary.
    /// <param name="code">Four hexadecimal digits that identify the boundary about to execute.</param>
    void ReportDirectFrameDiagnosticCode(uint16_t code);
#endif

#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
    /// Latches one rotating-logo anomaly code for presentation after the current normal frame completes.
    /// <param name="code">Four hexadecimal digits that identify the detected rotation anomaly.</param>
    void LatchLogoAnimationDiagnosticCode(uint16_t code);
#endif

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
        friend class GameCubeSceneTransitionTraceDiagnostics;
#if HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC
        friend class GameCubeExceptionDiagnostics;
#endif
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        friend void ReportDirectFrameDiagnosticCode(uint16_t code);
#endif
#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
        friend void LatchLogoAnimationDiagnosticCode(uint16_t code);
#endif

        /// Initializes the VI display state and allocates the first framebuffer.
        bool InitializeVideo();

        /// Initializes GX for the host clear-and-present loop.
        bool InitializeGraphics();

        /// Initializes the optional persistent diagnostic journal after video and GX are ready.
        /// <returns>True when no journal is enabled or the enabled journal became writable; otherwise false after presenting its failure color.</returns>
        bool InitializeMemoryCardDiagnosticJournal();

        /// Resolves the persistent diagnostic color assigned to one failed CARD initialization operation.
        /// <param name="failure">The CARD operation that failed.</param>
        /// <param name="resultCode">Native CARD result returned by the failed operation.</param>
        /// <returns>Visible boot-frame color that uniquely identifies the failure.</returns>
        GXColor GetMemoryCardDiagnosticFailureColor(GameCubeMemoryCardDiagnosticFailure failure, s32 resultCode) const;

        /// <summary>
        /// Verifies that Nintendont's temporary DOL-entry trampoline completed before the generated game entry began executing.
        /// </summary>
        /// <returns>True when the handoff word contains a valid GameCube DOL entry address; otherwise false after presenting the failed-handoff color.</returns>
        bool VerifyNintendontHandoff();

        /// Initializes packaged-disc access without starting the generated engine core.
        bool InitializePackagedDisc();

        /// Initializes the minimal-sample triangle state used to validate the host loop with a known-good GX sample.
        bool InitializeMinimalSample();

        /// Draws one known-good libogc sample triangle through the application-owned host loop.
        void DrawMinimalSample();

        /// Presents the current boot-phase color through VI or GX, depending on which native subsystem is ready.
        void PresentBootFrame();

        /// Presents one timed first-frame trace checkpoint when the optional runtime trace is enabled.
        /// <param name="color">Visible color that identifies the next runtime operation to execute.</param>
        void PresentFirstFrameTraceCheckpoint(GXColor color);

#if HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC
        /// Presents one fatal PowerPC exception report through the active VI framebuffer and does not return.
        /// <param name="exceptionId">Native PowerPC exception identifier.</param>
        /// <param name="context">Saved processor context at the faulting instruction.</param>
        /// <param name="faultAddress">Data address that caused a DSI, or zero for other exception types.</param>
        /// <param name="dataStorageInterruptStatus">PowerPC DSISR contents for a DSI, or zero for other exception types.</param>
        /// <param name="sceneListCount">Scene-record list count captured immediately before the suspected insertion.</param>
        /// <param name="sceneListCapacity">Scene-record list capacity captured immediately before the suspected insertion.</param>
        /// <param name="sceneListData">Scene-record list storage address captured immediately before the suspected insertion.</param>
        void PresentExceptionDiagnostics(
            unsigned exceptionId,
            const PPCContext* context,
            uint32_t faultAddress,
            uint32_t dataStorageInterruptStatus,
            int32_t sceneListCount,
            int32_t sceneListCapacity,
            const void* sceneListData);
#endif

        /// Presents the current boot-phase color through VI without submitting GX work, for diagnostics that isolate the display-copy pipeline.
        void PresentVideoBootFrame();

        /// Converts one RGB diagnostic color into the packed YCbYCr framebuffer value required by VIDEO_ClearFrameBuffer.
        static uint32_t ConvertToVideoClearColor(const GXColor& color);

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC || HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
        /// Writes one four-digit native checkpoint into both external framebuffers.
        /// <param name="code">Four hexadecimal digits that identify the checkpoint to draw.</param>
        void WriteDirectFrameDiagnosticCode(uint16_t code);

        /// Returns the three-bit row pattern for one hexadecimal glyph in the native checkpoint overlay.
        /// <param name="hexDigit">Hexadecimal digit in the inclusive range zero through fifteen.</param>
        /// <param name="row">Glyph row in the inclusive range zero through four.</param>
        /// <returns>Three foreground-bit flags ordered from the left-most to the right-most glyph column.</returns>
        static uint8_t GetDirectFrameDiagnosticGlyphRow(uint8_t hexDigit, uint8_t row);
#endif

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        /// Records a numeric native checkpoint and makes it visible without using GX, fonts, or the generated engine.
        /// <param name="code">Four hexadecimal digits that identify the last completed checkpoint.</param>
        void DisplayDirectFrameDiagnosticCode(uint16_t code);

        /// Advances the native progress counter observed by the VI retrace watchdog.
        void AdvanceDirectFrameDiagnosticHeartbeat();

        /// Handles one VI retrace and replaces a stale checkpoint with DEAD when native progress has stopped.
        /// <param name="retraceCount">VI retrace count supplied by libogc.</param>
        static void HandleDirectFrameDiagnosticRetrace(uint32_t retraceCount);

#endif

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

        /// Returns the optional runtime test-scene override that should bypass packaged scene loading when present.
        static std::string GetRuntimeTestSceneOverride();

        /// Advances one engine frame when the generated core was initialized successfully.
        bool UpdateEngineCore();

        /// Measures elapsed frame time with libogc's monotonic GameCube timebase.
        double MeasureElapsedFrameSeconds();

        /// Reports the GameCube host clock and scene state at a low fixed cadence when direct diagnostics are enabled.
        void ReportRuntimeFrameTelemetry(double elapsedSeconds);

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

        /// Tracks whether GX initialization completed and boot frames can use the GX display-copy path.
        bool GraphicsInitialized;

        /// Counts the number of frames presented after generated-core initialization succeeds.
        uint32_t PresentedFrameCount;

        /// Stores the libogc timebase tick sampled for the preceding generated-engine update.
        u64 PreviousFrameTicks;

        /// Stores the host frame duration that must be reported after the corresponding draw has captured its 2D queues.
        double LastElapsedFrameSeconds;

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

        /// Tracks whether the optional first-frame runtime trace reached its final success checkpoint.
        bool FirstFrameTraceCompleted;

        /// Tracks whether the automated probe detected a shared triangle disappearance during verification.
        bool VerificationProbeFailed;

        /// Tracks whether the center probe pixel was ever observed as visible during verification sampling.
        bool VerificationCenterVisibleOnce;

        /// <summary>
        /// Stores Nintendont's uncached handoff word written by its temporary DOL-entry trampoline.
        /// </summary>
        static constexpr uintptr_t NintendontHandoffStatusAddress = 0xD3003420U;

        /// <summary>
        /// Stores the transient value written by Nintendont while the temporary entry trampoline waits for the kernel patch pass.
        /// </summary>
        static constexpr uint32_t NintendontTrampolinePendingStatus = 0x4DEAU;

        /// <summary>
        /// Stores the lowest valid cached GameCube DOL entry address that Nintendont may return through its trampoline word.
        /// </summary>
        static constexpr uint32_t NintendontGameEntryMinimum = 0x80000000U;

        /// <summary>
        /// Stores the exclusive upper bound of valid cached GameCube DOL entry addresses.
        /// </summary>
        static constexpr uint32_t NintendontGameEntryMaximumExclusive = 0x81800000U;

        /// Stores the number of VI refreshes used to make each optional first-frame trace checkpoint observable.
        static constexpr uint32_t FirstFrameTraceCheckpointPresentationCount = 30U;

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        /// Stores the number of VI retraces without native progress before the diagnostic displays DEAD.
        static constexpr uint32_t DirectFrameDiagnosticStallRetraceLimit = 180U;

        /// Stores the active application instance used by the static VI retrace callback.
        static GameCubeApplication* DirectFrameDiagnosticApplication;

        /// Stores the VI post-retrace callback that was active before the diagnostic callback was installed.
        static VIRetraceCallback PreviousDirectFrameDiagnosticRetraceCallback;

        /// Counts native diagnostic checkpoints and steady-state frames observed by the VI watchdog.
        volatile uint32_t DirectFrameDiagnosticHeartbeat;

        /// Stores the latest heartbeat value consumed by the VI retrace watchdog.
        volatile uint32_t DirectFrameDiagnosticObservedHeartbeat;

        /// Counts uninterrupted VI retraces that observed no native diagnostic progress.
        volatile uint32_t DirectFrameDiagnosticStalledRetraceCount;

        /// Stores the last native checkpoint rendered into the direct framebuffer overlay.
        uint16_t DirectFrameDiagnosticCode;
#endif

#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
        /// Stores the active application instance used by the rotating-logo anomaly reporter.
        static GameCubeApplication* LogoAnimationDiagnosticApplication;

        /// Tracks whether a rotating-logo anomaly was observed and must remain visible on subsequent frames.
        bool LogoAnimationDiagnosticLatched;

        /// Stores the first rotating-logo anomaly code reported by the native renderer.
        uint16_t LogoAnimationDiagnosticCode;
#endif

        /// Tracks whether the marker probe pixel was ever observed as visible during verification sampling.
        bool VerificationMarkerVisibleOnce;

        /// Counts consecutive verification samples where both probe pixels were absent after first visibility.
        uint32_t VerificationMissingSampleCount;

#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        /// Stores the optional Slot-A journal used to persist hardware startup checkpoints.
        GameCubeMemoryCardDiagnosticJournal* MemoryCardDiagnosticJournal;

        /// Tracks whether the first generated-core update checkpoint sequence has been written.
        bool HasRecordedCoreUpdate;

        /// Tracks whether the first generated-core draw checkpoint sequence has been written.
        bool HasRecordedCoreDraw;
#endif

        /// Stores the position array used by the packaged minimal-sample control triangle.
        s16 MinimalSampleVertices[9];

        /// Stores the color array used by the packaged minimal-sample control triangle.
        u8 MinimalSampleColors[12];

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        /// Stores the optional generated scene-transition diagnostics sink used by the first-frame trace.
        GameCubeSceneTransitionTraceDiagnostics* SceneTransitionTraceDiagnostics;

        /// Stores the generated engine core instance when the build includes generated sources.
        Core* EngineCore;

        /// Stores the generated 3D render manager bridge.
        GameCubeRenderManager3D* EngineRenderManager3D;

        /// Stores the generated 2D render manager bridge.
        GameCubeRenderManager2D* EngineRenderManager2D;

        /// Stores the generated input manager bridge.
        GameCubeInputManager* EngineInputManager;

        /// Stores the GameCube audio backend used by generated audio source components.
        IAudioBackend* EngineAudioBackend;

        /// Stores the platform descriptor passed into the generated core initialization contract.
        PlatformInfo* EnginePlatformInfo;
#endif
    };
}
