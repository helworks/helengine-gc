#include "platform/gamecube/GameCubeApplication.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <string>
#include <vector>

#include <ogc/dvd.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/pad.h>
#include <ogc/system.h>

#include "platform/gamecube/GameCubeDiscReader.hpp"

#if HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC
#include <cstdio>

#include <ogc/console.h>
#include <tuxedo/ppc/context.h>

#include "platform/gamecube/GameCubeExceptionDiagnostics.hpp"
#endif

#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
#include "platform/gamecube/GameCubeMemoryCardDiagnosticJournal.hpp"
#endif

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "HostFileSystemContentStreamSource.hpp"
#include "CameraComponent.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "LoadedSceneRecord.hpp"
#include "PlatformInfo.hpp"
#if HELENGINE_GAMECUBE_HAS_GENERATED_RUNTIME_MODULE_REGISTRATION
#include "GeneratedRuntimeModuleRegistration.hpp"
#endif
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/native_exceptions.hpp"
#include "platform/gamecube/GameCubeCubeTestSceneInstaller.hpp"
#include "platform/gamecube/GameCubeInputManager.hpp"
#include "platform/gamecube/audio/GameCubeAudioBackend.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"
#include "platform/gamecube/GameCubeRenderManager3D.hpp"
#include "platform/gamecube/GameCubeSceneBootstrap.hpp"
#include "platform/gamecube/GameCubeSceneTransitionTraceDiagnostics.hpp"
#endif

namespace {
    constexpr const char* BuildStamp = __DATE__ " " __TIME__;
    u64 PendingSceneLoadStartTicks = 0;
    std::string PendingSceneLoadSceneId;

    void ReportCompletedSceneLoadIfPending() {
        if (PendingSceneLoadStartTicks == 0) {
            return;
        }

        const u64 elapsedTicks = gettime() - PendingSceneLoadStartTicks;
        const double elapsedMilliseconds = ticks_to_millisecs(elapsedTicks);
        SYS_Report(
            "[GC] Scene load to first draw scene=%s elapsedMs=%.3f\n",
            PendingSceneLoadSceneId.c_str(),
            elapsedMilliseconds);
        PendingSceneLoadStartTicks = 0;
        PendingSceneLoadSceneId.clear();
    }
}

extern "C" void GameCubeRecordSceneLoadRequest(const char* sceneId) {
    PendingSceneLoadStartTicks = gettime();
    PendingSceneLoadSceneId = sceneId != nullptr ? sceneId : "<null>";
}

extern "C" void GameCubeClearSceneLoadRequest() {
    PendingSceneLoadStartTicks = 0;
    PendingSceneLoadSceneId.clear();
}

namespace helengine::gamecube {
    namespace {
        constexpr std::size_t DefaultFifoSize = 256 * 1024;
        constexpr std::size_t DiscHeaderReadLength = 0x440;
        constexpr std::size_t DiscSectorSize = 2048;
        constexpr std::size_t FstEntrySize = 12;
        constexpr uint32_t DiscMagic = 0xC2339F3D;
        constexpr uint16_t ProbeCenterSampleX = 320;
        constexpr uint16_t ProbeCenterSampleY = 240;
        constexpr uint16_t ProbeMarkerSampleX = 110;
        constexpr uint16_t ProbeMarkerSampleY = 100;
        constexpr uint16_t ProbeBackgroundSampleX = 24;
        constexpr uint16_t ProbeBackgroundSampleY = 24;
        constexpr int16_t ProbeRegionOffsets[][2] = {
            { 0, 0 },
            { -10, 0 },
            { 10, 0 },
            { 0, -10 },
            { 0, 10 }
        };
        alignas(32) uint8_t FifoBufferStorage[DefaultFifoSize];

        uint32_t ReadBigEndianU32(const uint8_t* bytes) {
            return (static_cast<uint32_t>(bytes[0]) << 24)
                | (static_cast<uint32_t>(bytes[1]) << 16)
                | (static_cast<uint32_t>(bytes[2]) << 8)
                | static_cast<uint32_t>(bytes[3]);
        }

#if HELENGINE_GAMECUBE_MINIMAL_SAMPLE && (HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT > 0)
        bool ShouldReportProbePixels(uint32_t frame) {
            return frame <= 5U || (frame % 60U) == 0U;
        }

        void PeekProbePixels(GXColor& centerColor, GXColor& markerColor, GXColor& backgroundColor) {
            GX_PeekARGB(ProbeCenterSampleX, ProbeCenterSampleY, &centerColor);
            GX_PeekARGB(ProbeMarkerSampleX, ProbeMarkerSampleY, &markerColor);
            GX_PeekARGB(ProbeBackgroundSampleX, ProbeBackgroundSampleY, &backgroundColor);
        }

        void ReportProbePixels(uint32_t frame, const GXColor& centerColor, const GXColor& markerColor, const GXColor& backgroundColor) {
            SYS_Report(
                "[GC] Probe pixels frame=%u center=(%02X %02X %02X %02X) marker=(%02X %02X %02X %02X) background=(%02X %02X %02X %02X)\n",
                frame,
                centerColor.r,
                centerColor.g,
                centerColor.b,
                centerColor.a,
                markerColor.r,
                markerColor.g,
                markerColor.b,
                markerColor.a,
                backgroundColor.r,
                backgroundColor.g,
                backgroundColor.b,
                backgroundColor.a);
        }

        bool IsProbePixelVisible(const GXColor& sampleColor, const GXColor& backgroundColor) {
            constexpr int32_t VisibilityThreshold = 24;
            return std::abs(static_cast<int32_t>(sampleColor.r) - static_cast<int32_t>(backgroundColor.r)) > VisibilityThreshold
                || std::abs(static_cast<int32_t>(sampleColor.g) - static_cast<int32_t>(backgroundColor.g)) > VisibilityThreshold
                || std::abs(static_cast<int32_t>(sampleColor.b) - static_cast<int32_t>(backgroundColor.b)) > VisibilityThreshold;
        }

        bool IsProbeRegionVisible(uint16_t centerX, uint16_t centerY, const GXColor& backgroundColor) {
            GXColor sampleColor {};
            for (const auto& offset : ProbeRegionOffsets) {
                const uint16_t sampleX = static_cast<uint16_t>(static_cast<int32_t>(centerX) + offset[0]);
                const uint16_t sampleY = static_cast<uint16_t>(static_cast<int32_t>(centerY) + offset[1]);
                GX_PeekARGB(sampleX, sampleY, &sampleColor);
                if (IsProbePixelVisible(sampleColor, backgroundColor)) {
                    return true;
                }
            }

            return false;
        }
#endif
    }

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
    GameCubeApplication* GameCubeApplication::DirectFrameDiagnosticApplication = nullptr;
    VIRetraceCallback GameCubeApplication::PreviousDirectFrameDiagnosticRetraceCallback = nullptr;

    /// Updates the active direct-frame checkpoint from a GameCube native rendering boundary.
    void ReportDirectFrameDiagnosticCode(uint16_t code) {
        if (GameCubeApplication::DirectFrameDiagnosticApplication != nullptr) {
            GameCubeApplication::DirectFrameDiagnosticApplication->DisplayDirectFrameDiagnosticCode(code);
        }
    }
#endif

#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
    GameCubeApplication* GameCubeApplication::LogoAnimationDiagnosticApplication = nullptr;

    /// Latches one rotating-logo anomaly code for presentation after the current normal frame completes.
    void LatchLogoAnimationDiagnosticCode(uint16_t code) {
        GameCubeApplication* const application = GameCubeApplication::LogoAnimationDiagnosticApplication;
        if (application != nullptr
            && (!application->LogoAnimationDiagnosticLatched
                || (application->LogoAnimationDiagnosticCode == 0xA000U && code != 0xA000U))) {
            application->LogoAnimationDiagnosticCode = code;
            application->LogoAnimationDiagnosticLatched = true;
        }
    }
#endif

    /// Creates the GameCube application with no initialized native or engine state.
    GameCubeApplication::GameCubeApplication()
        : RenderMode(nullptr)
        , FrameBuffers { nullptr, nullptr }
        , FrameBufferIndex(0U)
        , ClearColor { 0xFF, 0x00, 0xFF, 0xFF }
        , BootPhase(GameCubeBootPhase::NativeVideo)
        , EngineInitialized(false)
        , GraphicsInitialized(false)
        , PresentedFrameCount(0)
        , PreviousFrameTicks(0U)
        , LastElapsedFrameSeconds(0.0)
        , VerifiedFrameCount(0)
        , UpdateCompletedSincePresent(false)
        , DrawCompletedSincePresent(false)
        , FirstUpdateBeginReported(false)
        , FirstUpdateCompletedReported(false)
        , FirstDrawBeginReported(false)
        , FirstDrawCompletedReported(false)
        , FirstFrameTraceCompleted(false)
        , VerificationProbeFailed(false)
        , VerificationCenterVisibleOnce(false)
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        , DirectFrameDiagnosticHeartbeat(0U)
        , DirectFrameDiagnosticObservedHeartbeat(0U)
        , DirectFrameDiagnosticStalledRetraceCount(0U)
        , DirectFrameDiagnosticCode(0x0000U)
#endif
#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
        , LogoAnimationDiagnosticLatched(false)
        , LogoAnimationDiagnosticCode(0x0000U)
#endif
        , VerificationMarkerVisibleOnce(false)
        , VerificationMissingSampleCount(0U)
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        , MemoryCardDiagnosticJournal(nullptr)
        , HasRecordedCoreUpdate(false)
        , HasRecordedCoreDraw(false)
#endif
        , MinimalSampleVertices { 0, 15, 0, -15, -15, 0, 15, -15, 0 }
        , MinimalSampleColors { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255 }
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        , SceneTransitionTraceDiagnostics(nullptr)
        , EngineCore(nullptr)
        , EngineRenderManager3D(nullptr)
        , EngineRenderManager2D(nullptr)
        , EngineInputManager(nullptr)
        , EngineAudioBackend(nullptr)
        , EnginePlatformInfo(nullptr)
#endif
    {
    }

