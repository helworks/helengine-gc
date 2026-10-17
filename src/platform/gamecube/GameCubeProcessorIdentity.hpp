#pragma once

#include <cstdint>

namespace helengine::gamecube {
    /// Recognizes documented GameCube Gekko and Lonestar processor revisions before any Wii-only MEM2 access.
    /// Broadway and Espresso identities remain eligible for Nintendont detection.
    inline bool IsGameCubeProcessorVersion(uint32_t processorVersion) {
        const uint32_t processorFamily = processorVersion & 0xFFFFFF00U;
        return processorFamily == 0x00083200U
            || processorFamily == 0x00080100U
            || processorFamily == 0x00080200U
            || processorFamily == 0x00088200U
            || processorFamily == 0x70000100U;
    }
}
