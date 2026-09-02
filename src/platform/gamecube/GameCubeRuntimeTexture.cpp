#include "platform/gamecube/GameCubeRuntimeTexture.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <malloc.h>
#include <ogc/cache.h>
#include "TextureAsset.hpp"
#include "TextureAssetColorFormat.hpp"
#include "runtime/native_exceptions.hpp"

namespace {
    /// Converts one 8-bit channel into the 5-bit range used by opaque GX RGB5A3 texels.
    static uint16_t Convert8To5(uint8_t value) {
        return static_cast<uint16_t>((static_cast<uint32_t>(value) * 31U + 127U) / 255U);
    }

    /// Converts one 8-bit channel into the 4-bit range used by translucent GX RGB5A3 texels.
    static uint16_t Convert8To4(uint8_t value) {
        return static_cast<uint16_t>((static_cast<uint32_t>(value) * 15U + 127U) / 255U);
    }

    /// Converts one 8-bit alpha channel into the 3-bit range used by translucent GX RGB5A3 texels.
    static uint16_t Convert8To3(uint8_t value) {
        return static_cast<uint16_t>((static_cast<uint32_t>(value) * 7U + 127U) / 255U);
    }

    /// Encodes one logical RGBA texel into the GX RGB5A3 packed representation.
    static uint16_t EncodeRgb5A3Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
        if (alpha >= 224U) {
            return static_cast<uint16_t>(
                0x8000U
                | (Convert8To5(red) << 10)
                | (Convert8To5(green) << 5)
                | Convert8To5(blue));
        }

        return static_cast<uint16_t>(
            (Convert8To3(alpha) << 12)
            | (Convert8To4(red) << 8)
            | (Convert8To4(green) << 4)
            | Convert8To4(blue));
    }
}

namespace helengine::gamecube {
    /// Creates one empty GameCube runtime texture with no native texture memory yet.
    GameCubeRuntimeTexture::GameCubeRuntimeTexture()
        : RuntimeTexture()
        , NativeTextureData(nullptr)
        , NativeTextureDataSize(0U)
        , NativeTextureObject {}
        , NativeTextureObjectInitialized(false) {
    }

    /// Releases owned GameCube texture memory.
    GameCubeRuntimeTexture::~GameCubeRuntimeTexture() {
        ResetNativeTextureData();
    }

    /// Encodes one shared-engine texture asset into the first supported GameCube GX texture format.
    void GameCubeRuntimeTexture::LoadFromRaw(TextureAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        ResetNativeTextureData();

        if (data->ColorFormat == TextureAssetColorFormat::GxRgb5A3) {
            LoadPrepackedRgb5A3(data);
            return;
        } else if (data->ColorFormat != TextureAssetColorFormat::Rgba32) {
            throw new InvalidOperationException("GameCube runtime textures require either GxRgb5A3 or RGBA32 texture assets.");
        }

        EncodeRgba32ToRgb5A3(data);
    }

