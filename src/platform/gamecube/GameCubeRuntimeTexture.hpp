#pragma once

#include <cstddef>
#include <cstdint>

#include <gccore.h>

#include "RuntimeTexture.hpp"

class TextureAsset;

namespace helengine::gamecube {
    /// Stores one GameCube-native GX texture object plus its encoded texture memory.
    class GameCubeRuntimeTexture final : public RuntimeTexture {
    public:
        /// Creates one empty GameCube runtime texture with no native texture memory yet.
        GameCubeRuntimeTexture();

        /// Releases owned GameCube texture memory.
        ~GameCubeRuntimeTexture() override;

        /// Encodes one shared-engine texture asset into the first supported GameCube GX texture format.
        void LoadFromRaw(TextureAsset* data);

        /// Returns whether a native GX texture object was initialized for this runtime texture.
        bool HasNativeTextureObject() const;

        /// Returns the native GX texture object used by textured material submission.
        GXTexObj* GetNativeTextureObject();

    private:
        /// Owned native GX texture-memory buffer.
        void* NativeTextureData;

        /// Size in bytes of the owned native texture-memory buffer.
        std::size_t NativeTextureDataSize;

        /// Native GX texture object initialized from `NativeTextureData`.
        GXTexObj NativeTextureObject;

        /// Tracks whether `NativeTextureObject` is valid.
        bool NativeTextureObjectInitialized;

        /// Releases any previously encoded native texture memory.
        void ResetNativeTextureData();

        /// Loads one prepacked GX RGB5A3 payload that is already stored in native tiled texture memory order.
        void LoadPrepackedRgb5A3(TextureAsset* data);

        /// Encodes one RGBA8 logical texture into tiled GX RGB5A3 memory.
        void EncodeRgba32ToRgb5A3(TextureAsset* data);
    };
}