    /// Releases generated-core bridge objects after the application loop finishes.
    GameCubeApplication::~GameCubeApplication() {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        if (DirectFrameDiagnosticApplication == this) {
            VIDEO_SetPostRetraceCallback(PreviousDirectFrameDiagnosticRetraceCallback);
            DirectFrameDiagnosticApplication = nullptr;
            PreviousDirectFrameDiagnosticRetraceCallback = nullptr;
        }
#endif
#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
        if (LogoAnimationDiagnosticApplication == this) {
            LogoAnimationDiagnosticApplication = nullptr;
        }
#endif
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        delete EngineCore;
        delete SceneTransitionTraceDiagnostics;
        delete EngineInputManager;
        delete EngineAudioBackend;
        delete EngineRenderManager2D;
        delete EngineRenderManager3D;
        delete EnginePlatformInfo;
#endif
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        delete MemoryCardDiagnosticJournal;
#endif
    }

    /// Returns the optional runtime test-scene override that should bypass packaged scene loading when present.
    std::string GameCubeApplication::GetRuntimeTestSceneOverride() {
        const char* environmentValue = std::getenv("HELENGINE_GAMECUBE_RUNTIME_TEST_SCENE");
        if (environmentValue == nullptr) {
            return std::string();
        }

        return environmentValue;
    }

    /// Initializes the native host and enters the steady-state frame loop.
    int GameCubeApplication::Run() {
        SetBootPhase(GameCubeBootPhase::NativeVideo, GXColor { 0xFF, 0x00, 0x00, 0xFF });
        if (!InitializeVideo()) {
            FailBootPhase(GameCubeBootPhase::NativeVideo, GXColor { 0x80, 0x00, 0x80, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

#if HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC
        GameCubeExceptionDiagnostics::Install(this);
#endif

        SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x00, 0x00, 0xFF, 0xFF });
        PresentBootFrame();
        if (!InitializeGraphics()) {
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x80, 0x00, 0x80, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

        if (!VerifyNintendontHandoff()) {
            while (true) {
                PresentBootFrame();
            }
        }

#if HELENGINE_GAMECUBE_NINTENDONT_HANDOFF_DIAGNOSTIC
        while (true) {
            PresentBootFrame();
        }
#endif

        if (!InitializeMemoryCardDiagnosticJournal()) {
            while (true) {
                PresentBootFrame();
            }
        }

#if !HELENGINE_GAMECUBE_HAS_GENERATED_CORE || HELENGINE_GAMECUBE_MINIMAL_SAMPLE
        if (!InitializeMinimalSample()) {
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x80, 0x00, 0x80, 0xFF });
            return 1;
        }
#endif

#if !HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        SetBootPhase(GameCubeBootPhase::Running, GXColor { 0x00, 0x00, 0x00, 0xFF });
        while (true) {
            DrawMinimalSample();
            PresentFrame();
        }
#endif

#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
#if !HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0x40, 0x80, 0xFF });
        if (!InitializePackagedDisc()) {
            FailBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

        SetBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0x80, 0x00, 0x00, 0xFF });
        if (!ValidatePackagedDiscLayout()) {
            FailBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

        SetBootPhase(GameCubeBootPhase::CoreConstruction, GXColor { 0xA0, 0x00, 0x00, 0xFF });
        if (!ValidatePackagedDiscFileSystemTable()) {
            FailBootPhase(GameCubeBootPhase::CoreConstruction, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

        SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xC0, 0x00, 0x00, 0xFF });
        if (!ValidatePackagedSceneAssetRead()) {
            FailBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

        SetBootPhase(GameCubeBootPhase::Running, GXColor { 0xFF, 0x00, 0x00, 0xFF });
        while (true) {
            PresentFrame();
        }
#endif
#endif

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!InitializeEngineCore()) {
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

        PresentFirstFrameTraceCheckpoint(GXColor { 0x20, 0x40, 0xFF, 0xFF });
#endif

        while (true) {
#if HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC
            if (FirstFrameTraceCompleted) {
                while (true) {
                    PresentBootFrame();
                }
            }
#endif
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
            if (!UpdateEngineCore()) {
                if (IsVerificationBuild()) {
                    return GetVerificationExitCode();
                }
                return 1;
            }

            if (!DrawEngineCore()) {
                if (IsVerificationBuild()) {
                    return GetVerificationExitCode();
                }
                return 1;
            }
#endif

            PresentFrame();

            if (HasSatisfiedVerificationExitCondition()) {
                return VerificationProbeFailed ? 2 : 0;
            }
        }

        return 0;
    }

    /// Initializes the VI display state and allocates the first framebuffer.
    bool GameCubeApplication::InitializeVideo() {
        VIDEO_Init();

        RenderMode = VIDEO_GetPreferredMode(nullptr);
        if (RenderMode == nullptr) {
            return false;
        }

        FrameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(RenderMode));
        FrameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(RenderMode));
        if (FrameBuffers[0] == nullptr || FrameBuffers[1] == nullptr) {
            return false;
        }

        VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], COLOR_BLACK);
        VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[1], COLOR_BLACK);

        VIDEO_Configure(RenderMode);
        VIDEO_SetNextFramebuffer(FrameBuffers[0]);
        VIDEO_SetBlack(FALSE);
        VIDEO_Flush();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        DirectFrameDiagnosticApplication = this;
        PreviousDirectFrameDiagnosticRetraceCallback = VIDEO_SetPostRetraceCallback(HandleDirectFrameDiagnosticRetrace);
        DisplayDirectFrameDiagnosticCode(0x0001U);
#endif
#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
        LogoAnimationDiagnosticApplication = this;
#endif
        VIDEO_WaitVSync();

        if (RenderMode->viTVMode & VI_NON_INTERLACE) {
            VIDEO_WaitVSync();
        }
        return true;
    }

    /// Presents the current boot-phase color through VI or GX, depending on which native subsystem is ready.
    void GameCubeApplication::PresentBootFrame() {
        if (!GraphicsInitialized) {
            PresentVideoBootFrame();
            return;
        }

        FrameBufferIndex ^= 1U;
        GX_SetCopyClear(ClearColor, 0x00FFFFFF);
        GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);
        GX_DrawDone();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        WriteDirectFrameDiagnosticCode(DirectFrameDiagnosticCode);
#endif
#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC && !HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        if (LogoAnimationDiagnosticLatched) {
            WriteDirectFrameDiagnosticCode(LogoAnimationDiagnosticCode);
        }
#endif
        VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }

    /// Presents one timed first-frame trace checkpoint when the optional runtime trace is enabled.
    void GameCubeApplication::PresentFirstFrameTraceCheckpoint(GXColor color) {
#if HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC
        SetClearColor(color);
        for (uint32_t presentationIndex = 0U; presentationIndex < FirstFrameTraceCheckpointPresentationCount; ++presentationIndex) {
            PresentBootFrame();
        }
#else
        static_cast<void>(color);
#endif
    }

#if HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC
    /// Presents one fatal PowerPC exception report through the active VI framebuffer and does not return.
    void GameCubeApplication::PresentExceptionDiagnostics(
        unsigned exceptionId,
        const PPCContext* context,
        uint32_t faultAddress,
        uint32_t dataStorageInterruptStatus,
        int32_t sceneListCount,
        int32_t sceneListCapacity,
        const void* sceneListData) {
        const uint32_t programCounter = context != nullptr ? context->pc : 0U;
        void* frameBuffer = FrameBuffers[FrameBufferIndex];
        if (RenderMode == nullptr || frameBuffer == nullptr) {
            while (true) {
            }
        }

        VIDEO_ClearFrameBuffer(RenderMode, frameBuffer, COLOR_BLACK);
        CON_Init(
            frameBuffer,
            16,
            16,
            static_cast<int>(RenderMode->fbWidth) - 32,
            static_cast<int>(RenderMode->xfbHeight) - 32,
            static_cast<int>(RenderMode->fbWidth) * 2);
        std::printf("HELENGINE GAMECUBE EXCEPTION\n\n");
        std::printf("EXCEPTION: %u\n", exceptionId);
        std::printf("PC:        %08lX\n", static_cast<unsigned long>(programCounter));
        std::printf("FAULT:     %08lX\n", static_cast<unsigned long>(faultAddress));
        std::printf("DSISR:     %08lX\n\n", static_cast<unsigned long>(dataStorageInterruptStatus));
        std::printf("SCENE LIST COUNT:    %ld\n", static_cast<long>(sceneListCount));
        std::printf("SCENE LIST CAPACITY: %ld\n", static_cast<long>(sceneListCapacity));
        std::printf("SCENE LIST DATA:     %08lX\n", static_cast<unsigned long>(reinterpret_cast<uintptr_t>(sceneListData)));
        VIDEO_SetNextFramebuffer(frameBuffer);
        VIDEO_SetBlack(FALSE);
        VIDEO_Flush();
        while (true) {
            VIDEO_WaitVSync();
        }
    }
#endif

    /// Presents the current boot-phase color through VI without submitting GX work, for diagnostics that isolate the display-copy pipeline.
    void GameCubeApplication::PresentVideoBootFrame() {
        const uint32_t videoClearColor = ConvertToVideoClearColor(ClearColor);
        VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[0], videoClearColor);
        VIDEO_ClearFrameBuffer(RenderMode, FrameBuffers[1], videoClearColor);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        WriteDirectFrameDiagnosticCode(DirectFrameDiagnosticCode);