    /// Updates one RGBA8 logical pixel rectangle in the existing tiled GX RGB5A3 allocation.
    void GameCubeRuntimeTexture::UpdateRegionRgba8(int32_t x, int32_t y, int32_t width, int32_t height, Array<uint8_t>* rgba8, int32_t sourceRowPitch) {
        if (!NativeTextureObjectInitialized || NativeTextureData == nullptr || NativeTextureDataSize == 0U) {
            throw new InvalidOperationException("GameCube texture region updates require an initialized native texture.");
        } else if (rgba8 == nullptr) {
            throw new ArgumentNullException("rgba8");
        } else if (x < 0) {
            throw new ArgumentOutOfRangeException("x");
        } else if (y < 0) {
            throw new ArgumentOutOfRangeException("y");
        } else if (width <= 0) {
            throw new ArgumentOutOfRangeException("width");
        } else if (height <= 0) {
            throw new ArgumentOutOfRangeException("height");
        }

        const int32_t textureWidth = this->get_Width();
        const int32_t textureHeight = this->get_Height();
        if (textureWidth <= 0 || textureHeight <= 0) {
            throw new InvalidOperationException("GameCube texture region updates require nonzero texture dimensions.");
        }

        const int64_t endX = static_cast<int64_t>(x) + static_cast<int64_t>(width);
        const int64_t endY = static_cast<int64_t>(y) + static_cast<int64_t>(height);
        if (endX > textureWidth) {
            throw new ArgumentOutOfRangeException("width");
        } else if (endY > textureHeight) {
            throw new ArgumentOutOfRangeException("height");
        } else if (width > std::numeric_limits<int32_t>::max() / 4) {
            throw new ArgumentOutOfRangeException("width");
        }

        const std::size_t requiredRowBytes = static_cast<std::size_t>(width) * 4U;
        if (sourceRowPitch < static_cast<int32_t>(requiredRowBytes)) {
            throw new ArgumentOutOfRangeException("sourceRowPitch");
        } else if (sourceRowPitch % 4 != 0) {
            throw new ArgumentException("Source row pitch must be divisible by four for RGBA8 data.", "sourceRowPitch");
        }

        const std::size_t requiredSourceBytes = static_cast<std::size_t>(sourceRowPitch) * static_cast<std::size_t>(height - 1) + requiredRowBytes;
        if (requiredSourceBytes > static_cast<std::size_t>(rgba8->Length)) {
            throw new ArgumentException("Source buffer is shorter than the requested texture region.", "rgba8");
        }

        const uint32_t paddedWidth = (static_cast<uint32_t>(textureWidth) + 3U) & ~3U;
        const uint32_t paddedHeight = (static_cast<uint32_t>(textureHeight) + 3U) & ~3U;
        const uint32_t blockColumns = paddedWidth / 4U;
        const std::size_t expectedNativeTextureDataSize = static_cast<std::size_t>(paddedWidth) * static_cast<std::size_t>(paddedHeight) * sizeof(uint16_t);
        if (NativeTextureDataSize < expectedNativeTextureDataSize) {
            throw new InvalidOperationException("GameCube texture region updates require a complete tiled native texture allocation.");
        }

        auto* destination = static_cast<uint16_t*>(NativeTextureData);
        for (int32_t destinationY = y; destinationY < y + height; destinationY++) {
            const uint32_t blockY = static_cast<uint32_t>(destinationY) / 4U;
            const uint32_t innerY = static_cast<uint32_t>(destinationY) & 3U;
            const std::size_t sourceRow = static_cast<std::size_t>(destinationY - y);
            const std::size_t sourceRowOffset = static_cast<std::size_t>(sourceRow) * static_cast<std::size_t>(sourceRowPitch);
            for (int32_t destinationX = x; destinationX < x + width; destinationX++) {
                const uint32_t blockX = static_cast<uint32_t>(destinationX) / 4U;
                const uint32_t innerX = static_cast<uint32_t>(destinationX) & 3U;
                const std::size_t sourcePixelOffset = sourceRowOffset + static_cast<std::size_t>(destinationX - x) * 4U;
                const std::size_t destinationPixelIndex = (static_cast<std::size_t>(blockY) * static_cast<std::size_t>(blockColumns) + static_cast<std::size_t>(blockX)) * 16U + static_cast<std::size_t>(innerY) * 4U + static_cast<std::size_t>(innerX);
                destination[destinationPixelIndex] = EncodeRgb5A3Pixel(
                    (*rgba8)[static_cast<int32_t>(sourcePixelOffset + 0U)],
                    (*rgba8)[static_cast<int32_t>(sourcePixelOffset + 1U)],
                    (*rgba8)[static_cast<int32_t>(sourcePixelOffset + 2U)],
                    (*rgba8)[static_cast<int32_t>(sourcePixelOffset + 3U)]);
            }
        }

        const uint32_t firstBlockX = static_cast<uint32_t>(x) / 4U;
        const uint32_t lastBlockX = static_cast<uint32_t>(x + width - 1) / 4U;
        const uint32_t firstBlockY = static_cast<uint32_t>(y) / 4U;
        const uint32_t lastBlockY = static_cast<uint32_t>(y + height - 1) / 4U;
        const std::size_t blockRowByteCount = static_cast<std::size_t>(lastBlockX - firstBlockX + 1U) * 16U * sizeof(uint16_t);
        for (uint32_t blockY = firstBlockY; blockY <= lastBlockY; blockY++) {
            const std::size_t firstBlockPixelIndex = (static_cast<std::size_t>(blockY) * static_cast<std::size_t>(blockColumns) + static_cast<std::size_t>(firstBlockX)) * 16U;
            DCFlushRange(destination + firstBlockPixelIndex, static_cast<u32>(blockRowByteCount));
        }

        GX_InvalidateTexAll();
    }

    /// Returns whether a native GX texture object was initialized for this runtime texture.
    bool GameCubeRuntimeTexture::HasNativeTextureObject() const {
        return NativeTextureObjectInitialized;
    }

    /// Returns the native GX texture object used by textured material submission.
    GXTexObj* GameCubeRuntimeTexture::GetNativeTextureObject() {
        if (!NativeTextureObjectInitialized) {
            throw new InvalidOperationException("GameCube runtime texture does not own an initialized GX texture object.");
        }

        return &NativeTextureObject;
    }

    /// Releases any previously encoded native texture memory.
    void GameCubeRuntimeTexture::ResetNativeTextureData() {
        if (NativeTextureData != nullptr) {
            free(NativeTextureData);
            NativeTextureData = nullptr;
        }

        NativeTextureDataSize = 0U;
        std::memset(&NativeTextureObject, 0, sizeof(NativeTextureObject));
        NativeTextureObjectInitialized = false;
    }

