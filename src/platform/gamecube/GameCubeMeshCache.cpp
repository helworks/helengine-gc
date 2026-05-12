#include "platform/gamecube/GameCubeMeshCache.hpp"

#include "RuntimeModel.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/string_comparer.hpp"

namespace helengine::gamecube {
    /// Creates an empty runtime-model cache keyed by authored runtime id.
    GameCubeMeshCache::GameCubeMeshCache()
        : CachedModelsById(new Dictionary<std::string, GameCubeRuntimeModel*>(StringComparer::OrdinalIgnoreCase)) {
    }

    /// Resolves a runtime model into a validated GameCube runtime model or fails loudly.
    GameCubeRuntimeModel* GameCubeMeshCache::Resolve(RuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        }

        GameCubeRuntimeModel* cachedRuntimeModel;
        if (CachedModelsById->TryGetValue(runtimeModel->get_Id(), cachedRuntimeModel)) {
            return cachedRuntimeModel;
        }

        GameCubeRuntimeModel* typedRuntimeModel = he_cpp_try_cast<GameCubeRuntimeModel>(runtimeModel);
        if (typedRuntimeModel == nullptr) {
            throw new InvalidOperationException("GameCube 3D rendering requires GameCubeRuntimeModel instances.");
        } else if (typedRuntimeModel->Positions == nullptr || typedRuntimeModel->Positions->Length == 0) {
            throw new InvalidOperationException("GameCube runtime models must contain authored position data.");
        } else if (!typedRuntimeModel->Uses32BitIndices && (typedRuntimeModel->Indices16 == nullptr || typedRuntimeModel->Indices16->Length == 0)) {
            throw new InvalidOperationException("GameCube runtime models using 16-bit indices must provide authored index data.");
        } else if (typedRuntimeModel->Uses32BitIndices && (typedRuntimeModel->Indices32 == nullptr || typedRuntimeModel->Indices32->Length == 0)) {
            throw new InvalidOperationException("GameCube runtime models using 32-bit indices must provide authored index data.");
        }

        CachedModelsById->Add(runtimeModel->get_Id(), typedRuntimeModel);
        return typedRuntimeModel;
    }
}
