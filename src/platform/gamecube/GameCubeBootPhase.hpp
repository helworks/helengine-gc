#pragma once

namespace helengine::gamecube {
    /// Identifies the current GameCube host boot phase for deterministic runtime diagnostics.
    enum class GameCubeBootPhase {
        NativeVideo,
        NativeGraphics,
        CoreConstruction,
        CoreOptions,
        BridgeConstruction,
        CoreInitialization,
        CoreUpdate,
        CoreDraw,
        Running,
        Failed
    };
}
