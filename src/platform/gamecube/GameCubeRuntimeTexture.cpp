#include "platform/gamecube/GameCubeRuntimeTexture.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <malloc.h>
#include <ogc/cache.h>
#include "TextureAsset.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetPixelCodec.hpp"
#include "runtime/native_exceptions.hpp"

namespace {
    /// Stable regions owned by this renderer, with nonoverlapping small and full fourteen-bit palette slots.
    static GXTlutRegion NativeTlutRegions[12] {};
    /// Callback retained for names not assigned by this renderer; initialization is global rather than tied to one texture lifetime.
    static GXTlutRegionCallback PreviousTlutCallback = nullptr;
    /// Records whether GX's existing callback was captured after GX initialization.
    static bool NativeTlutCallbackInstalled = false;

    /// Returns the same region object for a given renderer slot, preserving libogc's TLUT-name contract.
    static GXTlutRegion* ResolveNativeTlutRegion(u32 name) {
        if (name < 8) return &NativeTlutRegions[name];
        if (name >= 16 && name < 20) return &NativeTlutRegions[name - 8];
        return PreviousTlutCallback == nullptr ? nullptr : PreviousTlutCallback(name);
    }

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
        , NativeTextureObjectInitialized(false)
        , NativePaletteData(nullptr)
        , NativePaletteObject {}
        , NativeLayout {}
        , UsesNativePayload(false) {
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

        if (data->ColorFormat == TextureAssetColorFormat::GxNative) {
            LoadNativePayload(data);
            return;
        } else if (data->ColorFormat == TextureAssetColorFormat::GxRgb5A3) {
            LoadPrepackedRgb5A3(data);
            return;
        } else if (data->ColorFormat != TextureAssetColorFormat::Rgba32) {
            // Generic formats use the shared decoder; native GXN1 data never takes this expansion route.
            Array<uint8_t>* decoded = TextureAssetPixelCodec::DecodeToRgba32(data);
            try {
                TextureAsset rgba;
                rgba.Width = data->Width; rgba.Height = data->Height;
                rgba.Colors = decoded;
                EncodeRgba32ToRgb5A3(&rgba);
                delete decoded;
                return;
            } catch (...) {
                delete decoded;
                throw;
            }
        }

        EncodeRgba32ToRgb5A3(data);
    }

