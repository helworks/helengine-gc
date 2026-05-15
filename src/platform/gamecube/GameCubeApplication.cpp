#include "platform/gamecube/GameCubeApplication.hpp"

#include <cstddef>
#include <cstdint>

#include <ogc/system.h>

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "SceneLoadMode.hpp"
#include "platform/gamecube/GameCubeCubeTestSceneInstaller.hpp"
#include "platform/gamecube/GameCubeInputManager.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"
#include "platform/gamecube/GameCubeRenderManager3D.hpp"
#include "platform/gamecube/GameCubeSceneBootstrap.hpp"
#endif

namespace helengine::gamecube {
    namespace {
        constexpr std::size_t DefaultFifoSize = 256 * 1024;
        alignas(32) uint8_t FifoBufferStorage[DefaultFifoSize];
    }

    /// Creates the GameCube application with no initialized native or engine state.
    GameCubeApplication::GameCubeApplication()
        : RenderMode(nullptr)
        , FrameBuffers { nullptr, nullptr }
        , FrameBufferIndex(0U)
        , ClearColor { 0x00, 0xFF, 0x00, 0xFF }
        , BootPhase(GameCubeBootPhase::NativeVideo)
        , EngineInitialized(false)
        , PresentedFrameCount(0)
        , VerifiedFrameCount(0)
        , UpdateCompletedSincePresent(false)
        , DrawCompletedSincePresent(false)
        , FirstUpdateBeginReported(false)
        , FirstUpdateCompletedReported(false)
        , FirstDrawBeginReported(false)
        , FirstDrawCompletedReported(false)
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        , EngineCore(nullptr)
        , EngineRenderManager3D(nullptr)
        , EngineRenderManager2D(nullptr)
        , EngineInputManager(nullptr)
#endif
    {
    }

    /// Releases generated-core bridge objects after the application loop finishes.
    GameCubeApplication::~GameCubeApplication() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        delete EngineCore;
        delete EngineInputManager;
        delete EngineRenderManager2D;
        delete EngineRenderManager3D;
#endif
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

        SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x00, 0x00, 0xFF, 0xFF });
        if (!InitializeGraphics()) {
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x80, 0x00, 0x80, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }

#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
        SetBootPhase(GameCubeBootPhase::Running, GXColor { 0xFF, 0x00, 0x00, 0xFF });
        while (true) {
            PresentFrame();
        }
#endif

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!InitializeEngineCore()) {
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
        }
#endif

        while (true) {
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
                return 0;
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
        VIDEO_WaitVSync();

        if (RenderMode->viTVMode & VI_NON_INTERLACE) {
            VIDEO_WaitVSync();
        }
        return true;
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
        GX_InvVtxCache();
        GX_InvalidateTexAll();
        GX_SetCopyClear(GXColor { 0x00, 0x00, 0x00, 0xFF }, 0x00FFFFFF);
        GX_CopyDisp(FrameBuffers[0], GX_TRUE);
        GX_CopyDisp(FrameBuffers[0], GX_TRUE);

        return true;
    }

    /// Initializes the generated engine core when generated sources are present in the build.
    bool GameCubeApplication::InitializeEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        try {
            SetBootPhase(GameCubeBootPhase::CoreConstruction, GXColor { 0xFF, 0xFF, 0x00, 0xFF });
            EngineCore = new Core();

            SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x80, 0x00, 0xFF });
            CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
            if (options == nullptr) {
                FailBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                return false;
            }

            SetBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0x00, 0x40, 0x80, 0xFF });
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (!GameCubeSceneBootstrap::InitializePackagedDisc()) {
                FailBootPhase(GameCubeBootPhase::SceneBootstrap, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                SYS_Report("[GC] Packaged DVD mount failed.\n");
                return false;
            }
            const std::string packagedContentRootPath = GameCubeSceneBootstrap::GetPackagedContentRootPath();
            SYS_Report("[GC] Packaged content root: %s\n", packagedContentRootPath.c_str());
            options->ContentRootPath = packagedContentRootPath;
            options->SceneCatalog = GameCubeSceneBootstrap::CreatePackagedSceneCatalog();
            const std::string packagedStartupSceneId = GameCubeSceneBootstrap::GetPackagedStartupSceneId();
            SYS_Report("[GC] Packaged startup scene id: %s\n", packagedStartupSceneId.c_str());
#else
            options->ContentRootPath = ".";
            options->SceneCatalog = nullptr;
#endif
            options->UpdateOrderLayers = 4;
            options->RenderOrderLayers3D = 4;
            options->UpdateListInitialCapacity = 64;
            options->RenderList2DInitialCapacity = 64;
            options->RenderList3DInitialCapacity = 64;

            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x00, 0xFF, 0xFF, 0xFF });
            EngineRenderManager3D = new GameCubeRenderManager3D();
            EngineRenderManager2D = new GameCubeRenderManager2D();
            EngineInputManager = new GameCubeInputManager();

            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0x00, 0x00, 0xFF, 0xFF });
            EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);
            EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, options);
            SYS_Report("[GC] Engine core initialized.\n");
        }
        catch (...) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
            SYS_Report("[GC] Engine core initialization threw.\n");
            return false;
        }

        try {
            SetBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0x40, 0x40, 0xFF, 0xFF });
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (EngineCore->get_SceneManager() == nullptr) {
                throw std::runtime_error("Packaged GameCube boot requires a runtime scene manager.");
            }

            const std::string packagedStartupSceneId = GameCubeSceneBootstrap::GetPackagedStartupSceneId();
            EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);
            SYS_Report("[GC] Packaged runtime startup scene queued.\n");
