#pragma once

#include "SpriteQuad2D.hpp"
#include <cstddef>
#include <vector>

#include "IRenderVisitor2D.hpp"
#include "RenderManager2D.hpp"

class RuntimeTexture;
class IDrawable2D;
class CameraComponent;

namespace helengine::gamecube {
    /// Identifies which primitive-specific queue owns one globally ordered GameCube 2D draw command.
    enum class GameCubeDrawCommandType2D {
        Sprite,
        Text,
        RoundedRect
    };

    /// Preserves one shared-engine 2D submission's position while primitive-specific queues retain strongly typed drawable pointers.
    struct GameCubeDrawCommand2D {
        /// Camera whose ordered queue submitted this drawable during the current frame.
        CameraComponent* Camera;

        /// Primitive queue that owns the strongly typed command referenced by this entry.
        GameCubeDrawCommandType2D Type;

        /// Index of the strongly typed command inside the queue selected by Type.
        std::size_t TypeQueueIndex;
    };

    /// Stores one captured sprite draw request for the current GameCube 2D frame.
    struct GameCubeSpriteDrawCommand {
        /// Camera whose ordered queue submitted this sprite during the current frame.
        CameraComponent* Camera;

        /// Pointer to the shared-engine sprite drawable submitted during the current frame.
        ISpriteDrawable2D* Drawable;

        /// Exact logical corners resolved when the sprite is submitted.
        SpriteQuad2D Geometry;
    };

    /// Stores one captured text draw request for the current GameCube 2D frame.
    struct GameCubeTextDrawCommand {
        /// Camera whose ordered queue submitted this text drawable during the current frame.
        CameraComponent* Camera;

        /// Pointer to the shared-engine text drawable submitted during the current frame.
        ITextDrawable2D* Drawable;
    };

    /// Stores one captured rounded-rectangle draw request for the current GameCube 2D frame.
    struct GameCubeRoundedRectDrawCommand {
        /// Camera whose ordered queue submitted this rounded rectangle during the current frame.
        CameraComponent* Camera;

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

        /// Returns whether one camera submitted any 2D draw requests during the current frame.
        bool HasCapturedDrawablesForCamera(CameraComponent* camera) const;

        /// Returns every captured 2D draw request in the exact order supplied by the shared engine render queue.
        const std::vector<GameCubeDrawCommand2D>& GetDrawQueue() const;

        /// Returns the captured sprite draw requests for the current frame.
        const std::vector<GameCubeSpriteDrawCommand>& GetSpriteQueue() const;

        /// Returns the captured text draw requests for the current frame.
        const std::vector<GameCubeTextDrawCommand>& GetTextQueue() const;

        /// Returns the captured rounded-rectangle draw requests for the current frame.
        const std::vector<GameCubeRoundedRectDrawCommand>& GetRoundedRectQueue() const;

    protected:
        /// Uploads one validated RGBA8 rectangle into one GameCube-owned runtime texture.
        void UpdateTextureRegionCore(RuntimeTexture* texture, int32_t x, int32_t y, int32_t width, int32_t height, Array<uint8_t>* rgba8, int32_t sourceRowPitch) override;

    private:
        /// Captured mixed-primitive draw requests in shared-engine render order.
        std::vector<GameCubeDrawCommand2D> DrawQueue;

        /// Captured sprite draw requests in shared-engine render order.
        std::vector<GameCubeSpriteDrawCommand> SpriteQueue;

        /// Captured text draw requests in shared-engine render order.
        std::vector<GameCubeTextDrawCommand> TextQueue;

        /// Captured rounded-rectangle draw requests in shared-engine render order.
        std::vector<GameCubeRoundedRectDrawCommand> RoundedRectQueue;

        /// Camera whose render queue is currently submitting commands into this frame capture.
        CameraComponent* ActiveCaptureCamera;

        /// Runtime textures deferred until the renderer reaches a safe destruction boundary.
        std::vector<RuntimeTexture*> ReleasedTextures;
    };
}
