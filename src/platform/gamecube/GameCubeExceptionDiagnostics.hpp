#pragma once

#include <cstdint>

struct PPCContext;

namespace helengine::gamecube {
    class GameCubeApplication;

    /// Captures native PowerPC faults and forwards a stable scene-list snapshot to the GameCube framebuffer crash screen.
    class GameCubeExceptionDiagnostics final {
    public:
        /// Installs the native PowerPC panic callback for the application that owns the active video framebuffer.
        /// <param name="application">Initialized GameCube application that will render the crash screen.</param>
        static void Install(GameCubeApplication* application);

        /// Stores the scene-list metadata immediately before the generated runtime performs the suspected list insertion.
        /// <param name="count">Current number of scene records in the native list.</param>
        /// <param name="capacity">Current reserved capacity of the native list.</param>
        /// <param name="data">Native storage address returned by the scene-record list.</param>
        static void CaptureSceneListState(int32_t count, int32_t capacity, const void* data);

    private:
        /// Receives one unrecoverable PowerPC exception from libogc and transfers its captured context to the framebuffer crash screen.
        /// <param name="exceptionId">Hardware exception identifier supplied by the PowerPC runtime.</param>
        /// <param name="context">Saved PowerPC processor context at the faulting instruction.</param>
        static void HandlePanic(unsigned exceptionId, PPCContext* context);

        /// Stores the application that owns the framebuffer used for diagnostic text.
        static GameCubeApplication* Application;

        /// Stores the scene-list element count captured before the generated scene-record insertion.
        static int32_t SceneListCount;

        /// Stores the scene-list capacity captured before the generated scene-record insertion.
        static int32_t SceneListCapacity;

        /// Stores the scene-list storage address captured before the generated scene-record insertion.
        static const void* SceneListData;
    };
}
