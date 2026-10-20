#include "Batch2DGeometry.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"

#include <algorithm>

#include "Asset.hpp"
#include "CameraComponent.hpp"
#include "Core.hpp"
#include "PackagedAssetBinarySerializer.hpp"
#include "Entity.hpp"
#include "FontAsset.hpp"
#include "FontInfo.hpp"
#include "ICamera.hpp"
#include "IDrawable2D.hpp"
#include "IContentStreamSource.hpp"
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
        : RenderManager2D()
        , ActiveCaptureCamera(nullptr) {
    }

    /// Builds one GameCube-native runtime texture from generated texture asset metadata.
    RuntimeTexture* GameCubeRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        GameCubeRuntimeTexture* runtimeTexture = new GameCubeRuntimeTexture();
        runtimeTexture->set_Id(data->get_Id());
        try {
            runtimeTexture->LoadFromRaw(data);
        } catch (...) {
            delete runtimeTexture;
            throw;
        }
        return runtimeTexture;
    }

    /// Builds one GameCube-native runtime texture from one packaged cooked texture asset path.
    RuntimeTexture* GameCubeRenderManager2D::BuildTextureFromCooked(HeCppString cookedAssetPath, IContentStreamSource* contentStreamSource) {
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked texture path is required.", "cookedAssetPath");
        } else if (contentStreamSource == nullptr) {
            throw new ArgumentNullException("contentStreamSource");
        }

        ::Stream* textureStream = contentStreamSource->OpenRead(cookedAssetPath);
        try {
            ::Asset* textureAssetPayload = ::PackagedAssetBinarySerializer::Deserialize(textureStream);
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

        if (std::find(ReleasedTextures.begin(), ReleasedTextures.end(), texture) == ReleasedTextures.end()) {
            ReleasedTextures.push_back(texture);
        }
    }

    /// Uploads one validated RGBA8 rectangle into one GameCube-owned runtime texture.
    void GameCubeRenderManager2D::UpdateTextureRegionCore(
        RuntimeTexture* texture,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height,
        Array<uint8_t>* rgba8,
        int32_t sourceRowPitch) {
        if (texture == nullptr) {
            throw new ArgumentNullException("texture");
        } else if (rgba8 == nullptr) {
            throw new ArgumentNullException("rgba8");
        }

        GameCubeRuntimeTexture* runtimeTexture = he_cpp_try_cast<GameCubeRuntimeTexture>(texture);
        if (runtimeTexture == nullptr) {
            throw new InvalidOperationException("GameCube texture region updates require GameCubeRuntimeTexture instances.");
        }

        if (std::find(ReleasedTextures.begin(), ReleasedTextures.end(), texture) != ReleasedTextures.end()) {
            throw new InvalidOperationException("GameCube texture region updates cannot target a released texture.");
        }

        runtimeTexture->UpdateRegionFromRgba8(x, y, width, height, rgba8, sourceRowPitch);
    }

    /// Releases one GameCube font asset together with its owned runtime texture and source texture payload.
    void GameCubeRenderManager2D::ReleaseFont(FontAsset* font) {
        if (font == nullptr) {
            throw new ArgumentNullException("font");
        }

        RuntimeTexture* texture = font->get_Texture();
        if (texture != nullptr && !texture->get_IsDisposed()) {
            texture->Dispose();
            ReleaseTexture(texture);
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

            ActiveCaptureCamera = camera;
            camera->get_RenderQueue2D()->VisitOrdered(this);
        }

        ActiveCaptureCamera = nullptr;
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
        DrawQueue.clear();
        SpriteQueue.clear();
        TextQueue.clear();
        RoundedRectQueue.clear();
        ActiveCaptureCamera = nullptr;
    }

    /// Returns whether the current frame captured any 2D draw requests.
    bool GameCubeRenderManager2D::HasCapturedDrawables() const {
        return !DrawQueue.empty();
    }

    /// Returns whether one camera submitted any 2D draw requests during the current frame.
    bool GameCubeRenderManager2D::HasCapturedDrawablesForCamera(CameraComponent* camera) const {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        for (const GameCubeDrawCommand2D& command : DrawQueue) {
            if (command.Camera == camera) {
                return true;
            }
        }

        return false;
    }

    /// Returns every captured 2D draw request in the exact order supplied by the shared engine render queue.
    const std::vector<GameCubeDrawCommand2D>& GameCubeRenderManager2D::GetDrawQueue() const {
        return DrawQueue;
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
        } else if (ActiveCaptureCamera == nullptr) {
            throw new InvalidOperationException("GameCube sprite capture requires an active source camera.");
        }

        SpriteQueue.push_back(GameCubeSpriteDrawCommand { ActiveCaptureCamera, sprite, Batch2DGeometry::ResolveSpriteQuad(sprite) });
        DrawQueue.push_back(GameCubeDrawCommand2D { ActiveCaptureCamera, GameCubeDrawCommandType2D::Sprite, SpriteQueue.size() - 1U });
    }

    /// Accepts a text draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawText(ITextDrawable2D* text) {
        if (text == nullptr) {
            throw new ArgumentNullException("text");
        } else if (ActiveCaptureCamera == nullptr) {
            throw new InvalidOperationException("GameCube text capture requires an active source camera.");
        }

        TextQueue.push_back(GameCubeTextDrawCommand { ActiveCaptureCamera, text });
        DrawQueue.push_back(GameCubeDrawCommand2D { ActiveCaptureCamera, GameCubeDrawCommandType2D::Text, TextQueue.size() - 1U });
    }

    /// Accepts a rounded-rectangle draw request without issuing native rendering yet.
    void GameCubeRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
        if (shape == nullptr) {
            throw new ArgumentNullException("shape");
        } else if (ActiveCaptureCamera == nullptr) {
            throw new InvalidOperationException("GameCube rounded-rectangle capture requires an active source camera.");
        }

        RoundedRectQueue.push_back(GameCubeRoundedRectDrawCommand { ActiveCaptureCamera, shape });
        DrawQueue.push_back(GameCubeDrawCommand2D { ActiveCaptureCamera, GameCubeDrawCommandType2D::RoundedRect, RoundedRectQueue.size() - 1U });
    }
}