    /// Loads one prepacked GX RGB5A3 payload that is already stored in native tiled texture memory order.
    void GameCubeRuntimeTexture::LoadPrepackedRgb5A3(TextureAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        } else if (data->Width == 0 || data->Height == 0) {
            throw new InvalidOperationException("GameCube runtime textures require nonzero dimensions.");
        } else if (data->Colors == nullptr) {
            throw new InvalidOperationException("GameCube runtime textures require prepacked RGB5A3 color data.");
        }

        const uint32_t width = data->Width;
        const uint32_t height = data->Height;
        const uint32_t paddedWidth = (width + 3U) & ~3U;
        const uint32_t paddedHeight = (height + 3U) & ~3U;
        const std::size_t expectedColorByteCount = static_cast<std::size_t>(paddedWidth) * static_cast<std::size_t>(paddedHeight) * 2U;
        if (data->Colors->Length != static_cast<int32_t>(expectedColorByteCount)) {
            throw new InvalidOperationException("GameCube prepacked textures must contain padded tiled RGB5A3 bytes.");
        }

        NativeTextureDataSize = expectedColorByteCount;
        NativeTextureData = memalign(32, NativeTextureDataSize);
        if (NativeTextureData == nullptr) {
            throw new InvalidOperationException("Could not allocate GameCube texture memory.");
        }

        std::memcpy(NativeTextureData, &(*data->Colors)[0], NativeTextureDataSize);
        DCFlushRange(NativeTextureData, NativeTextureDataSize);
        GX_InitTexObj(&NativeTextureObject, NativeTextureData, paddedWidth, paddedHeight, GX_TF_RGB5A3, GX_REPEAT, GX_REPEAT, GX_FALSE);
        GX_InitTexObjFilterMode(&NativeTextureObject, GX_LINEAR, GX_LINEAR);
        NativeTextureObjectInitialized = true;
        this->set_Width(static_cast<int32_t>(width));
        this->set_Height(static_cast<int32_t>(height));
    }

    /// Encodes one RGBA8 logical texture into tiled GX RGB5A3 memory.
    void GameCubeRuntimeTexture::EncodeRgba32ToRgb5A3(TextureAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        } else if (data->Width == 0 || data->Height == 0) {
            throw new InvalidOperationException("GameCube runtime textures require nonzero dimensions.");
        } else if (data->Colors == nullptr) {
            throw new InvalidOperationException("GameCube runtime textures require RGBA32 color data.");
        }

        const uint32_t width = data->Width;
        const uint32_t height = data->Height;
        const std::size_t expectedColorByteCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U;
        if (data->Colors->Length != static_cast<int32_t>(expectedColorByteCount)) {
            throw new InvalidOperationException("GameCube runtime textures require tightly packed RGBA32 color bytes.");
        }

        const uint32_t paddedWidth = (width + 3U) & ~3U;
        const uint32_t paddedHeight = (height + 3U) & ~3U;
        NativeTextureDataSize = static_cast<std::size_t>(paddedWidth) * static_cast<std::size_t>(paddedHeight) * 2U;
        NativeTextureData = memalign(32, NativeTextureDataSize);
        if (NativeTextureData == nullptr) {
            throw new InvalidOperationException("Could not allocate GameCube texture memory.");
        }

        std::memset(NativeTextureData, 0, NativeTextureDataSize);
        auto* destination = static_cast<uint16_t*>(NativeTextureData);

        for (uint32_t blockY = 0; blockY < paddedHeight; blockY += 4U) {
            for (uint32_t blockX = 0; blockX < paddedWidth; blockX += 4U) {
                for (uint32_t innerY = 0; innerY < 4U; innerY++) {
                    for (uint32_t innerX = 0; innerX < 4U; innerX++) {
                        const uint32_t sampleX = std::min(blockX + innerX, width - 1U);
                        const uint32_t sampleY = std::min(blockY + innerY, height - 1U);
                        const std::size_t sourceOffset = (static_cast<std::size_t>(sampleY) * static_cast<std::size_t>(width) + sampleX) * 4U;
                        const uint8_t red = (*data->Colors)[static_cast<int32_t>(sourceOffset + 0U)];
                        const uint8_t green = (*data->Colors)[static_cast<int32_t>(sourceOffset + 1U)];
                        const uint8_t blue = (*data->Colors)[static_cast<int32_t>(sourceOffset + 2U)];
                        const uint8_t alpha = (*data->Colors)[static_cast<int32_t>(sourceOffset + 3U)];
                        *destination++ = EncodeRgb5A3Pixel(red, green, blue, alpha);
                    }
                }
            }
        }

        DCFlushRange(NativeTextureData, NativeTextureDataSize);
        GX_InitTexObj(&NativeTextureObject, NativeTextureData, paddedWidth, paddedHeight, GX_TF_RGB5A3, GX_REPEAT, GX_REPEAT, GX_FALSE);
        GX_InitTexObjFilterMode(&NativeTextureObject, GX_LINEAR, GX_LINEAR);
        NativeTextureObjectInitialized = true;
        this->set_Width(static_cast<int32_t>(width));
        this->set_Height(static_cast<int32_t>(height));
    }
}
