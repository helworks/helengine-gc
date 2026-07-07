#pragma once

#include <vector>

#include "IRenderVisitor2D.hpp"
#include "RenderManager2D.hpp"

class RuntimeTexture;
class IDrawable2D;

namespace helengine::gamecube {
    /// Stores one captured sprite draw request for the current GameCube 2D frame.
    struct GameCubeSpriteDrawCommand {
        /// Pointer to the shared-engine sprite drawable submitted during the current frame.
        ISpriteDrawable2D* Drawable;
    };

    /// Stores one captured text draw request for the current GameCube 2D frame.
    struct GameCubeTextDrawCommand {
        /// Pointer to the shared-engine text drawable submitted during the current frame.
        ITextDrawable2D* Drawable;
    };

    /// Stores one captured rounded-rectangle draw request for the current GameCube 2D frame.
    struct GameCubeRoundedRectDrawCommand {
        /// Pointer to the shared-engine rounded-rectangle drawable submitted during the current frame.
        IRoundedRectDrawable2D* Drawable;
    };

    /// Implements the GameCube 2D render bridge by capturing shared-engine draw requests for one GX overlay pass.
    class GameCubeRenderManager2D : public RenderManager2D, public IRenderVisitor2D {
    public:
        /// Creates the GameCube 2D render bridge.
        GameCubeRenderManager2D();

        /// Builds one GameCube-native runtime texture from generated texture asset metadata.
        RuntimeTexture* BuildTextureFromRaw(TextureAsset* data) override;

        /// Builds one GameCube-native runtime texture from one packaged cooked texture payload.
        RuntimeTexture* BuildTextureFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource) override;

        /// Releases one GameCube runtime texture previously created for one packaged scene asset.
        void ReleaseTexture(RuntimeTexture* texture) override;

        /// Releases one GameCube font asset together with its owned runtime texture and source texture payload.
        void ReleaseFont(FontAsset* font) override;

        /// Accepts a sprite draw request without issuing native rendering yet.
        void DrawSprite(ISpriteDrawable2D* sprite) override;

        /// Accepts a text draw request without issuing native rendering yet.
        void DrawText(ITextDrawable2D* text) override;

        /// Accepts a rounded-rectangle draw request without issuing native rendering yet.
        void DrawRoundedRect(IRoundedRectDrawable2D* shape) override;

        /// Walks the active camera 2D queue and lets each drawable submit itself into this frame capture.
        void Draw() override;

        /// Visits one ordered 2D drawable from the active camera queue.
        void Visit(IDrawable2D* drawable) override;

        /// Releases any deferred runtime-texture deletions after the scene manager reaches a safe scene-transition boundary.
        void FlushReleasedTextures() override;

        /// Clears previously captured 2D draw requests before the next engine frame begins.
        void BeginFrame();

        /// Returns whether the current frame captured any 2D draw requests.
        bool HasCapturedDrawables() const;

        /// Returns the captured sprite draw requests for the current frame.
        const std::vector<GameCubeSpriteDrawCommand>& GetSpriteQueue() const;

        /// Returns the captured text draw requests for the current frame.
        const std::vector<GameCubeTextDrawCommand>& GetTextQueue() const;

        /// Returns the captured rounded-rectangle draw requests for the current frame.
        const std::vector<GameCubeRoundedRectDrawCommand>& GetRoundedRectQueue() const;

    private:
        /// Captured sprite draw requests in shared-engine render order.
        std::vector<GameCubeSpriteDrawCommand> SpriteQueue;

        /// Captured text draw requests in shared-engine render order.
        std::vector<GameCubeTextDrawCommand> TextQueue;

        /// Captured rounded-rectangle draw requests in shared-engine render order.
        std::vector<GameCubeRoundedRectDrawCommand> RoundedRectQueue;

        /// Runtime textures deferred until the renderer reaches a safe destruction boundary.
        std::vector<RuntimeTexture*> ReleasedTextures;
    };
}
