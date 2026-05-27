#include "platform/gamecube/GameCubeMeshCache.hpp"

#include <limits>

#include <ogc/cache.h>

#include "RuntimeModel.hpp"
#include "RuntimeSubmesh.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    /// Creates one GameCube mesh cache that keeps cached data on each runtime-model instance.
    GameCubeMeshCache::GameCubeMeshCache()
    {
    }

    /// Resolves a runtime model into a validated GameCube runtime model or fails loudly.
    GameCubeRuntimeModel* GameCubeMeshCache::Resolve(RuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
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

        if (typedRuntimeModel->CachedMeshData == nullptr) {
            typedRuntimeModel->CachedMeshData = BuildCachedMeshData(typedRuntimeModel);
        }

        return typedRuntimeModel;
    }

    /// Builds one cached GameCube mesh representation from the authored runtime-model arrays.
    GameCubeCachedMeshData* GameCubeMeshCache::BuildCachedMeshData(GameCubeRuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeModel->Positions == nullptr || runtimeModel->Positions->Length == 0) {
            throw new InvalidOperationException("GameCube cached mesh build requires authored position data.");
        }

        Array<RuntimeSubmesh*>* submeshes = runtimeModel->get_Submeshes();
        if (submeshes == nullptr || submeshes == Array<RuntimeSubmesh*>::Empty() || submeshes->get_Length() == 0) {
            throw new InvalidOperationException("GameCube cached mesh build requires authored submesh metadata.");
        }

        GameCubeCachedMeshData* cachedMeshData = new GameCubeCachedMeshData();
        cachedMeshData->PackedPositions = new Array<GameCubePackedPosition3>(runtimeModel->Positions->Length);
        for (int32_t positionIndex = 0; positionIndex < runtimeModel->Positions->Length; positionIndex++) {
            const float3 position = (*runtimeModel->Positions)[positionIndex];
            (*cachedMeshData->PackedPositions)[positionIndex] = GameCubePackedPosition3 {
                position.X,
                position.Y,
                position.Z
            };
        }

        if (runtimeModel->Normals != nullptr && runtimeModel->Normals != Array<float3>::Empty()) {
            if (runtimeModel->Normals->Length != runtimeModel->Positions->Length) {
                throw new InvalidOperationException("GameCube cached mesh normals must match the authored position count.");
            }

            cachedMeshData->PackedNormals = new Array<GameCubePackedNormal3>(runtimeModel->Normals->Length);
            for (int32_t normalIndex = 0; normalIndex < runtimeModel->Normals->Length; normalIndex++) {
                const float3 normal = (*runtimeModel->Normals)[normalIndex];
                (*cachedMeshData->PackedNormals)[normalIndex] = GameCubePackedNormal3 {
                    normal.X,
                    normal.Y,
                    normal.Z
                };
            }

            cachedMeshData->Normals = runtimeModel->Normals;
            cachedMeshData->HasNormals = true;
        }

        if (runtimeModel->TexCoords != nullptr && runtimeModel->TexCoords != Array<float2>::Empty()) {
            if (runtimeModel->TexCoords->Length != runtimeModel->Positions->Length) {
                throw new InvalidOperationException("GameCube cached mesh texture coordinates must match the authored position count.");
            }

            cachedMeshData->PackedTexCoords = new Array<GameCubePackedTexCoord2>(runtimeModel->TexCoords->Length);
            for (int32_t texCoordIndex = 0; texCoordIndex < runtimeModel->TexCoords->Length; texCoordIndex++) {
                const float2 texCoord = (*runtimeModel->TexCoords)[texCoordIndex];
                (*cachedMeshData->PackedTexCoords)[texCoordIndex] = GameCubePackedTexCoord2 {
                    texCoord.X,
                    texCoord.Y
                };
            }

            cachedMeshData->HasTexCoords = true;
        }

        const int32_t submeshCount = submeshes->get_Length();
        cachedMeshData->SubmeshIndexStarts = new Array<int32_t>(submeshCount);
        cachedMeshData->SubmeshIndexCounts = new Array<int32_t>(submeshCount);
        for (int32_t submeshIndex = 0; submeshIndex < submeshCount; submeshIndex++) {
            RuntimeSubmesh* submesh = (*submeshes)[submeshIndex];
            if (submesh == nullptr) {
                throw new InvalidOperationException("GameCube cached mesh build cannot process null authored submeshes.");
            }

            const int32_t indexStart = submesh->get_IndexStart();
            const int32_t indexCount = submesh->get_IndexCount();
            if (indexStart < 0 || indexCount <= 0) {
                throw new InvalidOperationException("GameCube cached mesh submesh ranges must contain positive authored index spans.");
            }

            (*cachedMeshData->SubmeshIndexStarts)[submeshIndex] = indexStart;
            (*cachedMeshData->SubmeshIndexCounts)[submeshIndex] = indexCount;
        }

        const int32_t sourceIndexCount = runtimeModel->Uses32BitIndices
            ? runtimeModel->Indices32->Length
            : runtimeModel->Indices16->Length;
        cachedMeshData->Indices16 = new Array<uint16_t>(sourceIndexCount);
        if (runtimeModel->Uses32BitIndices) {
            for (int32_t index = 0; index < sourceIndexCount; index++) {
                const uint32_t sourceIndex = (*runtimeModel->Indices32)[index];
                if (sourceIndex > static_cast<uint32_t>(std::numeric_limits<uint16_t>::max())) {
                    throw new InvalidOperationException("GameCube cached mesh build only supports indices that fit in 16 bits.");
                }

                (*cachedMeshData->Indices16)[index] = static_cast<uint16_t>(sourceIndex);
            }
        } else {
            for (int32_t index = 0; index < sourceIndexCount; index++) {
                (*cachedMeshData->Indices16)[index] = (*runtimeModel->Indices16)[index];
            }
        }

        for (int32_t submeshIndex = 0; submeshIndex < submeshCount; submeshIndex++) {
            const int32_t indexStart = (*cachedMeshData->SubmeshIndexStarts)[submeshIndex];
            const int32_t indexCount = (*cachedMeshData->SubmeshIndexCounts)[submeshIndex];
            if (indexStart + indexCount > sourceIndexCount) {
                throw new InvalidOperationException("GameCube cached mesh submesh ranges must stay within the authored index buffer.");
            }
        }

        DCFlushRange(&(*cachedMeshData->PackedPositions)[0], static_cast<u32>(cachedMeshData->PackedPositions->Length * sizeof(GameCubePackedPosition3)));
        if (cachedMeshData->HasNormals) {
            DCFlushRange(&(*cachedMeshData->PackedNormals)[0], static_cast<u32>(cachedMeshData->PackedNormals->Length * sizeof(GameCubePackedNormal3)));
            DCFlushRange(&(*cachedMeshData->Normals)[0], static_cast<u32>(cachedMeshData->Normals->Length * sizeof(float3)));
        }

        if (cachedMeshData->HasTexCoords) {
            DCFlushRange(&(*cachedMeshData->PackedTexCoords)[0], static_cast<u32>(cachedMeshData->PackedTexCoords->Length * sizeof(GameCubePackedTexCoord2)));
        }

        DCFlushRange(&(*cachedMeshData->Indices16)[0], static_cast<u32>(cachedMeshData->Indices16->Length * sizeof(uint16_t)));

        return cachedMeshData;
    }
}