#endif
        FrameBufferIndex ^= 1U;
        VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }

    /// Converts one RGB diagnostic color into the packed YCbYCr framebuffer value required by VIDEO_ClearFrameBuffer.
    uint32_t GameCubeApplication::ConvertToVideoClearColor(const GXColor& color) {
        const double red = static_cast<double>(color.r);
        const double green = static_cast<double>(color.g);
        const double blue = static_cast<double>(color.b);
        const uint8_t luminance = static_cast<uint8_t>(std::clamp(static_cast<int32_t>(16.0 + (0.257 * red) + (0.504 * green) + (0.098 * blue)), 0, 255));
        const uint8_t chromaBlue = static_cast<uint8_t>(std::clamp(static_cast<int32_t>(128.0 - (0.148 * red) - (0.291 * green) + (0.439 * blue)), 0, 255));
        const uint8_t chromaRed = static_cast<uint8_t>(std::clamp(static_cast<int32_t>(128.0 + (0.439 * red) - (0.368 * green) - (0.071 * blue)), 0, 255));
        return (static_cast<uint32_t>(luminance) << 24)
            | (static_cast<uint32_t>(chromaBlue) << 16)
            | (static_cast<uint32_t>(luminance) << 8)
            | static_cast<uint32_t>(chromaRed);
    }

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
    /// Records a numeric native checkpoint and makes it visible without using GX, fonts, or the generated engine.
    void GameCubeApplication::DisplayDirectFrameDiagnosticCode(uint16_t code) {
        DirectFrameDiagnosticCode = code;
        AdvanceDirectFrameDiagnosticHeartbeat();
        WriteDirectFrameDiagnosticCode(code);
    }

    /// Advances the native progress counter observed by the VI retrace watchdog.
    void GameCubeApplication::AdvanceDirectFrameDiagnosticHeartbeat() {
        DirectFrameDiagnosticHeartbeat++;
    }

    /// Handles one VI retrace and replaces a stale checkpoint with DEAD when native progress has stopped.
    void GameCubeApplication::HandleDirectFrameDiagnosticRetrace(uint32_t retraceCount) {
        static_cast<void>(retraceCount);
        GameCubeApplication* const application = DirectFrameDiagnosticApplication;
        if (application == nullptr) {
            return;
        }

        const uint32_t heartbeat = application->DirectFrameDiagnosticHeartbeat;
        if (heartbeat == application->DirectFrameDiagnosticObservedHeartbeat) {
            application->DirectFrameDiagnosticStalledRetraceCount++;
            if (application->DirectFrameDiagnosticStalledRetraceCount == DirectFrameDiagnosticStallRetraceLimit) {
                application->WriteDirectFrameDiagnosticCode(0xDEADU);
            }
        } else {
            application->DirectFrameDiagnosticObservedHeartbeat = heartbeat;
            application->DirectFrameDiagnosticStalledRetraceCount = 0U;
        }

        if (PreviousDirectFrameDiagnosticRetraceCallback != nullptr) {
            PreviousDirectFrameDiagnosticRetraceCallback(retraceCount);
        }
    }

    
#endif

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC || HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC
    /// Writes the current four-digit native checkpoint into both external framebuffers.
    void GameCubeApplication::WriteDirectFrameDiagnosticCode(uint16_t code) {
        if (RenderMode == nullptr || FrameBuffers[0] == nullptr || FrameBuffers[1] == nullptr) {
            return;
        }

        constexpr uint32_t foregroundColor = 0xEB80EB80U;
        constexpr uint32_t backgroundColor = 0x10801080U;
        constexpr uint32_t horizontalScale = 3U;
        constexpr uint32_t verticalScale = 4U;
        constexpr uint32_t glyphWidth = 3U;
        constexpr uint32_t glyphHeight = 5U;
        constexpr uint32_t glyphGap = 2U;
        constexpr uint32_t margin = 8U;
        constexpr uint32_t digitCount = 4U;
        const uint32_t framebufferWordWidth = static_cast<uint32_t>(RenderMode->fbWidth) / 2U;
        const uint32_t framebufferHeight = static_cast<uint32_t>(RenderMode->xfbHeight);
        const uint32_t overlayWidth = (digitCount * glyphWidth * horizontalScale) + ((digitCount - 1U) * glyphGap);
        const uint32_t overlayHeight = glyphHeight * verticalScale;

        if (framebufferWordWidth <= margin + overlayWidth || framebufferHeight <= margin + overlayHeight) {
            return;
        }

        for (uint32_t framebufferIndex = 0U; framebufferIndex < 2U; framebufferIndex++) {
            volatile uint32_t* const framebufferWords = static_cast<volatile uint32_t*>(FrameBuffers[framebufferIndex]);
            for (uint32_t y = 0U; y < overlayHeight; y++) {
                volatile uint32_t* const row = framebufferWords + ((margin + y) * framebufferWordWidth) + margin;
                for (uint32_t x = 0U; x < overlayWidth; x++) {
                    row[x] = backgroundColor;
                }
            }

            for (uint32_t digitIndex = 0U; digitIndex < digitCount; digitIndex++) {
                const uint32_t shift = (digitCount - 1U - digitIndex) * 4U;
                const uint8_t hexDigit = static_cast<uint8_t>((code >> shift) & 0x0FU);
                const uint32_t digitStartX = margin + (digitIndex * ((glyphWidth * horizontalScale) + glyphGap));
                for (uint32_t glyphRow = 0U; glyphRow < glyphHeight; glyphRow++) {
                    const uint8_t glyphBits = GetDirectFrameDiagnosticGlyphRow(hexDigit, static_cast<uint8_t>(glyphRow));
                    for (uint32_t verticalOffset = 0U; verticalOffset < verticalScale; verticalOffset++) {
                        volatile uint32_t* const row = framebufferWords + ((margin + (glyphRow * verticalScale) + verticalOffset) * framebufferWordWidth) + digitStartX;
                        for (uint32_t glyphColumn = 0U; glyphColumn < glyphWidth; glyphColumn++) {
                            const bool foreground = (glyphBits & (1U << (glyphWidth - 1U - glyphColumn))) != 0U;
                            const uint32_t pixelColor = foreground ? foregroundColor : backgroundColor;
                            for (uint32_t horizontalOffset = 0U; horizontalOffset < horizontalScale; horizontalOffset++) {
                                row[(glyphColumn * horizontalScale) + horizontalOffset] = pixelColor;
                            }
                        }
                    }
                }
            }
        }
    }

    /// Returns the three-bit row pattern for one hexadecimal glyph in the native checkpoint overlay.
    uint8_t GameCubeApplication::GetDirectFrameDiagnosticGlyphRow(uint8_t hexDigit, uint8_t row) {
        static constexpr uint8_t glyphRows[16][5] = {
            { 0x07U, 0x05U, 0x05U, 0x05U, 0x07U },
            { 0x02U, 0x06U, 0x02U, 0x02U, 0x07U },
            { 0x07U, 0x01U, 0x07U, 0x04U, 0x07U },
            { 0x07U, 0x01U, 0x07U, 0x01U, 0x07U },
            { 0x05U, 0x05U, 0x07U, 0x01U, 0x01U },
            { 0x07U, 0x04U, 0x07U, 0x01U, 0x07U },
            { 0x07U, 0x04U, 0x07U, 0x05U, 0x07U },
            { 0x07U, 0x01U, 0x02U, 0x02U, 0x02U },
            { 0x07U, 0x05U, 0x07U, 0x05U, 0x07U },
            { 0x07U, 0x05U, 0x07U, 0x01U, 0x07U },
            { 0x02U, 0x05U, 0x07U, 0x05U, 0x05U },
            { 0x06U, 0x05U, 0x06U, 0x05U, 0x06U },
            { 0x07U, 0x04U, 0x04U, 0x04U, 0x07U },
            { 0x06U, 0x05U, 0x05U, 0x05U, 0x06U },
            { 0x07U, 0x04U, 0x07U, 0x04U, 0x07U },
            { 0x07U, 0x04U, 0x07U, 0x04U, 0x04U }
        };
        return hexDigit < 16U && row < 5U ? glyphRows[hexDigit][row] : 0U;
    }
