#include "platform/gamecube/GameCubeRuntimeTexture.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <malloc.h>
#include <ogc/cache.h>
#include <ogc/system.h>

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

        SYS_Report(
            "[GC] GCRT load raw begin id=%s format=%d size=%ux%u colors=%d palette=%d\n",
            data->get_Id().c_str(),
            static_cast<int>(data->ColorFormat),
            static_cast<unsigned>(data->Width),
            static_cast<unsigned>(data->Height),
            data->Colors != nullptr ? data->Colors->get_Length() : -1,
            data->PaletteColors != nullptr ? data->PaletteColors->get_Length() : -1);
        ResetNativeTextureData();

        if (data->ColorFormat == TextureAssetColorFormat::GxRgb5A3) {
            SYS_Report("[GC] GCRT load raw branch=prepacked\n");
            LoadPrepackedRgb5A3(data);
            SYS_Report("[GC] GCRT load raw complete id=%s width=%d height=%d\n", data->get_Id().c_str(), this->get_Width(), this->get_Height());
            return;
        } else if (data->ColorFormat != TextureAssetColorFormat::Rgba32) {
            throw new InvalidOperationException("GameCube runtime textures require either GxRgb5A3 or RGBA32 texture assets.");
        }

        SYS_Report("[GC] GCRT load raw branch=encode-rgba32\n");
        EncodeRgba32ToRgb5A3(data);
        SYS_Report("[GC] GCRT load raw complete id=%s width=%d height=%d\n", data->get_Id().c_str(), this->get_Width(), this->get_Height());
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

        SYS_Report(
            "[GC] GCRT prepacked begin id=%s width=%u height=%u expectedBytes=%u\n",
            data->get_Id().c_str(),
            width,
            height,
            static_cast<unsigned>(expectedColorByteCount));
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
        SYS_Report("[GC] GCRT prepacked complete id=%s ptr=%p bytes=%u\n", data->get_Id().c_str(), NativeTextureData, static_cast<unsigned>(NativeTextureDataSize));
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
        SYS_Report(
            "[GC] GCRT encode begin id=%s width=%u height=%u padded=%ux%u bytes=%u\n",
            data->get_Id().c_str(),
            width,
            height,
            paddedWidth,
            paddedHeight,
            static_cast<unsigned>(NativeTextureDataSize));
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
        SYS_Report("[GC] GCRT encode complete id=%s ptr=%p bytes=%u\n", data->get_Id().c_str(), NativeTextureData, static_cast<unsigned>(NativeTextureDataSize));
    }
}
