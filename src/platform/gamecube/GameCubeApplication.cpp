#include "platform/gamecube/GameCubeApplication.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

#if __has_include("Core.hpp")
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "platform/gamecube/GameCubeInputManager.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"
#include "platform/gamecube/GameCubeRenderManager3D.hpp"
#endif

namespace helengine::gamecube {
    namespace {
        constexpr std::size_t DefaultFifoSize = 256 * 1024;
    }

    /// Creates the GameCube application with no initialized native or engine state.
    GameCubeApplication::GameCubeApplication()
        : RenderMode(nullptr)
        , FrameBuffer(nullptr)
        , FifoBuffer(nullptr)
        , ClearColor { 0x00, 0xFF, 0x00, 0xFF }
        , EngineInitialized(false)
#if __has_include("Core.hpp")
        , EngineCore(nullptr)
        , EngineRenderManager3D(nullptr)
        , EngineRenderManager2D(nullptr)
        , EngineInputManager(nullptr)
#endif
    {
    }

    /// Releases generated-core bridge objects after the application loop finishes.
    GameCubeApplication::~GameCubeApplication() {
#if __has_include("Core.hpp")
        delete EngineCore;
        delete EngineInputManager;
        delete EngineRenderManager2D;
        delete EngineRenderManager3D;
#endif
    }

    /// Initializes the native host and enters the steady-state frame loop.
    int GameCubeApplication::Run() {
        if (!InitializeVideo()) {
            return 1;
        }

        if (!InitializeGraphics()) {
            return 1;
        }

        InitializeEngineCore();

        while (true) {
            UpdateEngineCore();
            PresentFrame();
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

        FrameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(RenderMode));
        if (FrameBuffer == nullptr) {
            return false;
        }

        VIDEO_Configure(RenderMode);
        VIDEO_SetNextFramebuffer(FrameBuffer);
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
        FifoBuffer = memalign(32, DefaultFifoSize);
        if (FifoBuffer == nullptr) {
            return false;
        }

        std::memset(FifoBuffer, 0, DefaultFifoSize);
        GX_Init(FifoBuffer, DefaultFifoSize);

        const f32 yScale = GX_GetYScaleFactor(RenderMode->efbHeight, RenderMode->xfbHeight);
        const u16 xfbHeight = GX_SetDispCopyYScale(yScale);

        GX_SetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
        GX_SetDispCopySrc(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
        GX_SetDispCopyDst(RenderMode->fbWidth, xfbHeight);
        GX_SetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
        GX_SetFieldMode(RenderMode->field_rendering, ((RenderMode->viHeight == (RenderMode->xfbHeight * 2)) ? GX_ENABLE : GX_DISABLE));
        GX_SetCullMode(GX_CULL_NONE);
        GX_SetDispCopyGamma(GX_GM_1_0);
        GX_SetNumChans(1);
        GX_SetNumTexGens(0);
        GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_FALSE);
        GX_SetViewport(0.0F, 0.0F, static_cast<f32>(RenderMode->fbWidth), static_cast<f32>(RenderMode->efbHeight), 0.0F, 1.0F);
        GX_InvVtxCache();
        GX_InvalidateTexAll();

        return true;
    }

    /// Initializes the generated engine core when generated sources are present in the build.
    void GameCubeApplication::InitializeEngineCore() {
#if __has_include("Core.hpp")
        try {
            SetClearColor(GXColor { 0xFF, 0xFF, 0x00, 0xFF });
            EngineCore = new Core();

            SetClearColor(GXColor { 0xFF, 0x80, 0x00, 0xFF });
            CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
            options->ContentRootPath = ".";
            options->UpdateOrderLayers = 4;
            options->RenderOrderLayers3D = 4;
            options->UpdateListInitialCapacity = 64;
            options->RenderList2DInitialCapacity = 64;
            options->RenderList3DInitialCapacity = 64;

            SetClearColor(GXColor { 0x00, 0xFF, 0xFF, 0xFF });
            EngineRenderManager3D = new GameCubeRenderManager3D();
            EngineRenderManager2D = new GameCubeRenderManager2D();
            EngineInputManager = new GameCubeInputManager();
            EngineInputManager->SetKeyboardActive(true);

            SetClearColor(GXColor { 0x00, 0x00, 0xFF, 0xFF });
            EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);
            EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, options);
            EngineInitialized = true;
            SetClearColor(GXColor { 0x00, 0xFF, 0x00, 0xFF });
        }
        catch (...) {
            EngineInitialized = false;
            SetClearColor(GXColor { 0xFF, 0x00, 0xFF, 0xFF });
        }
#endif
    }

    /// Advances one engine frame when the generated core was initialized successfully.
    void GameCubeApplication::UpdateEngineCore() {
#if __has_include("Core.hpp")
        if (!EngineInitialized || EngineCore == nullptr) {
            return;
        }

        try {
            EngineCore->Update();
            EngineCore->Draw();
        }
        catch (...) {
            EngineInitialized = false;
            SetClearColor(GXColor { 0xFF, 0x00, 0x00, 0xFF });
        }
#endif
    }

    /// Presents one fallback frame to the active framebuffer.
    void GameCubeApplication::PresentFrame() {
        GX_SetCopyClear(ClearColor, 0x00FFFFFF);
        GX_CopyDisp(FrameBuffer, GX_TRUE);
        GX_DrawDone();
        GX_Flush();
        VIDEO_SetNextFramebuffer(FrameBuffer);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }

    /// Updates the currently presented clear color used for boot-state diagnostics.
    void GameCubeApplication::SetClearColor(GXColor color) {
        ClearColor = color;
    }
}