#endif

    /// Initializes the minimal-sample triangle state used to validate the host loop with a known-good GX sample.
    bool GameCubeApplication::InitializeMinimalSample() {
        PAD_Init();

        Mtx44 projection;
        guPerspective(projection, 60.0F, 1.33F, 10.0F, 300.0F);
        GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
        GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetArray(GX_VA_POS, MinimalSampleVertices, 3 * sizeof(s16));
        GX_SetArray(GX_VA_CLR0, MinimalSampleColors, 4 * sizeof(u8));
        GX_SetNumChans(1);
        GX_SetNumTexGens(0);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GX_SetCopyClear(GXColor { 0x00, 0x00, 0x00, 0xFF }, 0x00FFFFFF);
        return true;
    }

    /// Draws one known-good libogc sample triangle through the application-owned host loop.
    void GameCubeApplication::DrawMinimalSample() {
        guVector camera = { 0.0F, 0.0F, 0.0F };
        guVector up = { 0.0F, 1.0F, 0.0F };
        guVector look = { 0.0F, 0.0F, -1.0F };
        Mtx viewMatrix;
        guLookAt(viewMatrix, &camera, &up, &look);

        GX_SetViewport(0, 0, RenderMode->fbWidth, RenderMode->efbHeight, 0, 1);
        GX_InvVtxCache();
        GX_InvalidateTexAll();

        Mtx modelView;
        guMtxIdentity(modelView);
        guMtxTransApply(modelView, modelView, 0.0F, 0.0F, -50.0F);
        guMtxConcat(viewMatrix, modelView, modelView);
        GX_LoadPosMtxImm(modelView, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
        GX_Position1x8(0);
        GX_Color1x8(0);
        GX_Position1x8(1);
        GX_Color1x8(1);
        GX_Position1x8(2);
        GX_Color1x8(2);
        GX_End();
    }

    /// Initializes GX for the host clear-and-present loop.
    bool GameCubeApplication::InitializeGraphics() {
        GX_Init(FifoBufferStorage, DefaultFifoSize);

        const f32 yScale = GX_GetYScaleFactor(RenderMode->efbHeight, RenderMode->xfbHeight);
        const u16 xfbHeight = GX_SetDispCopyYScale(yScale);

        GX_SetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
        GX_SetDispCopySrc(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
        GX_SetDispCopyDst(RenderMode->fbWidth, xfbHeight);
        GX_SetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
        GX_SetFieldMode(RenderMode->field_rendering, ((RenderMode->viHeight == (RenderMode->xfbHeight * 2)) ? GX_ENABLE : GX_DISABLE));
        GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
        GX_SetCullMode(GX_CULL_NONE);
        GX_SetDispCopyGamma(GX_GM_1_0);
        GX_SetNumChans(1);
        GX_SetNumTexGens(0);
        GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_TRUE);
        GX_SetViewport(0.0F, 0.0F, static_cast<f32>(RenderMode->fbWidth), static_cast<f32>(RenderMode->efbHeight), 0.0F, 1.0F);
        SYS_Report(
            "[GC] Video mode fb=%ux%u efbHeight=%u xfbHeight=%u vi=%ux%u\n",
            static_cast<unsigned>(RenderMode->fbWidth),
            static_cast<unsigned>(RenderMode->efbHeight),
            static_cast<unsigned>(RenderMode->efbHeight),
            static_cast<unsigned>(xfbHeight),
            static_cast<unsigned>(RenderMode->viWidth),
            static_cast<unsigned>(RenderMode->viHeight));
        GX_InvVtxCache();
        GX_InvalidateTexAll();
        GX_SetCopyClear(GXColor { 0x00, 0x00, 0x00, 0xFF }, 0x00FFFFFF);
        GX_CopyDisp(FrameBuffers[0], GX_TRUE);
        GX_CopyDisp(FrameBuffers[0], GX_TRUE);
        GraphicsInitialized = true;

        return true;
    }

    /// Initializes the optional persistent diagnostic journal after video and GX are ready.
    bool GameCubeApplication::InitializeMemoryCardDiagnosticJournal() {
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0xFF, 0x80, 0x20, 0xFF });
        PresentBootFrame();
        MemoryCardDiagnosticJournal = new GameCubeMemoryCardDiagnosticJournal();
        if (!MemoryCardDiagnosticJournal->Initialize()) {
            const GameCubeMemoryCardDiagnosticFailure failure = MemoryCardDiagnosticJournal->GetInitializationFailure();
            const s32 resultCode = MemoryCardDiagnosticJournal->GetInitializationResultCode();
            delete MemoryCardDiagnosticJournal;
            MemoryCardDiagnosticJournal = nullptr;
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GetMemoryCardDiagnosticFailureColor(failure, resultCode));
            PresentBootFrame();
            return false;
        }