#else
            GameCubeCubeTestSceneInstaller::Install();
            SYS_Report("[GC] Runtime cube-test scene installed.\n");
#endif
            EngineInitialized = true;
            PresentedFrameCount = 0;
            VerifiedFrameCount = 0;
            UpdateCompletedSincePresent = false;
            DrawCompletedSincePresent = false;
            FirstUpdateBeginReported = false;
            FirstUpdateCompletedReported = false;
            FirstDrawBeginReported = false;
            FirstDrawCompletedReported = false;
            SetBootPhase(GameCubeBootPhase::Running, GXColor { 0x00, 0xFF, 0x00, 0xFF });
            return true;
        }
        catch (...) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::SceneLoad, GXColor { 0xFF, 0x40, 0x80, 0xFF });
            SYS_Report("[GC] Runtime cube-test scene installation threw.\n");
            return false;
        }
#endif

        return true;
    }

    /// Advances one engine frame when the generated core was initialized successfully.
    bool GameCubeApplication::UpdateEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!EngineInitialized || EngineCore == nullptr) {
            FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            return false;
        }

        try {
            SetBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0x00, 0xA0, 0x00, 0xFF });
            if (!FirstUpdateBeginReported) {
                SYS_Report("[GC] First update begin.\n");
                FirstUpdateBeginReported = true;
            }
            EngineCore->Update();
            UpdateCompletedSincePresent = true;
            if (!FirstUpdateCompletedReported) {
                SYS_Report("[GC] First update completed.\n");
                FirstUpdateCompletedReported = true;
            }
            return true;
        }
        catch (...) {
            EngineInitialized = false;
            FailBootPhase(GameCubeBootPhase::CoreUpdate, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            SYS_Report("[GC] Engine update threw.\n");
            return false;
        }
#endif

        return true;
    }

    /// Draws one engine frame when the generated core was initialized successfully.
    bool GameCubeApplication::DrawEngineCore() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        if (!EngineInitialized || EngineCore == nullptr) {
            FailBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0xFF, 0x00, 0x00, 0xFF });
            return false;
        }

        try {
            SetBootPhase(GameCubeBootPhase::CoreDraw, GXColor { 0x00, 0x60, 0x00, 0xFF });
            if (!FirstDrawBeginReported) {
                SYS_Report("[GC] First draw begin.\n");
                FirstDrawBeginReported = true;
            }
            EngineCore->Draw();
            DrawCompletedSincePresent = true;
            if (!FirstDrawCompletedReported) {
                SYS_Report("[GC] First draw completed.\n");
                FirstDrawCompletedReported = true;
            }
            return true;
        }
        catch (...) {
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
        const GXColor visibleColor = ResolvePresentedClearColor();
        PresentedFrameCount++;
        GX_SetCopyClear(visibleColor, 0x00FFFFFF);
        FrameBufferIndex ^= 1U;
        GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);
        GX_CopyDisp(FrameBuffers[FrameBufferIndex], GX_TRUE);
        GX_DrawDone();
        VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }

    /// Resolves the currently visible diagnostic color for the next presented frame.
    GXColor GameCubeApplication::ResolvePresentedClearColor() {
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        const bool evenPresentedFrame = (PresentedFrameCount & 1U) == 0U;
        if (EngineInitialized) {
            if (EngineRenderManager3D != nullptr && EngineRenderManager3D->HasRenderedScene()) {
                UpdateCompletedSincePresent = false;
                DrawCompletedSincePresent = false;
                if (evenPresentedFrame) {
                    return GXColor { 0xFF, 0x80, 0x00, 0xFF };
                }

                return GXColor { 0x80, 0x40, 0x00, 0xFF };
            }

            if (EngineRenderManager3D != nullptr && !EngineRenderManager3D->HasRenderedScene()) {
                if (evenPresentedFrame) {
                    return GXColor { 0xFF, 0x00, 0xFF, 0xFF };
                }

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
        if (HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT > 0) {
            return VerifiedFrameCount >= static_cast<uint32_t>(HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT);
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
