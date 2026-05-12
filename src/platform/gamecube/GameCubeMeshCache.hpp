#pragma once

#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"

class RuntimeModel;

namespace helengine::gamecube {
    class GameCubeRuntimeModel;

    /// Validates authored runtime models once and reuses the backend-local geometry view across frames.
    class GameCubeMeshCache {
    public:
        /// Creates an empty runtime-model cache keyed by authored runtime id.
        GameCubeMeshCache();

        /// Resolves a runtime model into a validated GameCube runtime model or fails loudly.
        GameCubeRuntimeModel* Resolve(RuntimeModel* runtimeModel);
    private:
        /// Cache of runtime models already validated for GX drawing.
        Dictionary<std::string, GameCubeRuntimeModel*>* CachedModelsById;
    };
}