#endif

        return true;
    }

    /// Resolves the persistent diagnostic color assigned to one failed CARD initialization operation.
    GXColor GameCubeApplication::GetMemoryCardDiagnosticFailureColor(GameCubeMemoryCardDiagnosticFailure failure, s32 resultCode) const {
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
        if (failure == GameCubeMemoryCardDiagnosticFailure::CardMount) {
            switch (resultCode) {
                case CARD_ERROR_BUSY:
                    return GXColor { 0xFF, 0x20, 0x20, 0xFF };
                case CARD_ERROR_WRONGDEVICE:
                    return GXColor { 0xFF, 0xFF, 0x20, 0xFF };
                case CARD_ERROR_NOCARD:
                    return GXColor { 0x20, 0x20, 0xFF, 0xFF };
                case CARD_ERROR_IOERROR:
                    return GXColor { 0xFF, 0xFF, 0xFF, 0xFF };
                case CARD_ERROR_BROKEN:
                    return GXColor { 0x20, 0xFF, 0xFF, 0xFF };
                case CARD_ERROR_ENCODING:
                    return GXColor { 0xA0, 0x20, 0xFF, 0xFF };
                case CARD_ERROR_CANCELED:
                    return GXColor { 0xFF, 0x60, 0x20, 0xFF };
                case CARD_ERROR_FATAL_ERROR:
                    return GXColor { 0xFF, 0xFF, 0x80, 0xFF };
                default:
                    return GXColor { 0xFF, 0x20, 0xFF, 0xFF };
            }
        }

        switch (failure) {
            case GameCubeMemoryCardDiagnosticFailure::CardInit:
                return GXColor { 0xFF, 0x20, 0x20, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardProbe:
                return GXColor { 0x20, 0x20, 0xFF, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardMount:
                return GXColor { 0xFF, 0x20, 0xFF, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardSectorSize:
                return GXColor { 0xFF, 0xFF, 0x20, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardOpen:
                return GXColor { 0x20, 0xFF, 0xFF, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardCreate:
                return GXColor { 0xFF, 0x60, 0x20, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardRead:
                return GXColor { 0xA0, 0x20, 0xFF, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::CardWrite:
                return GXColor { 0x20, 0xFF, 0x80, 0xFF };
            case GameCubeMemoryCardDiagnosticFailure::None:
                return GXColor { 0xFF, 0xFF, 0xFF, 0xFF };
        }
#else
        (void)failure;
        (void)resultCode;
#endif

        return GXColor { 0xFF, 0xFF, 0xFF, 0xFF };
    }

    /// Verifies that Nintendont's temporary DOL-entry trampoline completed before the generated game entry began executing.
    bool GameCubeApplication::VerifyNintendontHandoff() {
#if HELENGINE_GAMECUBE_NINTENDONT_HANDOFF_DIAGNOSTIC
        const volatile uint32_t* const handoffStatusAddress = reinterpret_cast<const volatile uint32_t*>(NintendontHandoffStatusAddress);
        const uint32_t handoffStatus = *handoffStatusAddress;
        if (handoffStatus == NintendontTrampolinePendingStatus) {
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0xFF, 0x20, 0x20, 0xFF });
            PresentBootFrame();
            return false;
        } else if (handoffStatus < NintendontGameEntryMinimum || handoffStatus >= NintendontGameEntryMaximumExclusive) {
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0xFF, 0x20, 0xFF, 0xFF });
            PresentBootFrame();
            return false;
        }

        SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0xFF, 0x80, 0x20, 0xFF });
        PresentBootFrame();
#endif

        return true;
    }

    /// Initializes the DVD interface used by packaged-disc boots before any content reads occur.
    bool GameCubeApplication::InitializePackagedDisc() {
        return GameCubeDiscReader::Initialize();
    }

    /// Reads one byte range from the mounted GameCube disc using aligned sector transfers.
    bool GameCubeApplication::ReadDiscRange(void* destination, std::size_t offset, std::size_t length) {
        if (destination == nullptr) {
            return false;
        } else if (length == 0U) {
            return true;
        }

        const std::size_t firstSectorIndex = offset / DiscSectorSize;
        const std::size_t firstSectorByteOffset = offset % DiscSectorSize;
        const std::size_t lastByteOffsetExclusive = offset + length;
        const std::size_t lastSectorIndex = (lastByteOffsetExclusive + (DiscSectorSize - 1U)) / DiscSectorSize;
        const std::size_t sectorCount = lastSectorIndex - firstSectorIndex;
        const std::size_t sectorBufferLength = sectorCount * DiscSectorSize;
        const std::size_t alignedSectorBufferLength = (sectorBufferLength + 31U) & ~static_cast<std::size_t>(31U);
        uint8_t* sectorBuffer = static_cast<uint8_t*>(memalign(32, alignedSectorBufferLength));
        if (sectorBuffer == nullptr) {
            return false;
        }

        const bool readSucceeded = GameCubeDiscReader::ReadBytes(sectorBuffer, firstSectorIndex * DiscSectorSize, sectorCount * DiscSectorSize);
        if (readSucceeded) {
            std::memcpy(destination, sectorBuffer + firstSectorByteOffset, length);
        }

        free(sectorBuffer);
        return readSucceeded;
    }

    /// Validates the mounted GameCube retail disc header and packaged FST header fields.
    bool GameCubeApplication::ValidatePackagedDiscLayout() {
        alignas(32) uint8_t discHeader[DiscHeaderReadLength];
        std::memset(discHeader, 0, sizeof(discHeader));
        if (!ReadDiscRange(discHeader, 0U, sizeof(discHeader))) {
            SYS_Report("[GC] Failed to read the packaged disc header.\n");
            return false;
        }

        const uint32_t discMagic = ReadBigEndianU32(discHeader + 0x1C);
        const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424);
        const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428);
        SYS_Report(
            "[GC] Packaged disc probe magic=%08lX fstOffset=%08lX fstSize=%08lX\n",
            static_cast<unsigned long>(discMagic),
            static_cast<unsigned long>(fstOffset),
            static_cast<unsigned long>(fstSize));
        if (discMagic != DiscMagic) {
            SYS_Report("[GC] Packaged disc probe failed: unexpected disc magic.\n");
            return false;
        }

        if (fstOffset == 0U || fstSize < 12U) {
            SYS_Report("[GC] Packaged disc probe failed: invalid FST header fields.\n");
            return false;
        }

        return true;
    }

    /// Reads the packaged FST body and verifies it contains the expected startup-scene asset entry.
    bool GameCubeApplication::ValidatePackagedDiscFileSystemTable() {
        alignas(32) uint8_t discHeader[DiscHeaderReadLength];
        std::memset(discHeader, 0, sizeof(discHeader));
        if (!ReadDiscRange(discHeader, 0U, sizeof(discHeader))) {
            SYS_Report("[GC] Failed to reread the packaged disc header for FST validation.\n");
            return false;
        }

        const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424);
        const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428);
        if (fstOffset == 0U || fstSize < FstEntrySize) {
            SYS_Report("[GC] Packaged FST validation failed: invalid header fields.\n");
            return false;
        }

        const std::size_t alignedFstSize = (static_cast<std::size_t>(fstSize) + 31U) & ~static_cast<std::size_t>(31U);
        uint8_t* fstBytes = static_cast<uint8_t*>(memalign(32, alignedFstSize));
        if (fstBytes == nullptr) {
            SYS_Report("[GC] Packaged FST validation failed: could not allocate FST buffer.\n");
            return false;
        }

        std::memset(fstBytes, 0, alignedFstSize);
        if (!ReadDiscRange(fstBytes, fstOffset, fstSize)) {
            free(fstBytes);
            SYS_Report("[GC] Packaged FST validation failed: could not read FST bytes.\n");
            return false;
        }

        const uint32_t rootEntryCount = ReadBigEndianU32(fstBytes + 8);
        const char expectedAssetName[] = "DemoDiscMainMenu.hasset";
        const uint8_t* searchBegin = fstBytes;
        const uint8_t* searchEnd = fstBytes + fstSize;
        const bool foundExpectedAsset = std::search(
            searchBegin,
            searchEnd,
            expectedAssetName,
            expectedAssetName + (sizeof(expectedAssetName) - 1U)) != searchEnd;

        SYS_Report(
            "[GC] Packaged FST probe rootEntryCount=%08lX foundDemoDiscMainMenu=%d\n",
            static_cast<unsigned long>(rootEntryCount),
            foundExpectedAsset ? 1 : 0);

        free(fstBytes);
        return foundExpectedAsset;
    }

    /// Reads one packaged scene asset from disc after resolving its file entry from the FST.
    bool GameCubeApplication::ValidatePackagedSceneAssetRead() {
        alignas(32) uint8_t discHeader[DiscHeaderReadLength];
        std::memset(discHeader, 0, sizeof(discHeader));
        if (!ReadDiscRange(discHeader, 0U, sizeof(discHeader))) {
            SYS_Report("[GC] Packaged asset probe failed: could not reread disc header.\n");
            return false;
        }

        const uint32_t fstOffset = ReadBigEndianU32(discHeader + 0x424);
        const uint32_t fstSize = ReadBigEndianU32(discHeader + 0x428);
        if (fstOffset == 0U || fstSize < FstEntrySize) {
            SYS_Report("[GC] Packaged asset probe failed: invalid FST header fields.\n");
            return false;
        }

        const std::size_t alignedFstSize = (static_cast<std::size_t>(fstSize) + 31U) & ~static_cast<std::size_t>(31U);
        uint8_t* fstBuffer = static_cast<uint8_t*>(memalign(32, alignedFstSize));
        if (fstBuffer == nullptr) {
            SYS_Report("[GC] Packaged asset probe failed: could not allocate FST buffer.\n");
            return false;
        }

        std::memset(fstBuffer, 0, alignedFstSize);
        if (!ReadDiscRange(fstBuffer, fstOffset, fstSize)) {
            free(fstBuffer);
            SYS_Report("[GC] Packaged asset probe failed: could not read FST bytes.\n");
            return false;
        }

        std::vector<uint8_t> fstBytes(fstBuffer, fstBuffer + fstSize);
        free(fstBuffer);

        std::size_t discOffset = 0U;
        std::size_t fileSize = 0U;
        const std::string expectedPath = "dvd:/cooked/scenes/DemoDiscMainMenu.hasset";
        if (!TryResolvePackagedFstFile(fstBytes, 0U, "dvd:/", expectedPath, discOffset, fileSize)) {
            SYS_Report("[GC] Packaged asset probe failed: expected file was not resolved from the FST.\n");
            return false;
        }

        const std::size_t previewLength = fileSize < 16U ? fileSize : 16U;
        std::vector<uint8_t> previewBytes(previewLength, 0U);
        if (previewLength > 0U && !ReadDiscRange(previewBytes.data(), discOffset, previewLength)) {
            SYS_Report("[GC] Packaged asset probe failed: could not read preview bytes.\n");
            return false;
        }

        SYS_Report(
            "[GC] Packaged asset probe path=%s offset=%08lX size=%08lX head=%02X %02X %02X %02X\n",
            expectedPath.c_str(),
            static_cast<unsigned long>(discOffset),
            static_cast<unsigned long>(fileSize),
            previewLength > 0U ? static_cast<unsigned int>(previewBytes[0]) : 0U,
            previewLength > 1U ? static_cast<unsigned int>(previewBytes[1]) : 0U,
            previewLength > 2U ? static_cast<unsigned int>(previewBytes[2]) : 0U,
            previewLength > 3U ? static_cast<unsigned int>(previewBytes[3]) : 0U);
        return fileSize > 0U;
    }

    /// Reads the UTF-8 name stored for one FST entry.
    std::string GameCubeApplication::ReadPackagedFstEntryName(const std::vector<uint8_t>& fstBytes, std::size_t entryIndex) {
        const std::size_t entryCount = ReadBigEndianU32(fstBytes.data() + 8);
        const std::size_t stringTableOffset = entryCount * FstEntrySize;
        const std::size_t entryOffset = entryIndex * FstEntrySize;
        const uint32_t nameOffset = (static_cast<uint32_t>(fstBytes[entryOffset + 1]) << 16)
            | (static_cast<uint32_t>(fstBytes[entryOffset + 2]) << 8)
            | static_cast<uint32_t>(fstBytes[entryOffset + 3]);
        return std::string(reinterpret_cast<const char*>(fstBytes.data() + stringTableOffset + nameOffset));
    }

    /// Recursively resolves one packaged file path to its disc offset and byte length from the FST bytes.
    bool GameCubeApplication::TryResolvePackagedFstFile(
        const std::vector<uint8_t>& fstBytes,
        std::size_t directoryEntryIndex,
        const std::string& directoryPath,
        const std::string& expectedPath,
        std::size_t& discOffset,
        std::size_t& fileSize) {
        const std::size_t directoryOffset = directoryEntryIndex * FstEntrySize;
        const std::size_t directoryEndIndex = ReadBigEndianU32(fstBytes.data() + directoryOffset + 8);
        for (std::size_t entryIndex = directoryEntryIndex + 1U; entryIndex < directoryEndIndex; entryIndex++) {
            const std::size_t entryOffset = entryIndex * FstEntrySize;
            const bool isDirectory = fstBytes[entryOffset] != 0U;
            const std::string entryName = ReadPackagedFstEntryName(fstBytes, entryIndex);
            const std::string entryPath = directoryPath == "dvd:/"
                ? directoryPath + entryName
                : directoryPath + "/" + entryName;

            if (isDirectory) {
                const uint32_t parentDirectoryIndex = ReadBigEndianU32(fstBytes.data() + entryOffset + 4);
                if (parentDirectoryIndex == directoryEntryIndex) {
                    if (TryResolvePackagedFstFile(fstBytes, entryIndex, entryPath, expectedPath, discOffset, fileSize)) {
                        return true;
                    }

                    entryIndex = ReadBigEndianU32(fstBytes.data() + entryOffset + 8) - 1U;
                }

                continue;
            }

            if (entryPath != expectedPath) {
                continue;
            }

            discOffset = ReadBigEndianU32(fstBytes.data() + entryOffset + 4);
            fileSize = ReadBigEndianU32(fstBytes.data() + entryOffset + 8);
            return true;
        }

        return false;
    }

    /// Initializes the generated engine core when generated sources are present in the build.
    bool GameCubeApplication::InitializeEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        const char* initializationStage = "BeforeCoreConstruction";
        try {
            initializationStage = "ConstructCore";
            SetBootPhase(GameCubeBootPhase::CoreConstruction, GXColor { 0xFF, 0xFF, 0x00, 0xFF });
            PresentBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA001U);
#endif
            EngineCore = new Core();

            initializationStage = "ReadInitializationOptions";
            SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x80, 0x00, 0xFF });
            PresentBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA002U);
#endif
            CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
            initializationStage = "ReadInitializationOptionsCompleted";
            SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0x80, 0xFF, 0x00, 0xFF });
            PresentBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA003U);
#endif
            if (options == nullptr) {
                FailBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                return false;
            }

            initializationStage = "ConfigureSceneBootstrap";
            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0x40, 0x80, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            initializationStage = "InitializePackagedDisc";
            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0xFF, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA004U);
#endif
            if (!GameCubeSceneBootstrap::InitializePackagedDiscInterface()) {
                FailBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                return false;
            }
            initializationStage = "VerifyPackagedDiscReadiness";
            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0xFF, 0x00, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA005U);
#endif
            if (!GameCubeSceneBootstrap::VerifyPackagedDiscReadiness()) {
                FailBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                SYS_Report("[GC] Packaged disc readiness validation failed.\n");
                return false;
            }
            initializationStage = "PackagedDiscReadinessVerified";
            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0xFF, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA006U);
#endif
            const std::string packagedContentRootPath = GameCubeSceneBootstrap::GetPackagedContentRootPath();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA007U);
