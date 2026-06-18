#include "platform/gamecube/GameCubeRenderManager2D.hpp"

#include "Asset.hpp"
#include "CameraComponent.hpp"
#include "Core.hpp"
#include "EditorAssetBinarySerializer.hpp"
#include "Entity.hpp"
#include "FontAsset.hpp"
#include "FontInfo.hpp"
#include "ICamera.hpp"
#include "IDrawable2D.hpp"
#include "IRenderQueue2D.hpp"
#include "ObjectManager.hpp"
#include "RuntimeTexture.hpp"
#include "TextureAsset.hpp"
#include "platform/gamecube/GameCubeRuntimeTexture.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/file.hpp"

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

    /// Builds one GameCube-native runtime texture from one packaged cooked texture asset path.
    RuntimeTexture* GameCubeRenderManager2D::BuildTextureFromCooked(std::string cookedAssetPath) {
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked texture path is required.", "cookedAssetPath");
        }

        ::FileStream* textureStream = ::File::OpenRead(cookedAssetPath);
        try {
            ::Asset* textureAssetPayload = ::EditorAssetBinarySerializer::Deserialize(textureStream);
            ::TextureAsset* textureAsset = he_cpp_try_cast<::TextureAsset>(textureAssetPayload);
            if (textureAsset == nullptr) {
                throw new InvalidOperationException("GameCube cooked texture payload did not deserialize as TextureAsset.");
            }

            textureStream->Dispose();
            RuntimeTexture* runtimeTexture = BuildTextureFromRaw(textureAsset);
            if (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {
                delete textureAsset->Colors;
                textureAsset->Colors = Array<uint8_t>::Empty();
            }

            if (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {
                delete textureAsset->PaletteColors;
                textureAsset->PaletteColors = Array<uint8_t>::Empty();
            }

            delete textureAsset;
            return runtimeTexture;
        } catch (...) {
            if (textureStream != nullptr) {
                textureStream->Dispose();
            }

            throw;
        }
    }

    /// Releases one GameCube runtime texture previously created for one packaged scene asset.
    void GameCubeRenderManager2D::ReleaseTexture(RuntimeTexture* texture) {
        if (texture == nullptr) {
            throw new ArgumentNullException("texture");
        } else if (texture->get_IsEngineOwned()) {
            return;
        }

        ReleasedTextures.push_back(texture);
    }

    /// Releases one GameCube font asset together with its owned runtime texture and source texture payload.
    void GameCubeRenderManager2D::ReleaseFont(FontAsset* font) {
        if (font == nullptr) {
            throw new ArgumentNullException("font");
        }

        RuntimeTexture* texture = font->get_Texture();
        if (texture != nullptr && !texture->get_IsDisposed()) {
            texture->Dispose();
            delete static_cast<GameCubeRuntimeTexture*>(texture);
        }

        font->Dispose();
        delete font;
    }

    /// Walks the active camera 2D queue and lets each drawable submit itself into this frame capture.
    void GameCubeRenderManager2D::Draw() {
        Core* core = Core::get_Instance();
        if (core == nullptr || core->get_ObjectManager() == nullptr) {
            return;
        }

        List<ICamera*>* cameras = core->get_ObjectManager()->get_Cameras();
        for (int32_t cameraIndex = 0; cameraIndex < cameras->get_Count(); cameraIndex++) {
            CameraComponent* camera = he_cpp_try_cast<CameraComponent>((*cameras)[cameraIndex]);
            if (camera == nullptr || camera->get_Parent() == nullptr || !camera->get_Parent()->get_IsHierarchyEnabled()) {
                continue;
            }

            camera->get_RenderQueue2D()->VisitOrdered(this);
        }
    }

    /// Visits one ordered 2D drawable from the active camera queue.
    void GameCubeRenderManager2D::Visit(IDrawable2D* drawable) {
        if (drawable == nullptr || drawable->get_Parent() == nullptr || !drawable->get_Parent()->get_IsHierarchyEnabled()) {
            return;
        }

        drawable->Draw();
    }

    /// Releases deferred runtime textures after the scene manager reaches a safe transition boundary.
    void GameCubeRenderManager2D::FlushReleasedTextures() {
        for (RuntimeTexture* texture : ReleasedTextures) {
            if (texture == nullptr) {
                continue;
            }

            delete static_cast<GameCubeRuntimeTexture*>(texture);
        }

        ReleasedTextures.clear();
    }

    /// Clears previously captured 2D draw requests before the next engine frame begins.
    void GameCubeRenderManager2D::BeginFrame() {
        SpriteQueue.clear();
        TextQueue.clear();
        RoundedRectQueue.clear();
    }

    /// Returns whether the current frame captured any 2D draw requests.
    bool GameCubeRenderManager2D::HasCapturedDrawables() const {
        return !SpriteQueue.empty() || !TextQueue.empty() || !RoundedRectQueue.empty();
    }

    /// Returns the captured sprite draw requests for the current frame.
    const std::vector<GameCubeSpriteDrawCommand>& GameCubeRenderManager2D::GetSpriteQueue() const {
        return SpriteQueue;
    }

    /// Returns the captured text draw requests for the current frame.
    const std::vector<GameCubeTextDrawCommand>& GameCubeRenderManager2D::GetTextQueue() const {
        return TextQueue;
    }

    /// Returns the captured rounded-rectangle draw requests for the current frame.
    const std::vector<GameCubeRoundedRectDrawCommand>& GameCubeRenderManager2D::GetRoundedRectQueue() const {
        return RoundedRectQueue;
    }

    /// Accepts a sprite draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
        if (sprite == nullptr) {
            throw new ArgumentNullException("sprite");
        }

        SpriteQueue.push_back(GameCubeSpriteDrawCommand { sprite });
    }

    /// Accepts a text draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawText(ITextDrawable2D* text) {
        if (text == nullptr) {
            throw new ArgumentNullException("text");
        }

        TextQueue.push_back(GameCubeTextDrawCommand { text });
    }

    /// Accepts a rounded-rectangle draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        if (shape == nullptr) {
            throw new ArgumentNullException("shape");
        }

        RoundedRectQueue.push_back(GameCubeRoundedRectDrawCommand { shape });
    }
}
