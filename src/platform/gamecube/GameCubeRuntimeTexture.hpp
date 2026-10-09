#pragma once

#include <cstddef>
#include <cstdint>

#include <gccore.h>

#include "RuntimeTexture.hpp"
#include "runtime/array.hpp"
#include "platform/gamecube/GameCubeNativeTextureLayout.hpp"

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

        /// Re-encodes one RGBA8 source rectangle into the owned tiled GX RGB5A3 texture memory and republishes it to the graphics processor.
        void UpdateRegionFromRgba8(int32_t x, int32_t y, int32_t width, int32_t height, Array<uint8_t>* rgba8, int32_t sourceRowPitch);

        /// Returns whether a native GX texture object was initialized for this runtime texture.
        bool HasNativeTextureObject() const;

        /// Returns the native GX texture object used by textured material submission.
        GXTexObj* GetNativeTextureObject();

        /// Loads the associated TLUT, when present, before binding this texture to one hardware texture map.
        void BindNativeTexture(std::uint8_t map);

    private:
        /// Owned native GX texture-memory buffer.
        void* NativeTextureData;

        /// Size in bytes of the owned native texture-memory buffer.
        std::size_t NativeTextureDataSize;

        /// Native GX texture object initialized from `NativeTextureData`.
        GXTexObj NativeTextureObject;

        /// Tracks whether `NativeTextureObject` is valid.
        bool NativeTextureObjectInitialized;

        /// Owned aligned native TLUT bytes retained until a safe renderer retirement boundary.
        void* NativePaletteData;

        /// Native TLUT descriptor whose format and actual entry count come from the validated header.
        GXTlutObj NativePaletteObject;

        /// Canonical layout for a GXN1 asset; legacy RGB5A3 textures retain their existing update policy.
        GameCubeNativeTextureLayout NativeLayout;

        /// Distinguishes strict GXN1 assets from the historical raw RGB5A3 route.
        bool UsesNativePayload;

        /// Releases any previously encoded native texture memory.
        void ResetNativeTextureData();

        /// Loads one prepacked GX RGB5A3 payload that is already stored in native tiled texture memory order.
        void LoadPrepackedRgb5A3(TextureAsset* data);

        /// Copies validated GXN1 tiled bytes and native TLUT bytes directly without RGBA expansion.
        void LoadNativePayload(TextureAsset* data);

        /// Encodes one RGBA8 logical texture into tiled GX RGB5A3 memory.
        void EncodeRgba32ToRgb5A3(TextureAsset* data);
    };
}