#endif
            SYS_Report("[GC] Packaged content root: %s\n", packagedContentRootPath.c_str());
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA008U);
#endif
            options->ContentStreamSource = new HostFileSystemContentStreamSource(packagedContentRootPath);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA009U);
#endif
            options->SceneCatalog = GameCubeSceneBootstrap::CreatePackagedSceneCatalog();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA00AU);
#endif
            options->StandardPlatformInputConfiguration = GameCubeSceneBootstrap::CreatePackagedStandardPlatformInputConfiguration();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA00BU);
#endif
            const std::string packagedStartupSceneId = GameCubeSceneBootstrap::GetPackagedStartupSceneId();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA00CU);
#endif
            SYS_Report("[GC] Packaged startup scene id: %s\n", packagedStartupSceneId.c_str());
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA00DU);
#endif
            SYS_Report("[GC] Runtime build stamp: %s\n", BuildStamp);
#else
            options->ContentStreamSource = new HostFileSystemContentStreamSource(".");
            options->SceneCatalog = nullptr;
#endif
            options->UpdateOrderLayers = 4;
            options->RenderOrderLayers3D = 4;
            options->UpdateListInitialCapacity = 64;
            options->RenderList2DInitialCapacity = 64;
            options->RenderList3DInitialCapacity = 64;
#if HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC
            options->CommitPendingSceneOperationsDuringDraw = false;
#endif
#if HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC || HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            SceneTransitionTraceDiagnostics = new GameCubeSceneTransitionTraceDiagnostics(this);
            options->set_RuntimeDiagnosticsProvider(SceneTransitionTraceDiagnostics);
#endif

            initializationStage = "ConstructRenderManager3D";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x00, 0x00, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA00EU);
#endif
            EngineRenderManager3D = new GameCubeRenderManager3D();

            initializationStage = "ConstructRenderManager2D";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA00FU);
#endif
            EngineRenderManager2D = new GameCubeRenderManager2D();

            initializationStage = "ConnectRenderManagers";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0xFF, 0x00, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA010U);
#endif
            EngineRenderManager3D->SetOverlayRenderManager2D(EngineRenderManager2D);

            initializationStage = "InitializePlatformInput";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0x80, 0x00, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA011U);
#endif
            GameCubeInputManager::InitializePlatformInput(
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
                MemoryCardDiagnosticJournal
#else
                nullptr
#endif
            );

            initializationStage = "ConstructInputManager";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x80, 0x00, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA012U);
#endif
            EngineInputManager = new GameCubeInputManager(
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
                MemoryCardDiagnosticJournal
#else
                nullptr
#endif
            );

            initializationStage = "ConstructAudioBackend";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA013U);
#endif
            EngineAudioBackend = new GameCubeAudioBackend();

            initializationStage = "ConstructPlatformInfo";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0xFF, 0xFF, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA014U);
#endif
            EnginePlatformInfo = new PlatformInfo("gamecube", "1.0.0");

            initializationStage = "AddPrimaryWindow";
            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0x00, 0x40, 0xFF, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA015U);
#endif
            EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);

            initializationStage = "SetPresentedFrameSize";
            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x80, 0x00, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA016U);
#endif
            EngineRenderManager3D->SetPresentedFrameSize(static_cast<uint16_t>(RenderMode->fbWidth), static_cast<uint16_t>(RenderMode->efbHeight));

            initializationStage = "InitializeCore";
            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA017U);
#endif
            EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, EnginePlatformInfo, options);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA018U);
#endif
            EngineCore->SetAudioBackend(EngineAudioBackend);
            initializationStage = "InitializeCoreCompleted";
            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA019U);
#endif
            PresentVideoBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA01AU);
#endif
            SYS_Report("[GC] Engine core initialized.\n");
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA01BU);
#endif
#if HELENGINE_GAMECUBE_HAS_GENERATED_RUNTIME_MODULE_REGISTRATION && HELENGINE_GAMECUBE_GENERATED_RUNTIME_MODULE_REGISTRATION_ENABLED
            initializationStage = "RegisterGeneratedRuntimeModules";
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA01CU);
#endif
            RegisterGeneratedRuntimeModules(EngineCore);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA01DU);
#endif
            SYS_Report("[GC] Generated runtime modules registered.\n");
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA01EU);
#endif
#endif
        }
        catch (const std::exception& exception) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            SYS_Report(
                "[GC] Engine core initialization threw std::exception stage=%s message=%s\n",
                initializationStage,
                exception.what());
            return false;
        }
        catch (Exception* exception) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            const char* exceptionMessage = exception != nullptr ? exception->what() : "<null>";
            SYS_Report(
                "[GC] Engine core initialization threw Exception stage=%s message=%s\n",
                initializationStage,
                exceptionMessage);
            return false;
        }
        catch (...) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            SYS_Report("[GC] Engine core initialization threw stage=%s.\n", initializationStage);
            return false;
        }

        try {
            SetBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0x40, 0x40, 0xFF, 0xFF });
            PresentBootFrame();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA01FU);
#endif
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (EngineCore->get_SceneManager() == nullptr) {
                throw std::runtime_error("Packaged GameCube boot requires a runtime scene manager.");
            }

            const std::string runtimeTestSceneOverride = GetRuntimeTestSceneOverride();
            if (runtimeTestSceneOverride == "slope") {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xA020U);
#endif
                GameCubeCubeTestSceneInstaller::InstallSlopeScene();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xA021U);
#endif
                SYS_Report("[GC] Runtime slope test scene installed.\n");
            } else if (!runtimeTestSceneOverride.empty()) {
                throw std::runtime_error(std::string("Unsupported GameCube runtime test scene override: ") + runtimeTestSceneOverride);
            } else {
                const std::string packagedStartupSceneId = GameCubeSceneBootstrap::GetPackagedStartupSceneId();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xA022U);
#endif
                EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xA023U);
#endif
                SYS_Report("[GC] Packaged runtime startup scene queued.\n");
            }
#else
            const std::string runtimeTestSceneOverride = GetRuntimeTestSceneOverride();
            if (runtimeTestSceneOverride == "slope") {
                GameCubeCubeTestSceneInstaller::InstallSlopeScene();
                SYS_Report("[GC] Runtime slope test scene installed.\n");
            } else if (!runtimeTestSceneOverride.empty()) {
                throw std::runtime_error(std::string("Unsupported GameCube runtime test scene override: ") + runtimeTestSceneOverride);
            } else {
                GameCubeCubeTestSceneInstaller::Install();
                SYS_Report("[GC] Runtime cube-test scene installed.\n");
            }
#endif
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xA024U);
#endif
            EngineInitialized = true;
            PresentedFrameCount = 0;
            PreviousFrameTicks = 0U;
            LastElapsedFrameSeconds = 0.0;
            VerifiedFrameCount = 0;
            UpdateCompletedSincePresent = false;
            DrawCompletedSincePresent = false;
            FirstUpdateBeginReported = false;
            FirstUpdateCompletedReported = false;
            FirstDrawBeginReported = false;
            FirstDrawCompletedReported = false;
            SetBootPhase(GameCubeBootPhase::Running, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            return true;
        }
        catch (const std::exception& exception) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0xFF, 0x40, 0x80, 0xFF });
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (EngineCore != nullptr && EngineCore->get_SceneManager() != nullptr) {
                SYS_Report(
                    "[GC] SceneManager trace stage=%s scene=%s loaded=%ld pending=%ld\n",
                    EngineCore->get_SceneManager()->get_LastTraceStage().c_str(),
                    EngineCore->get_SceneManager()->get_LastTraceSceneId().c_str(),
                    static_cast<long>(EngineCore->get_SceneManager()->get_LastTraceLoadedSceneCount()),
                    static_cast<long>(EngineCore->get_SceneManager()->get_LastTracePendingOperationCount()));
            }

            if (EngineCore != nullptr && EngineCore->get_SceneLoadService() != nullptr) {
                SYS_Report(
                    "[GC] SceneLoad trace stage=%s root=%ld depth=%ld component=%s textStage=%s textFont=%s fontStage=%s\n",
                    EngineCore->get_SceneLoadService()->get_LastTraceStage().c_str(),
                    static_cast<long>(EngineCore->get_SceneLoadService()->get_LastTraceRootEntityIndex()),
                    static_cast<long>(EngineCore->get_SceneLoadService()->get_LastTraceEntityDepth()),
                    EngineCore->get_SceneLoadService()->get_LastTraceComponentTypeId().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastTextLoadStage().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastTextFontRelativePath().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastFontDeserializeStage().c_str());
            }
#endif
            SYS_Report("[GC] Runtime cube-test scene installation threw std::exception: %s\n", exception.what());
            return false;
        }
        catch (Exception* exception) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0xFF, 0x40, 0x80, 0xFF });
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (EngineCore != nullptr && EngineCore->get_SceneManager() != nullptr) {
                SYS_Report(
                    "[GC] SceneManager trace stage=%s scene=%s loaded=%ld pending=%ld\n",
                    EngineCore->get_SceneManager()->get_LastTraceStage().c_str(),
                    EngineCore->get_SceneManager()->get_LastTraceSceneId().c_str(),
                    static_cast<long>(EngineCore->get_SceneManager()->get_LastTraceLoadedSceneCount()),
                    static_cast<long>(EngineCore->get_SceneManager()->get_LastTracePendingOperationCount()));
            }

            if (EngineCore != nullptr && EngineCore->get_SceneLoadService() != nullptr) {
                SYS_Report(
                    "[GC] SceneLoad trace stage=%s root=%ld depth=%ld component=%s textStage=%s textFont=%s fontStage=%s\n",
                    EngineCore->get_SceneLoadService()->get_LastTraceStage().c_str(),
                    static_cast<long>(EngineCore->get_SceneLoadService()->get_LastTraceRootEntityIndex()),
                    static_cast<long>(EngineCore->get_SceneLoadService()->get_LastTraceEntityDepth()),
                    EngineCore->get_SceneLoadService()->get_LastTraceComponentTypeId().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastTextLoadStage().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastTextFontRelativePath().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastFontDeserializeStage().c_str());
            }
