#include "platform/gamecube/GameCubeExceptionDiagnostics.hpp"

#include <stdexcept>

#include <tuxedo/ppc/exception.h>
#include <tuxedo/ppc/intrinsics.h>

#include "platform/gamecube/GameCubeApplication.hpp"

namespace helengine::gamecube {
    GameCubeApplication* GameCubeExceptionDiagnostics::Application = nullptr;
    int32_t GameCubeExceptionDiagnostics::SceneListCount = -1;
    int32_t GameCubeExceptionDiagnostics::SceneListCapacity = -1;
    const void* GameCubeExceptionDiagnostics::SceneListData = nullptr;

    /// Installs the native PowerPC panic callback for the application that owns the active video framebuffer.
    void GameCubeExceptionDiagnostics::Install(GameCubeApplication* application) {
        if (application == nullptr) {
            throw std::invalid_argument("A GameCube exception diagnostic requires an application.");
        }

        Application = application;
        PPCExcptCurPanicFn = HandlePanic;
    }

    /// Stores the scene-list metadata immediately before the generated runtime performs the suspected list insertion.
    void GameCubeExceptionDiagnostics::CaptureSceneListState(int32_t count, int32_t capacity, const void* data) {
        SceneListCount = count;
        SceneListCapacity = capacity;
        SceneListData = data;
    }

    /// Receives one unrecoverable PowerPC exception from libogc and transfers its captured context to the framebuffer crash screen.
    void GameCubeExceptionDiagnostics::HandlePanic(unsigned exceptionId, PPCContext* context) {
        const uint32_t faultAddress = exceptionId == PPC_EXCPT_DSI ? PPCMfspr(DAR) : 0U;
        const uint32_t dataStorageInterruptStatus = exceptionId == PPC_EXCPT_DSI ? PPCMfspr(DSISR) : 0U;
        if (Application != nullptr) {
            Application->PresentExceptionDiagnostics(
                exceptionId,
                context,
                faultAddress,
                dataStorageInterruptStatus,
                SceneListCount,
                SceneListCapacity,
                SceneListData);
        }

        while (true) {
        }
    }
}
