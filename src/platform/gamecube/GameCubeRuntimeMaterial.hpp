#pragma once

#include <string>

#include "RuntimeMaterial.hpp"
#include "float3.hpp"

namespace helengine::gamecube {
    /// Carries the minimal GameCube-owned material state required by the fixed-function GX raster path.
    class GameCubeRuntimeMaterial final : public RuntimeMaterial {
    public:
        /// Creates one GameCube runtime material with authored-color defaults that remain visible when cooking is incomplete.
        GameCubeRuntimeMaterial();

        /// Gets the normalized authored base RGB color resolved from the cooked platform material.
        float3 GetBaseColor() const;

        /// Replaces the normalized authored base RGB color used by the lit GX submission path.
        void SetBaseColor(float3 value);

        /// Gets the cooked diffuse-texture path resolved from the packaged platform material payload.
        const std::string& GetTextureRelativePath() const;

        /// Replaces the cooked diffuse-texture path used to validate authored textured submissions.
        void SetTextureRelativePath(std::string value);

    private:
        /// Normalized authored base RGB color copied from the cooked platform material payload.
        float3 BaseColorValue;

        /// Cooked diffuse-texture path copied from the packaged platform material payload.
        std::string TextureRelativePathValue;
    };
}
