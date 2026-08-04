#pragma once

#include <string>

#include <gccore.h>

#include "IRuntimeDiagnosticsProvider.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"

class RuntimeMemoryDiagnosticsSnapshot;

namespace helengine::gamecube {
    class GameCubeApplication;

    /// Presents generated scene-manager transition boundaries through the GameCube first-frame trace display.
    class GameCubeSceneTransitionTraceDiagnostics final : public IRuntimeDiagnosticsProvider, public IRuntimeSceneTransitionDiagnosticsProvider {
    public:
        /// Creates a scene-transition diagnostics sink that presents checkpoints through the supplied application.
        /// <param name="application">GameCube host that owns the visible diagnostic display.</param>
        explicit GameCubeSceneTransitionTraceDiagnostics(GameCubeApplication* application);

        /// Creates an empty runtime memory snapshot required by the generated diagnostics-provider contract.
        /// <returns>New empty snapshot because this provider only reports scene-transition boundaries.</returns>
        RuntimeMemoryDiagnosticsSnapshot* CaptureSnapshot() override;

        /// Presents the color assigned to one selected scene-manager transition stage.
        /// <param name="stage">Generated scene-manager stage label.</param>
        /// <param name="sceneId">Stable scene identifier associated with the stage.</param>
        /// <param name="loadedSceneCount">Number of currently loaded scene records.</param>
        /// <param name="pendingOperationCount">Number of scene operations remaining after the notification.</param>
        void ReportSceneTransitionStage(std::string stage, std::string sceneId, int32_t loadedSceneCount, int32_t pendingOperationCount) override;

    private:
        /// Resolves whether one generated stage has a dedicated visible checkpoint color.
        /// <param name="stage">Generated scene-manager stage label.</param>
        /// <param name="color">Receives the selected checkpoint color when the stage is traced.</param>
        /// <returns>True when the stage should be presented; otherwise false.</returns>
        static bool TryResolveStageColor(const std::string& stage, GXColor& color);

#if HELENGINE_GAMECUBE_DIRECT_FRAME_DIAGNOSTIC
        /// Resolves one generated scene-transition stage into a direct-frame diagnostic code.
        /// <param name="stage">Generated scene-manager stage label.</param>
        /// <param name="code">Receives the direct-frame code when the stage is traced.</param>
        /// <returns>True when the stage has a dedicated direct-frame checkpoint; otherwise false.</returns>
        static bool TryResolveStageCode(const std::string& stage, uint16_t& code);
#endif

        /// Stores the host that presents visible trace checkpoints.
        GameCubeApplication* Application;
    };
}
