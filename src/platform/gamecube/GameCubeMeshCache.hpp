#pragma once

#include "platform/gamecube/GameCubeCachedMeshData.hpp"

class RuntimeModel;

namespace helengine::gamecube {
    class GameCubeRuntimeModel;

    /// Validates authored runtime models once and reuses the backend-local geometry view across frames.
    class GameCubeMeshCache {
    public:
        /// Creates one GameCube mesh cache that keeps cached data on each runtime-model instance.
        GameCubeMeshCache();

        /// Resolves a runtime model into a validated GameCube runtime model or fails loudly.
        GameCubeRuntimeModel* Resolve(RuntimeModel* runtimeModel);

    private:
        /// Builds one cached GameCube mesh representation from the authored runtime-model arrays.
        GameCubeCachedMeshData* BuildCachedMeshData(GameCubeRuntimeModel* runtimeModel);
    };
}