    /// Re-encodes one RGBA8 source rectangle into the owned tiled GX RGB5A3 texture memory and republishes it to the graphics processor.
    void GameCubeRuntimeTexture::UpdateRegionFromRgba8(int32_t x, int32_t y, int32_t width, int32_t height, Array<uint8_t>* rgba8, int32_t sourceRowPitch) {
        if (rgba8 == nullptr || rgba8->Data == nullptr) {
            throw new ArgumentNullException("rgba8");
        } else if (!NativeTextureObjectInitialized || NativeTextureData == nullptr) {
            throw new InvalidOperationException("GameCube texture region updates require initialized native texture memory.");
        } else if (x < 0 || y < 0 || width <= 0 || height <= 0) {
            throw new ArgumentOutOfRangeException("texture region");
        }


        const int32_t textureWidth = this->get_Width();
        const int32_t textureHeight = this->get_Height();
        if (textureWidth <= 0
            || textureHeight <= 0
            || static_cast<int64_t>(x) + width > textureWidth
            || static_cast<int64_t>(y) + height > textureHeight) {
            throw new ArgumentOutOfRangeException("texture region", "Texture region exceeds the destination texture bounds.");
        }

        if (width > (INT32_MAX / 4)) {
            throw new ArgumentOutOfRangeException("width", "Texture region row size is too large.");
        }

        const int32_t requiredRowBytes = width * 4;
        if (sourceRowPitch < requiredRowBytes || (sourceRowPitch % 4) != 0) {
            throw new ArgumentOutOfRangeException("sourceRowPitch", "Source row pitch is smaller than the requested RGBA8 row.");
        }

        const int64_t requiredSourceBytes = static_cast<int64_t>(sourceRowPitch) * (height - 1) + requiredRowBytes;
        if (requiredSourceBytes > rgba8->Length) {
            throw new InvalidOperationException("Source buffer is shorter than the requested GameCube texture region.");
        }

        if (UsesNativePayload) {
            if (!PatchNativeTextureRegion(NativeLayout, static_cast<std::uint8_t*>(NativeTextureData), NativeTextureDataSize,
                x, y, width, height, rgba8->Data, rgba8->Length, sourceRowPitch)) {
                throw new InvalidOperationException("GameCube RGBA updates cannot alter immutable indexed, CMPR or depth-view native textures.");
            }
            DCFlushRange(NativeTextureData, NativeTextureDataSize);
            GX_InvalidateTexAll();
            return;
        }

        const uint32_t paddedWidth = (static_cast<uint32_t>(textureWidth) + 3U) & ~3U;
        const uint32_t paddedHeight = (static_cast<uint32_t>(textureHeight) + 3U) & ~3U;
        const uint32_t blockColumnCount = paddedWidth / 4U;
        const uint32_t regionLeft = static_cast<uint32_t>(x);
        const uint32_t regionTop = static_cast<uint32_t>(y);
        const uint32_t regionRight = regionLeft + static_cast<uint32_t>(width);
        const uint32_t regionBottom = regionTop + static_cast<uint32_t>(height);

        // Padded texels replicate the clamped edge texel exactly as the full-texture encoder does, so a region that
        // reaches the logical right or bottom edge must also rewrite the padding columns and rows that mirror it.
        const uint32_t firstColumn = regionLeft & ~3U;
        const uint32_t columnEnd = (regionRight >= static_cast<uint32_t>(textureWidth)) ? paddedWidth : ((regionRight + 3U) & ~3U);
        const uint32_t firstRow = regionTop & ~3U;
        const uint32_t rowEnd = (regionBottom >= static_cast<uint32_t>(textureHeight)) ? paddedHeight : ((regionBottom + 3U) & ~3U);

        auto* destination = static_cast<uint16_t*>(NativeTextureData);
        for (uint32_t destinationY = firstRow; destinationY < rowEnd; destinationY++) {
            const uint32_t sampleY = std::min(destinationY, static_cast<uint32_t>(textureHeight) - 1U);
            if (sampleY < regionTop || sampleY >= regionBottom) {
                continue;
            }

            const std::size_t sourceRowOffset = static_cast<std::size_t>(sampleY - regionTop) * static_cast<std::size_t>(sourceRowPitch);
            for (uint32_t destinationX = firstColumn; destinationX < columnEnd; destinationX++) {
                const uint32_t sampleX = std::min(destinationX, static_cast<uint32_t>(textureWidth) - 1U);
                if (sampleX < regionLeft || sampleX >= regionRight) {
                    continue;
                }

                const std::size_t sourceOffset = sourceRowOffset + static_cast<std::size_t>(sampleX - regionLeft) * 4U;
                const uint8_t red = (*rgba8)[static_cast<int32_t>(sourceOffset + 0U)];
                const uint8_t green = (*rgba8)[static_cast<int32_t>(sourceOffset + 1U)];
                const uint8_t blue = (*rgba8)[static_cast<int32_t>(sourceOffset + 2U)];
                const uint8_t alpha = (*rgba8)[static_cast<int32_t>(sourceOffset + 3U)];
                const std::size_t blockIndex = static_cast<std::size_t>(destinationY >> 2) * static_cast<std::size_t>(blockColumnCount)
                    + static_cast<std::size_t>(destinationX >> 2);
                const std::size_t texelIndex = blockIndex * 16U
                    + static_cast<std::size_t>(destinationY & 3U) * 4U
                    + static_cast<std::size_t>(destinationX & 3U);
                destination[texelIndex] = EncodeRgb5A3Pixel(red, green, blue, alpha);
            }
        }

        // Whole four-row texel blocks are contiguous, so only the touched block rows need to reach main memory.
        const std::size_t blockRowByteCount = static_cast<std::size_t>(blockColumnCount) * 32U;
        const std::size_t flushOffset = static_cast<std::size_t>(firstRow >> 2) * blockRowByteCount;
        const std::size_t flushByteCount = static_cast<std::size_t>((rowEnd - firstRow) >> 2) * blockRowByteCount;
        DCFlushRange(static_cast<uint8_t*>(NativeTextureData) + flushOffset, flushByteCount);
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

    void GameCubeRuntimeTexture::BindNativeTexture(std::uint8_t map) {
        if (!NativeTextureObjectInitialized || map >= 8) {
            throw new InvalidOperationException("GameCube texture binding requires an initialized texture and a valid map.");
        }
        if (NativePaletteData != nullptr) {
            GameCubeNativeTlutBinding binding;
            if (!TryBuildNativeTlutBinding(NativeLayout, map, binding)) {
                throw new InvalidOperationException("GameCube CI14 palettes require one of the four independent large TLUT map slots.");
            }
            if (!NativeTlutCallbackInstalled) {
                PreviousTlutCallback = GX_SetTlutRegionCallback(ResolveNativeTlutRegion);
                NativeTlutCallbackInstalled = true;
            }
            GXTlutRegion* region = ResolveNativeTlutRegion(binding.Name);
            GX_InitTlutRegion(region, binding.TmemAddress, GX_TLUT_16);
            // libogc df5c999 gx.c takes a u8 size and masks ten count bits. The hardware BP 0x65
            // count is eleven bits (Dolphin BPMemory.h BPU_LoadTlutInfo), so 16,384 entries need
            // this complete precompiled register word. Count reflects actual bytes, not slot capacity.
            region->val[0] = binding.LoadRegister;
            GX_InitTexObjTlut(&NativeTextureObject, binding.Name);
            GX_LoadTlut(&NativePaletteObject, binding.Name);
        }
        GX_LoadTexObj(&NativeTextureObject, map);
    }

    /// Releases any previously encoded native texture memory.
    void GameCubeRuntimeTexture::ResetNativeTextureData() {
        if (NativeTextureData != nullptr) {
            free(NativeTextureData);
            NativeTextureData = nullptr;
        }

        NativeTextureDataSize = 0U;
        free(NativePaletteData);
        NativePaletteData = nullptr;
        NativePaletteObject = {};
        NativeLayout = {};
        UsesNativePayload = false;
        std::memset(&NativeTextureObject, 0, sizeof(NativeTextureObject));
        NativeTextureObjectInitialized = false;
    }

    void GameCubeRuntimeTexture::LoadNativePayload(TextureAsset* data) {
        GameCubeNativeTextureLayout layout;
        if (data->Colors == nullptr || data->Colors->Length < 0
            || (data->PaletteColors != nullptr && data->PaletteColors->Length < 0)
            || !TryParseNativeTexturePayload(data->Colors->Data, static_cast<std::size_t>(data->Colors->Length),
                data->PaletteColors == nullptr ? nullptr : data->PaletteColors->Data,
                data->PaletteColors == nullptr ? 0 : static_cast<std::size_t>(data->PaletteColors->Length), layout)
            || layout.Width != data->Width || layout.Height != data->Height
            || layout.AlphaPrecision != static_cast<std::uint32_t>(data->AlphaPrecision)) {
            throw new InvalidOperationException("GameCube GXN1 header, metadata, tiled bytes or TLUT indices are invalid.");
        }
        NativeTextureData = memalign(32, layout.PixelBytes);
        if (NativeTextureData == nullptr) throw new InvalidOperationException("Could not allocate native GameCube texture memory.");
        if (layout.PaletteBytes != 0) {
            NativePaletteData = memalign(32, layout.PaletteBytes);
            if (NativePaletteData == nullptr) {
                ResetNativeTextureData();
                throw new InvalidOperationException("Could not allocate native GameCube TLUT memory.");
            }
            std::memcpy(NativePaletteData, data->PaletteColors->Data, layout.PaletteBytes);
            DCFlushRange(NativePaletteData, layout.PaletteBytes);
            GX_InitTlutObj(&NativePaletteObject, NativePaletteData, layout.TlutFormat, layout.PaletteEntries);
            GX_InitTexObjCI(&NativeTextureObject, NativeTextureData, layout.Width, layout.Height, layout.SamplerFormat,
                GX_CLAMP, GX_CLAMP, GX_FALSE, layout.SamplerFormat == 10 ? GX_BIGTLUT0 : GX_TLUT0);
        } else {
            GX_InitTexObj(&NativeTextureObject, NativeTextureData, layout.Width, layout.Height, layout.SamplerFormat,
                GX_CLAMP, GX_CLAMP, GX_FALSE);
        }
        std::memcpy(NativeTextureData, data->Colors->Data + 48, layout.PixelBytes);
        DCFlushRange(NativeTextureData, layout.PixelBytes);
        GX_InitTexObjFilterMode(&NativeTextureObject, GX_LINEAR, GX_LINEAR);
        NativeTextureDataSize = layout.PixelBytes;
        NativeLayout = layout;
        UsesNativePayload = true;
        NativeTextureObjectInitialized = true;
        this->set_Width(layout.Width); this->set_Height(layout.Height);
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
