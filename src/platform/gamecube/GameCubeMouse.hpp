#pragma once

#include "Mouse.hpp"
#include "MouseState.hpp"
#include "helengine_int2.hpp"

namespace helengine::gamecube {
    /// Implements the generated mouse contract for the current GameCube bootstrap with a fixed default pointer state.
    class GameCubeMouse : public Mouse {
    public:
        /// Creates the mouse backend with a default zeroed pointer state.
        GameCubeMouse();

        /// Returns the current mouse snapshot for the bootstrap host.
        MouseState GetState() override;

        /// Sets the virtual pointer position for the backend.
        void SetPosition(int32_t x, int32_t y) override;

        /// Enables or disables pointer wrapping for the backend.
        void SetPointerWrapEnabled(bool isEnabled) override;

        /// Returns the pointer-wrap delta accumulated since the previous call.
        int2 ConsumePointerWrapDeltaOffset() override;

    private:
        /// Stores the current pointer state reported to the generated input layer.
        MouseState State;

        /// Stores the most recent pointer-wrap offset consumed by the generated input layer.
        int2 PointerWrapDeltaOffset;

        /// Tracks whether pointer wrapping is currently enabled.
        bool PointerWrapEnabled;
    };
}
