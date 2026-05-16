#include "platform/gamecube/GameCubeRenderManager2D.hpp"

#include "RuntimeTexture.hpp"
#include "TextureAsset.hpp"
#include "platform/gamecube/GameCubeRuntimeTexture.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube 2D render bridge.
    GameCubeRenderManager2D::GameCubeRenderManager2D()
        : RenderManager2D() {
    }

    /// Builds one GameCube-native runtime texture from generated texture asset metadata.
    RuntimeTexture* GameCubeRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        GameCubeRuntimeTexture* runtimeTexture = new GameCubeRuntimeTexture();
        runtimeTexture->set_Id(data->get_Id());
        runtimeTexture->LoadFromRaw(data);
        return runtimeTexture;
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
