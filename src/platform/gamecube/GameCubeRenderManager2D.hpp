#pragma once

#include "RenderManager2D.hpp"

class RuntimeTexture;

namespace helengine::gamecube {
    /// Implements the minimal 2D render bridge needed to let the generated core initialize on GameCube.
    class GameCubeRenderManager2D : public RenderManager2D {
    public:
        /// Creates the GameCube 2D render bridge.
        GameCubeRenderManager2D();

        /// Builds one GameCube-native runtime texture from generated texture asset metadata.
        RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override;

        /// Accepts a sprite draw request without issuing native rendering yet.
        void DrawSprite(ISpriteDrawable2D* sprite) override;

        /// Accepts a text draw request without issuing native rendering yet.
        void DrawText(ITextDrawable2D* text) override;

        /// Accepts a rounded-rectangle draw request without issuing native rendering yet.
        void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;
    };
}
