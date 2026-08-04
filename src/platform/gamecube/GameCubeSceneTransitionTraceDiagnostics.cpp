#include "platform/gamecube/GameCubeSceneTransitionTraceDiagnostics.hpp"

#include <stdexcept>

#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "platform/gamecube/GameCubeApplication.hpp"

namespace helengine::gamecube {
    /// Creates a scene-transition diagnostics sink that presents checkpoints through the supplied application.
    GameCubeSceneTransitionTraceDiagnostics::GameCubeSceneTransitionTraceDiagnostics(GameCubeApplication* application)
        : Application(application) {
        if (Application == nullptr) {
            throw std::invalid_argument("A GameCube scene-transition trace requires an application.");
        }
    }

    /// Creates an empty runtime memory snapshot required by the generated diagnostics-provider contract.
    RuntimeMemoryDiagnosticsSnapshot* GameCubeSceneTransitionTraceDiagnostics::CaptureSnapshot() {
        return new RuntimeMemoryDiagnosticsSnapshot();
    }

    /// Presents the color assigned to one selected scene-manager transition stage.
    void GameCubeSceneTransitionTraceDiagnostics::ReportSceneTransitionStage(
        std::string stage,
        std::string sceneId,
        int32_t loadedSceneCount,
        int32_t pendingOperationCount) {
        static_cast<void>(sceneId);
        static_cast<void>(loadedSceneCount);
        static_cast<void>(pendingOperationCount);

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        uint16_t code = 0U;
        if (TryResolveStageCode(stage, code)) {
            ReportDirectFrameDiagnosticCode(code);
        }
#else
        GXColor color {};
        if (TryResolveStageColor(stage, color)) {
            Application->PresentFirstFrameTraceCheckpoint(color);
        }
#endif
    }

    /// Resolves whether one generated stage has a dedicated visible checkpoint color.
    bool GameCubeSceneTransitionTraceDiagnostics::TryResolveStageColor(const std::string& stage, GXColor& color) {
        if (stage == "CommitPendingOperationsAtFrameBoundaryBegin") {
            color = GXColor { 0xC0, 0x20, 0xFF, 0xFF };
        } else if (stage == "CommitPendingOperationsAtFrameBoundaryOperation") {
            color = GXColor { 0xFF, 0x90, 0x20, 0xFF };
        } else if (stage == "LoadSceneImmediateBegin") {
            color = GXColor { 0xFF, 0x40, 0x20, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeResolveSceneContentPath") {
            color = GXColor { 0xFF, 0x60, 0x20, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterResolveSceneContentPath") {
            color = GXColor { 0xFF, 0x80, 0x20, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeLoadedSceneRecordLookup") {
            color = GXColor { 0xFF, 0xA0, 0x20, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterLoadedSceneRecordLookup") {
            color = GXColor { 0xFF, 0xC0, 0x20, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeContentLoad") {
            color = GXColor { 0xFF, 0x00, 0x40, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeSceneLoadServiceLoad") {
            color = GXColor { 0x00, 0x80, 0xFF, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterSceneLoadServiceLoad") {
            color = GXColor { 0x00, 0xFF, 0xE0, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeLoadedSceneRecordTrack") {
            color = GXColor { 0x80, 0xFF, 0x40, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterLoadedSceneRecordListAdd") {
            color = GXColor { 0xFF, 0xD0, 0x00, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterLoadedSceneRecordDictionaryAdd") {
            color = GXColor { 0xFF, 0x80, 0x00, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedTextures") {
            color = GXColor { 0xFF, 0x00, 0x00, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedFonts") {
            color = GXColor { 0xE0, 0x00, 0xFF, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedAudio") {
            color = GXColor { 0x00, 0x00, 0xFF, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedModels") {
            color = GXColor { 0x00, 0xC0, 0xFF, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedMaterials") {
            color = GXColor { 0x00, 0xFF, 0xA0, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterRegisterOwnedAssets") {
            color = GXColor { 0x00, 0xFF, 0x00, 0xFF };
        } else if (stage == "LoadSceneImmediateBeforeSceneLoadedEvent") {
            color = GXColor { 0xFF, 0x00, 0xA0, 0xFF };
        } else if (stage == "LoadSceneImmediateAfterSceneLoadedEvent") {
            color = GXColor { 0xFF, 0xFF, 0xFF, 0xFF };
        } else if (stage == "LoadSceneImmediateEnd") {
            color = GXColor { 0xFF, 0xFF, 0xFF, 0xFF };
        } else {
            return false;
        }

        return true;
    }

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
    /// Resolves one generated scene-transition stage into a direct-frame diagnostic code.
    bool GameCubeSceneTransitionTraceDiagnostics::TryResolveStageCode(const std::string& stage, uint16_t& code) {
        if (stage == "CommitPendingOperationsAtFrameBoundaryBegin") {
            code = 0xE001U;
        } else if (stage == "CommitPendingOperationsAtFrameBoundaryOperation") {
            code = 0xE002U;
        } else if (stage == "LoadSceneImmediateBegin") {
            code = 0xE003U;
        } else if (stage == "LoadSceneImmediateBeforeResolveSceneContentPath") {
            code = 0xE013U;
        } else if (stage == "LoadSceneImmediateAfterResolveSceneContentPath") {
            code = 0xE014U;
        } else if (stage == "LoadSceneImmediateBeforeLoadedSceneRecordLookup") {
            code = 0xE015U;
        } else if (stage == "LoadSceneImmediateAfterLoadedSceneRecordLookup") {
            code = 0xE016U;
        } else if (stage == "LoadSceneImmediateBeforeContentLoad") {
            code = 0xE004U;
        } else if (stage == "LoadSceneImmediateBeforeSceneLoadServiceLoad") {
            code = 0xE005U;
        } else if (stage == "LoadSceneImmediateAfterSceneLoadServiceLoad") {
            code = 0xE006U;
        } else if (stage == "LoadSceneImmediateBeforeLoadedSceneRecordTrack") {
            code = 0xE007U;
        } else if (stage == "LoadSceneImmediateAfterLoadedSceneRecordListAdd") {
            code = 0xE008U;
        } else if (stage == "LoadSceneImmediateAfterLoadedSceneRecordDictionaryAdd") {
            code = 0xE009U;
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedTextures") {
            code = 0xE00AU;
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedFonts") {
            code = 0xE00BU;
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedAudio") {
            code = 0xE00CU;
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedModels") {
            code = 0xE00DU;
        } else if (stage == "LoadSceneImmediateBeforeRegisterOwnedMaterials") {
            code = 0xE00EU;
        } else if (stage == "LoadSceneImmediateAfterRegisterOwnedAssets") {
            code = 0xE00FU;
        } else if (stage == "LoadSceneImmediateBeforeSceneLoadedEvent") {
            code = 0xE010U;
        } else if (stage == "LoadSceneImmediateAfterSceneLoadedEvent") {
            code = 0xE011U;
        } else if (stage == "LoadSceneImmediateEnd") {
            code = 0xE012U;
        } else {
            return false;
        }

        return true;
    }
#endif
}
