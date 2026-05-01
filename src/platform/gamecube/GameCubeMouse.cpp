#include "platform/gamecube/GameCubeMouse.hpp"

namespace helengine::gamecube {
    /// Creates the mouse backend with a default zeroed pointer state.
    GameCubeMouse::GameCubeMouse()
        : State()
        , PointerWrapDeltaOffset()
        , PointerWrapEnabled(false) {
    }

    /// Returns the current mouse snapshot for the bootstrap host.
    MouseState GameCubeMouse::GetState() {
        return State;
    }

    /// Sets the virtual pointer position for the backend.
    void GameCubeMouse::SetPosition(int32_t x, int32_t y) {
        State.set_X(x);
        State.set_Y(y);
    }

    /// Enables or disables pointer wrapping for the backend.
    void GameCubeMouse::SetPointerWrapEnabled(bool isEnabled) {
        PointerWrapEnabled = isEnabled;
        if (!PointerWrapEnabled) {
            PointerWrapDeltaOffset = int2(0, 0);
        }
    }

    /// Returns the pointer-wrap delta accumulated since the previous call.
    int2 GameCubeMouse::ConsumePointerWrapDeltaOffset() {
        int2 pointerWrapDeltaOffset = PointerWrapDeltaOffset;
        PointerWrapDeltaOffset = int2(0, 0);
        return pointerWrapDeltaOffset;
    }
}
