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

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "PlatformInfo.hpp"
#if HELENGINE_GAMECUBE_HAS_PHYSICS3D_RUNTIME_REGISTRATION
#include "Physics3DRuntimeComponentRegistration.hpp"
#endif
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "SceneLoadMode.hpp"
#include "runtime/native_exceptions.hpp"
#include "platform/gamecube/GameCubeCubeTestSceneInstaller.hpp"
#include "platform/gamecube/GameCubeInputManager.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"
#include "platform/gamecube/GameCubeRenderManager3D.hpp"
#include "platform/gamecube/GameCubeSceneBootstrap.hpp"
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
        , VerificationProbeFailed(false)
        , VerificationCenterVisibleOnce(false)
        , VerificationMarkerVisibleOnce(false)
        , VerificationMissingSampleCount(0U)
        , MinimalSampleVertices { 0, 15, 0, -15, -15, 0, 15, -15, 0 }
        , MinimalSampleColors { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255 }
#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        , EngineCore(nullptr)
        , EngineRenderManager3D(nullptr)
        , EngineRenderManager2D(nullptr)
        , EngineInputManager(nullptr)
        , EnginePlatformInfo(nullptr)
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
        delete EnginePlatformInfo;
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

        SetBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x00, 0x00, 0xFF, 0xFF });
        if (!InitializeGraphics()) {
            FailBootPhase(GameCubeBootPhase::NativeGraphics, GXColor { 0x80, 0x00, 0x80, 0xFF });
            if (IsVerificationBuild()) {
                return GetVerificationExitCode();
            }
            return 1;
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
        VIDEO_WaitVSync();

        if (RenderMode->viTVMode & VI_NON_INTERLACE) {
            VIDEO_WaitVSync();
        }
        return true;
    }

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

        return true;
    }

    /// Initializes the DVD interface used by packaged-disc boots before any content reads occur.
    bool GameCubeApplication::InitializePackagedDisc() {
        DVD_Init();
        const s32 mountResult = DVD_Mount();
        SYS_Report("[GC] DVD_Mount result: %ld\n", static_cast<long>(mountResult));
        return mountResult >= 0;
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

        bool readSucceeded = true;
        if (__io_gcdvd.readSectors == nullptr || !__io_gcdvd.readSectors(static_cast<sec_t>(firstSectorIndex), static_cast<sec_t>(sectorCount), sectorBuffer)) {
            readSucceeded = false;
        } else {
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
            EngineCore = new Core();

            initializationStage = "ReadInitializationOptions";
            SetBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x80, 0x00, 0xFF });
            CoreInitializationOptions* options = EngineCore->get_InitializationOptions();
            if (options == nullptr) {
                FailBootPhase(GameCubeBootPhase::CoreOptions, GXColor { 0xFF, 0x00, 0xFF, 0xFF });
                return false;
            }

            initializationStage = "ConfigureSceneBootstrap";
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
            options->StandardPlatformInputConfiguration = GameCubeSceneBootstrap::CreatePackagedStandardPlatformInputConfiguration();
            const std::string packagedStartupSceneId = GameCubeSceneBootstrap::GetPackagedStartupSceneId();
            SYS_Report("[GC] Packaged startup scene id: %s\n", packagedStartupSceneId.c_str());
            SYS_Report("[GC] Runtime build stamp: %s\n", BuildStamp);
#else
            options->ContentRootPath = ".";
            options->SceneCatalog = nullptr;
#endif
            options->UpdateOrderLayers = 4;
            options->RenderOrderLayers3D = 4;
            options->UpdateListInitialCapacity = 64;
            options->RenderList2DInitialCapacity = 64;
            options->RenderList3DInitialCapacity = 64;

            initializationStage = "ConstructBridgeServices";
            SetBootPhase(GameCubeBootPhase::BridgeConstruction, GXColor { 0x00, 0xFF, 0xFF, 0xFF });
            EngineRenderManager3D = new GameCubeRenderManager3D();
            EngineRenderManager2D = new GameCubeRenderManager2D();
            EngineRenderManager3D->SetOverlayRenderManager2D(EngineRenderManager2D);
            EngineInputManager = new GameCubeInputManager();
            EnginePlatformInfo = new PlatformInfo("gamecube", "gc-headless");

            initializationStage = "AddPrimaryWindow";
            SetBootPhase(GameCubeBootPhase::CoreInitialization, GXColor { 0x00, 0x00, 0xFF, 0xFF });
            EngineRenderManager3D->AddWindow(0, RenderMode->fbWidth, RenderMode->efbHeight);
            EngineRenderManager3D->SetPresentedFrameSize(static_cast<uint16_t>(RenderMode->fbWidth), static_cast<uint16_t>(RenderMode->efbHeight));
            initializationStage = "InitializeCore";
            EngineCore->Initialize(EngineRenderManager3D, EngineRenderManager2D, EngineInputManager, EnginePlatformInfo, options);
            SYS_Report("[GC] Engine core initialized.\n");
#if HELENGINE_GAMECUBE_HAS_PHYSICS3D_RUNTIME_REGISTRATION
            initializationStage = "RegisterPhysicsRuntime";
            Physics3DRuntimeComponentRegistration::Register(EngineCore);
            SYS_Report("[GC] Physics runtime registered.\n");
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
#if HELENGINE_GAMECUBE_PACKAGED_DISC_BOOT
            if (EngineCore->get_SceneManager() == nullptr) {
                throw std::runtime_error("Packaged GameCube boot requires a runtime scene manager.");
            }

            const std::string runtimeTestSceneOverride = GetRuntimeTestSceneOverride();
            if (runtimeTestSceneOverride == "slope") {
                GameCubeCubeTestSceneInstaller::InstallSlopeScene();
                SYS_Report("[GC] Runtime slope test scene installed.\n");
            } else if (!runtimeTestSceneOverride.empty()) {
                throw std::runtime_error(std::string("Unsupported GameCube runtime test scene override: ") + runtimeTestSceneOverride);
            } else {
                const std::string packagedStartupSceneId = GameCubeSceneBootstrap::GetPackagedStartupSceneId();
                EngineCore->get_SceneManager()->LoadScene(packagedStartupSceneId, SceneLoadMode::Single);
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
                SYS_Report("[GC] First update begin.\n");
                FirstUpdateBeginReported = true;
            }
            EngineRenderManager2D->BeginFrame();
            EngineCore->Update();
            if (EngineRenderManager2D != nullptr) {
                EngineRenderManager2D->FlushReleasedTextures();
            }
            if (EngineRenderManager3D != nullptr) {
                EngineRenderManager3D->FlushReleasedAssets();
            }
            UpdateCompletedSincePresent = true;
            if (!FirstUpdateCompletedReported) {
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
                SYS_Report("[GC] First draw begin.\n");
                FirstDrawBeginReported = true;
            }
#if HELENGINE_GAMECUBE_MINIMAL_SAMPLE
            EngineCore->Draw();
            DrawCompletedSincePresent = true;
#else
            EngineCore->Draw();
            DrawCompletedSincePresent = true;
#endif
            if (!FirstDrawCompletedReported) {
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
        VIDEO_SetNextFramebuffer(FrameBuffers[FrameBufferIndex]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
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
