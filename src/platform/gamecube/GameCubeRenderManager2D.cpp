#include "platform/gamecube/GameCubeRenderManager2D.hpp"

#include "RuntimeTexture.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube 2D render bridge.
    GameCubeRenderManager2D::GameCubeRenderManager2D()
        : RenderManager2D() {
    }

    /// Builds a placeholder runtime texture from generated texture asset metadata.
    RuntimeTexture* GameCubeRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
        (void)data;
        return new RuntimeTexture();
    }

    /// Accepts a sprite draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        (void)sprite;
    }

    /// Accepts a text draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawText(ITextDrawable2D* text) {
        (void)text;
    }

    /// Accepts a rounded-rectangle draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        (void)shape;
    }
}