#endif
            SYS_Report("[GC] Runtime cube-test scene installation threw Exception*: %s\n", exception->what());
            delete exception;
            return false;
        }
        catch (...) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0xFF, 0x40, 0x80, 0xFF });
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (EngineCore != nullptr && EngineCore->get_SceneManager() != nullptr) {
                SYS_Report(
                    "[GC] SceneManager trace stage=%s scene=%s loaded=%ld pending=%ld\n",
                    EngineCore->get_SceneManager()->get_LastTraceStage().c_str(),
                    EngineCore->get_SceneManager()->get_LastTraceSceneId().c_str(),
                    static_cast<long>(EngineCore->get_SceneManager()->get_LastTraceLoadedSceneCount()),
                    static_cast<long>(EngineCore->get_SceneManager()->get_LastTracePendingOperationCount()));
            }

            if (EngineCore != nullptr && EngineCore->get_SceneLoadService() != nullptr) {
                SYS_Report(
                    "[GC] SceneLoad trace stage=%s root=%ld depth=%ld component=%s textStage=%s textFont=%s fontStage=%s\n",
                    EngineCore->get_SceneLoadService()->get_LastTraceStage().c_str(),
                    static_cast<long>(EngineCore->get_SceneLoadService()->get_LastTraceRootEntityIndex()),
                    static_cast<long>(EngineCore->get_SceneLoadService()->get_LastTraceEntityDepth()),
                    EngineCore->get_SceneLoadService()->get_LastTraceComponentTypeId().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastTextLoadStage().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastTextFontRelativePath().c_str(),
                    EngineCore->get_SceneLoadService()->get_LastFontDeserializeStage().c_str());
            }
#endif
            SYS_Report("[GC] Runtime cube-test scene installation threw.\n");
            return false;
        }
#endif

        return true;
    }

    /// Advances one engine frame when the generated core was initialized successfully.
    bool GameCubeApplication::UpdateEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!EngineInitialized || EngineCore == nullptr || EngineRenderManager2D == nullptr) {
            FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            return false;
        }

        try {
            SetBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0x00, 0xA0, 0x00, 0xFF });
            if (!FirstUpdateBeginReported) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xB001U);
#endif
                SYS_Report("[GC] First update begin.\n");
                FirstUpdateBeginReported = true;
            }
            PresentFirstFrameTraceCheckpoint(GXColor { 0xA0, 0x20, 0xFF, 0xFF });
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xB002U);
#endif
            EngineRenderManager2D->BeginFrame();
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
            if (!HasRecordedCoreUpdate && MemoryCardDiagnosticJournal != nullptr) {
                MemoryCardDiagnosticJournal->Record(GameCubeMemoryCardDiagnosticStage::CoreUpdateBegin, 0);
            }
#endif
            PresentFirstFrameTraceCheckpoint(GXColor { 0x20, 0xE0, 0xFF, 0xFF });
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xB003U);
#endif
            const double elapsedSeconds = MeasureElapsedFrameSeconds();
            EngineCore->Update(elapsedSeconds);
            LastElapsedFrameSeconds = elapsedSeconds;
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
            if (!HasRecordedCoreUpdate && MemoryCardDiagnosticJournal != nullptr) {
                MemoryCardDiagnosticJournal->Record(GameCubeMemoryCardDiagnosticStage::CoreUpdateComplete, 0);
                HasRecordedCoreUpdate = true;
            }
#endif
            if (EngineRenderManager2D != nullptr) {
                PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0xE0, 0x20, 0xFF });
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xB004U);
#endif
                EngineRenderManager2D->FlushReleasedTextures();
            }
            if (EngineRenderManager3D != nullptr) {
                PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0x80, 0x20, 0xFF });
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xB005U);
#endif
                EngineRenderManager3D->FlushReleasedAssets();
            }
            UpdateCompletedSincePresent = true;
            if (!FirstUpdateCompletedReported) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xB006U);
#endif
                SYS_Report("[GC] First update completed.\n");
                FirstUpdateCompletedReported = true;
            }
            return true;
        }
        catch (Exception* exception) {
            GameCubeClearSceneLoadRequest();
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report(
                "[GC] Engine update threw Exception*: %s\n",
                exception != nullptr ? exception->what() : "<null>");
            return false;
        }
        catch (const std::exception& exception) {
            GameCubeClearSceneLoadRequest();
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report("[GC] Engine update threw std::exception: %s\n", exception.what());
            return false;
        }
        catch (...) {
            GameCubeClearSceneLoadRequest();
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report("[GC] Engine update threw.\n");
            return false;
        }
#endif

        return true;
    }

    /// Measures elapsed frame time with libogc's monotonic GameCube timebase.
    double GameCubeApplication::MeasureElapsedFrameSeconds() {
        const u64 currentFrameTicks = gettime();
        if (PreviousFrameTicks == 0U) {
            PreviousFrameTicks = currentFrameTicks;
            return 0.0;
        }

        const double elapsedSeconds = ticks_to_millisecs(currentFrameTicks - PreviousFrameTicks) / 1000.0;
        PreviousFrameTicks = currentFrameTicks;
        return elapsedSeconds;
    }

    /// Reports the GameCube host clock and scene state at a low fixed cadence when direct diagnostics are enabled.
    void GameCubeApplication::ReportRuntimeFrameTelemetry(double elapsedSeconds) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC && HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if ((PresentedFrameCount % 60U) != 0U || EngineCore == nullptr) {
            return;
        }

        SceneManager* sceneManager = EngineCore->get_SceneManager();
        List<LoadedSceneRecord*>* loadedScenes = sceneManager != nullptr
            ? sceneManager->get_LoadedScenes()
            : nullptr;
        const int32_t loadedSceneCount = loadedScenes != nullptr
            ? loadedScenes->get_Count()
            : -1;
        const std::string firstSceneId = loadedScenes != nullptr
                && loadedScenes->get_Count() > 0
                && (*loadedScenes)[0] != nullptr
            ? (*loadedScenes)[0]->get_SceneId()
            : "<none>";
        const std::string secondSceneId = loadedScenes != nullptr
                && loadedScenes->get_Count() > 1
                && (*loadedScenes)[1] != nullptr
            ? (*loadedScenes)[1]->get_SceneId()
            : "<none>";
        const int32_t sceneTransitionActive = sceneManager != nullptr && sceneManager->get_IsSceneTransitionActive()
            ? 1
            : 0;
        SYS_Report(
            "[GC] Runtime telemetry frame=%lu hostMs=%.3f coreMs=%.3f scenes=%ld transition=%ld scene0=%s scene1=%s\n",
            static_cast<unsigned long>(PresentedFrameCount),
            elapsedSeconds * 1000.0,
            EngineCore->get_FrameDeltaSeconds() * 1000.0,
            static_cast<long>(loadedSceneCount),
            static_cast<long>(sceneTransitionActive),
            firstSceneId.c_str(),
            secondSceneId.c_str());

        const std::vector<GameCubeSpriteDrawCommand>& spriteQueue = EngineRenderManager2D->GetSpriteQueue();
        const std::vector<GameCubeTextDrawCommand>& textQueue = EngineRenderManager2D->GetTextQueue();
        const std::vector<GameCubeRoundedRectDrawCommand>& roundedRectQueue = EngineRenderManager2D->GetRoundedRectQueue();
        SYS_Report(
            "[GC] 2D queue counts sprites=%lu text=%lu rectangles=%lu\n",
            static_cast<unsigned long>(spriteQueue.size()),
            static_cast<unsigned long>(textQueue.size()),
            static_cast<unsigned long>(roundedRectQueue.size()));

        GXColor centerColor {};
        GXColor markerColor {};
        GXColor backgroundColor {};
        GX_DrawDone();
        GX_PeekARGB(ProbeCenterSampleX, ProbeCenterSampleY, &centerColor);
        GX_PeekARGB(ProbeMarkerSampleX, ProbeMarkerSampleY, &markerColor);
        GX_PeekARGB(ProbeBackgroundSampleX, ProbeBackgroundSampleY, &backgroundColor);
        SYS_Report(
            "[GC] EFB pixels frame=%lu center=(%02X,%02X,%02X,%02X) marker=(%02X,%02X,%02X,%02X) background=(%02X,%02X,%02X,%02X)\n",
            static_cast<unsigned long>(PresentedFrameCount),
            centerColor.r,
            centerColor.g,
            centerColor.b,
            centerColor.a,
            markerColor.r,
            markerColor.g,
            markerColor.b,
            markerColor.a,
            backgroundColor.r,
            backgroundColor.g,
            backgroundColor.b,
            backgroundColor.a);

        constexpr std::size_t RectangleTelemetryEdgeCount = 3U;
        for (std::size_t rectangleIndex = 0; rectangleIndex < roundedRectQueue.size(); rectangleIndex++) {
            if (rectangleIndex >= RectangleTelemetryEdgeCount
                && rectangleIndex + RectangleTelemetryEdgeCount < roundedRectQueue.size()) {
                continue;
            }

            const GameCubeRoundedRectDrawCommand& rectangleCommand = roundedRectQueue[rectangleIndex];
            if (rectangleCommand.Drawable == nullptr) {
                continue;
            }

            const auto fillColor = rectangleCommand.Drawable->get_FillColor();
            const int32_t cameraDrawOrder = rectangleCommand.Camera != nullptr
                ? static_cast<int32_t>(rectangleCommand.Camera->get_CameraDrawOrder())
                : -1;
            SYS_Report(
                "[GC] 2D rectangle index=%lu cameraOrder=%ld rgba=(%u,%u,%u,%u)\n",
                static_cast<unsigned long>(rectangleIndex),
                static_cast<long>(cameraDrawOrder),
                static_cast<unsigned int>(fillColor.X),
                static_cast<unsigned int>(fillColor.Y),
                static_cast<unsigned int>(fillColor.Z),
                static_cast<unsigned int>(fillColor.W));
        }
