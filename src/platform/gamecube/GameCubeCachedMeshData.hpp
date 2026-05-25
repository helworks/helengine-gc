#pragma once

#include <cstdint>

#include "float2.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"

namespace helengine::gamecube {
    /// Owns the cached GameCube-native mesh data that the GX renderer can reuse across frames.
    class GameCubeCachedMeshData {
    public:
        /// Creates an empty cached mesh container before cached arrays are attached.
        GameCubeCachedMeshData()
            : Positions(nullptr)
            , Normals(nullptr)
            , TexCoords(nullptr)
            , Indices16(nullptr)
            , SubmeshIndexStarts(nullptr)
            , SubmeshIndexCounts(nullptr)
            , HasNormals(false)
            , HasTexCoords(false) {
        }

        /// Cached positions stored for the default GameCube draw path.
        Array<float3>* Positions;

        /// Cached normals stored when the source mesh supports lit rendering.
        Array<float3>* Normals;

        /// Cached texture coordinates stored when the source mesh supports textured rendering.
        Array<float2>* TexCoords;

        /// Cached 16-bit indices used by the GameCube draw path.
        Array<uint16_t>* Indices16;

        /// Cached submesh start offsets measured in cached index entries.
        Array<int32_t>* SubmeshIndexStarts;

        /// Cached submesh index counts for each authored submesh.
        Array<int32_t>* SubmeshIndexCounts;

        /// Tracks whether cached normals were attached for lit rendering.
        bool HasNormals;

        /// Tracks whether cached texture coordinates were attached for textured rendering.
        bool HasTexCoords;
    };
}
