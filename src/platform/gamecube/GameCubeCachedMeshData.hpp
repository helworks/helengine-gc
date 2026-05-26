#pragma once

#include <cstdint>

#include "float2.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"

namespace helengine::gamecube {
    /// Stores one tightly packed GX position entry without managed-runtime object overhead.
    struct GameCubePackedPosition3 {
        float X;
        float Y;
        float Z;
    };

    /// Stores one tightly packed GX normal entry without managed-runtime object overhead.
    struct GameCubePackedNormal3 {
        float X;
        float Y;
        float Z;
    };

    /// Stores one tightly packed GX texture-coordinate entry without managed-runtime object overhead.
    struct GameCubePackedTexCoord2 {
        float U;
        float V;
    };

    /// Owns the cached GameCube-native mesh data that the GX renderer can reuse across frames.
    class GameCubeCachedMeshData {
    public:
        /// Creates an empty cached mesh container before cached arrays are attached.
        GameCubeCachedMeshData()
            : PackedPositions(nullptr)
            , PackedNormals(nullptr)
            , Normals(nullptr)
            , PackedTexCoords(nullptr)
            , Indices16(nullptr)
            , SubmeshIndexStarts(nullptr)
            , SubmeshIndexCounts(nullptr)
            , HasNormals(false)
            , HasTexCoords(false) {
        }

        /// Packed positions stored for the default GameCube indexed draw path.
        Array<GameCubePackedPosition3>* PackedPositions;

        /// Packed normals stored for the default GameCube indexed lit draw path.
        Array<GameCubePackedNormal3>* PackedNormals;

        /// Cached normals stored when the source mesh supports lit rendering.
        Array<float3>* Normals;

        /// Packed texture coordinates stored when the source mesh supports textured rendering.
        Array<GameCubePackedTexCoord2>* PackedTexCoords;

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