#else
        static_cast<void>(elapsedSeconds);
#endif
    }

    /// Draws one engine frame when the generated core was initialized successfully.
    bool GameCubeApplication::DrawEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!EngineInitialized || EngineCore == nullptr || EngineRenderManager3D == nullptr || EngineRenderManager2D == nullptr) {
            FailBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            return false;
        }

        try {
            SetBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0x00, 0x60, 0x00, 0xFF });
            if (!FirstDrawBeginReported) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xB007U);
#endif
                SYS_Report("[GC] First draw begin.\n");
                FirstDrawBeginReported = true;
            }
            PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0x20, 0xC0, 0xFF });
#if HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC
            PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0x00, 0xC0, 0xFF });
            auto* loadedScenes = EngineCore->get_SceneManager()->get_LoadedScenes();
            loadedScenes->set_Capacity(loadedScenes->get_Count() + 1);
#if HELENGINE_GAMECUBE_EXCEPTION_SCREEN_DIAGNOSTIC
            GameCubeExceptionDiagnostics::CaptureSceneListState(
                loadedScenes->get_Count(),
                loadedScenes->get_Capacity(),
                loadedScenes->data());
#endif
            PresentFirstFrameTraceCheckpoint(GXColor { 0x00, 0xFF, 0xFF, 0xFF });
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xB008U);
#endif
            EngineCore->CompleteFrameBoundary();
            PresentFirstFrameTraceCheckpoint(GXColor { 0x20, 0x60, 0xFF, 0xFF });
#endif
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
            if (!HasRecordedCoreDraw && MemoryCardDiagnosticJournal != nullptr) {
                MemoryCardDiagnosticJournal->Record(GameCubeMemoryCardDiagnosticStage::CoreDrawBegin, 0);
            }
#endif
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
            DisplayDirectFrameDiagnosticCode(0xB009U);
#endif
            EngineCore->Draw();
            ReportRuntimeFrameTelemetry(LastElapsedFrameSeconds);
#if HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL
            if (!HasRecordedCoreDraw && MemoryCardDiagnosticJournal != nullptr) {
                MemoryCardDiagnosticJournal->Record(GameCubeMemoryCardDiagnosticStage::CoreDrawComplete, 0);
                HasRecordedCoreDraw = true;
            }
#endif
            DrawCompletedSincePresent = true;
            if (!FirstDrawCompletedReported) {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
                DisplayDirectFrameDiagnosticCode(0xB00AU);
#endif
                SYS_Report("[GC] First draw completed.\n");
                FirstDrawCompletedReported = true;
            }
            ReportCompletedSceneLoadIfPending();
            return true;
        }
        catch (Exception* exception) {
            GameCubeClearSceneLoadRequest();
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report(
                "[GC] Engine draw threw Exception*: %s\n",
                exception != nullptr ? exception->what() : "<null>");
            return false;
        }
        catch (const std::exception& exception) {
            GameCubeClearSceneLoadRequest();
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report("[GC] Engine draw threw std::exception: %s\n", exception.what());
            return false;
        }
        catch (...) {
            GameCubeClearSceneLoadRequest();
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report("[GC] Engine draw threw.\n");
            return false;
        }
#endif

        return true;
    }

    /// Presents one fallback frame to the active framebuffer.
    void GameCubeApplication::PresentFrame() {
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        AdvanceDirectFrameDiagnosticHeartbeat();
#endif
        PresentFirstFrameTraceCheckpoint(GXColor { 0x20, 0xA0, 0xFF, 0xFF });
        const GXColor visibleColor = ResolvePresentedClearColor();
        PresentedFrameCount++;
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
#if HELENGINE_GAMECUBE_MINIMAL_SAMPLE
#else
        if (EngineRenderManager3D == nullptr || (!EngineRenderManager3D->HasRenderedScene() && (EngineRenderManager2D == nullptr || !EngineRenderManager2D->HasCapturedDrawables()))) {
            GX_SetCopyClear(visibleColor, 0x00FFFFFF);
        }
#endif
#else
        GX_SetCopyClear(visibleColor, 0x00FFFFFF);
#endif
#if HELENGINE_GAMECUBE_MINIMAL_SAMPLE && (HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT > 0)
        if (ShouldReportProbePixels(PresentedFrameCount)) {
            GXColor centerColor {};
            GXColor markerColor {};
            GXColor backgroundColor {};
            GX_DrawDone();
            PeekProbePixels(centerColor, markerColor, backgroundColor);
            ReportProbePixels(PresentedFrameCount, centerColor, markerColor, backgroundColor);

            const bool centerVisible = IsProbeRegionVisible(ProbeCenterSampleX, ProbeCenterSampleY, backgroundColor);
            const bool markerVisible = IsProbeRegionVisible(ProbeMarkerSampleX, ProbeMarkerSampleY, backgroundColor);
            VerificationCenterVisibleOnce = VerificationCenterVisibleOnce || centerVisible;
            VerificationMarkerVisibleOnce = VerificationMarkerVisibleOnce || markerVisible;

            if (VerificationCenterVisibleOnce && VerificationMarkerVisibleOnce && !centerVisible && !markerVisible) {
                VerificationMissingSampleCount++;
            } else {
                VerificationMissingSampleCount = 0U;
            }

            if (VerificationMissingSampleCount >= 2U) {
                VerificationProbeFailed = true;
                SYS_Report("[GC] Verification probe detected shared triangle disappearance.\n");
            }
        }
#endif
        FrameBufferIndex ^= 1U;
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_TRUE);
        GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);
        GX_DrawDone();
#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        WriteDirectFrameDiagnosticCode(DirectFrameDiagnosticCode);
#endif
#if HELENGINE_GAMECUBE_LOGO_ANIMATION_DIAGNOSTIC && !HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        if (LogoAnimationDiagnosticLatched) {
            WriteDirectFrameDiagnosticCode(LogoAnimationDiagnosticCode);
        }
#endif
        VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
#if HELENGINE_GAMECUBE_FIRST_FRAME_TRACE_DIAGNOSTIC
        PresentFirstFrameTraceCheckpoint(GXColor { 0xFF, 0xFF, 0xFF, 0xFF });
        FirstFrameTraceCompleted = true;
#endif
    }

    /// Resolves the currently visible diagnostic color for the next presented frame.
    GXColor GameCubeApplication::ResolvePresentedClearColor() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
#if HELENGINE_GAMECUBE_MINIMAL_SAMPLE
        return ClearColor;
#else
        if (EngineInitialized) {
            if ((EngineRenderManager3D != nullptr && EngineRenderManager3D->HasRenderedScene())
                || (EngineRenderManager2D != nullptr && EngineRenderManager2D->HasCapturedDrawables())) {
                UpdateCompletedSincePresent = false;
                DrawCompletedSincePresent = false;
                return ClearColor;
            }

            if ((EngineRenderManager3D != nullptr && !EngineRenderManager3D->HasRenderedScene())
                && (EngineRenderManager2D == nullptr || !EngineRenderManager2D->HasCapturedDrawables())) {
                return GXColor { 0x80, 0x00, 0x80, 0xFF };
            }

            if (UpdateCompletedSincePresent) {
                UpdateCompletedSincePresent = false;
                DrawCompletedSincePresent = false;
                return GXColor { 0xC0, 0xC0, 0x00, 0xFF };
            }

            if (DrawCompletedSincePresent) {
                UpdateCompletedSincePresent = false;
                DrawCompletedSincePresent = false;
                return GXColor { 0x00, 0x80, 0x80, 0xFF };
            }
        }
#endif
#endif

        return ClearColor;
    }

    /// Updates the currently presented clear color used for boot-state diagnostics.
    void GameCubeApplication::SetClearColor(GXColor color) {
        ClearColor = color;
    }

    /// Sets the current boot phase and visible clear color.
    void GameCubeApplication::SetBootPhase(GameCubeBootPhase phase, GXColor color) {
        BootPhase = phase;
        SetClearColor(color);
    }

    /// Marks the current boot phase as failed and updates the visible clear color.
    void GameCubeApplication::FailBootPhase(GameCubeBootPhase phase, GXColor color) {
        BootPhase = phase;
        SetClearColor(color);
    }

    /// Returns whether the current build was configured to stop after enough verified rendered frames.
    bool GameCubeApplication::HasSatisfiedVerificationExitCondition() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE && HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
        if (VerificationProbeFailed) {
            return true;
        }

        if (HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT > 0) {
            return PresentedFrameCount >= static_cast<uint32_t>(HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT);
        }
#endif

        return false;
    }

    /// Returns whether the current build should convert runtime success and failure into deterministic process exits.
    bool GameCubeApplication::IsVerificationBuild() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE && HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
        return HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT > 0;
#else
        return false;
#endif
    }

    /// Returns the process exit code that represents the current boot phase during verification runs.
    int GameCubeApplication::GetVerificationExitCode() {
        return 100 + static_cast<int>(BootPhase);
    }
}
