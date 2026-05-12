#pragma once

#include <gccore.h>

#include "platform/gamecube/GameCubeBootPhase.hpp"

class Core;

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

        /// Initializes the generated engine core when generated sources are present in the build.
        bool InitializeEngineCore();

        /// Advances one engine frame when the generated core was initialized successfully.
        bool UpdateEngineCore();

        /// Draws one engine frame when the generated core was initialized successfully.
        bool DrawEngineCore();

        /// Presents one fallback frame to the active framebuffer.
        void PresentFrame();

        /// Presents the current failure state forever after a boot-phase failure.
        void PresentFailureLoop();

        /// Updates the currently presented clear color used for boot-state diagnostics.
        void SetClearColor(GXColor color);

        /// Sets the current boot phase and visible clear color.
        void SetBootPhase(GameCubeBootPhase phase, GXColor color);

        /// Marks the current boot phase as failed and updates the visible clear color.
        void FailBootPhase(GameCubeBootPhase phase, GXColor color);

        /// Stores the preferred video mode selected for the current console or emulator.
        GXRModeObj* RenderMode;

        /// Stores the allocated external framebuffer used for display output.
        void* FrameBuffer;

        /// Stores the GX command FIFO allocation used by the host bootstrap.
        void* FifoBuffer;

        /// Stores the current fallback clear color for crash-phase diagnostics.
        GXColor ClearColor;

        /// Stores the current host boot phase.
        GameCubeBootPhase BootPhase;

        /// Tracks whether the generated engine core finished initialization.
        bool EngineInitialized;

#if HELENGINE_GAMECUBE_HAS_GENERATED_CORE
        /// Stores the generated engine core instance when the build includes generated sources.
        Core* EngineCore;

        /// Stores the generated 3D render manager bridge.
        GameCubeRenderManager3D* EngineRenderManager3D;

        /// Stores the generated 2D render manager bridge.
        GameCubeRenderManager2D* EngineRenderManager2D;

        /// Stores the generated input manager bridge.
        GameCubeInputManager* EngineInputManager;
#endif
    };
}
