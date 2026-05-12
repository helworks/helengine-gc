#pragma once

#include "RuntimeModel.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"

namespace helengine::gamecube {
    /// Stores the authored model geometry arrays needed by the first GameCube GX backend tier.
    class GameCubeRuntimeModel : public RuntimeModel {
    public:
        /// Creates an empty GameCube runtime model before authored arrays are attached.
        GameCubeRuntimeModel()
            : RuntimeModel()
            , Positions(nullptr)
            , Indices16(nullptr)
            , Indices32(nullptr)
            , Uses32BitIndices(false) {
        }

        /// Authored model positions used by the first unlit triangle path.
        Array<float3>* Positions;

        /// Authored 16-bit indices when the source mesh uses them.
        Array<uint16_t>* Indices16;

        /// Authored 32-bit indices when the source mesh uses them.
        Array<uint32_t>* Indices32;

        /// Tracks whether the runtime mesh must read from `Indices32`.
        bool Uses32BitIndices;
    };
}
