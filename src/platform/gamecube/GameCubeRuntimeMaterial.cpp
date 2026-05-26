#include "platform/gamecube/GameCubeRuntimeMaterial.hpp"

#include <utility>

namespace helengine::gamecube {
    /// Creates one GameCube runtime material with authored-color defaults that remain visible when cooking is incomplete.
    GameCubeRuntimeMaterial::GameCubeRuntimeMaterial()
        : BaseColorValue(1.0f, 1.0f, 1.0f)
        , TextureRelativePathValue()
        , OwnedDiffuseTextureValue(nullptr) {
    }

    /// Gets the normalized authored base RGB color resolved from the cooked platform material.
    float3 GameCubeRuntimeMaterial::GetBaseColor() const {
        return BaseColorValue;
    }

    /// Replaces the normalized authored base RGB color used by the lit GX submission path.
    void GameCubeRuntimeMaterial::SetBaseColor(float3 value) {
        BaseColorValue = value;
    }

    /// Gets the cooked diffuse-texture path resolved from the packaged platform material payload.
    const std::string& GameCubeRuntimeMaterial::GetTextureRelativePath() const {
        return TextureRelativePathValue;
    }

    /// Replaces the cooked diffuse-texture path used to validate authored textured submissions.
    void GameCubeRuntimeMaterial::SetTextureRelativePath(std::string value) {
        TextureRelativePathValue = std::move(value);
    }

    /// Gets the runtime diffuse texture owned directly by this material when the path-based cooked-material contract loads one internally.
    RuntimeTexture* GameCubeRuntimeMaterial::GetOwnedDiffuseTexture() const {
        return OwnedDiffuseTextureValue;
    }

    /// Replaces the runtime diffuse texture owned directly by this material when the path-based cooked-material contract loads one internally.
    void GameCubeRuntimeMaterial::SetOwnedDiffuseTexture(RuntimeTexture* value) {
        OwnedDiffuseTextureValue = value;
    }
}
