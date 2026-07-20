#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#include <algorithm>

#include <ogc/system.h>

#include "Asset.hpp"
#include "Core.hpp"
#include "PackagedAssetBinarySerializer.hpp"
#include "Entity.hpp"
#include "float2.hpp"
#include "IContentStreamSource.hpp"
#include "IDrawable3D.hpp"
#include "MaterialCullMode.hpp"
#include "MaterialRenderState.hpp"
#include "PlatformMaterialAsset.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "RenderManager2D.hpp"
#include "platform/gamecube/GameCubeFramePlan.hpp"
#include "platform/gamecube/GameCubeMeshCache.hpp"
#include "platform/gamecube/GameCubeRasterRenderer.hpp"
#include "platform/gamecube/GameCubeRuntimeMaterial.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelAsset.hpp"
#include "ModelSubmeshAsset.hpp"
#include "ModelSubmeshResolver.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "RuntimeSubmesh.hpp"
#include "TextureAsset.hpp"
#include "platform/gamecube/GameCubeRenderManager2D.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "platform/gamecube/GameCubeSceneRenderBridge.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/file.hpp"
#include "system/io/path.hpp"

namespace helengine::gamecube {
    /// Creates the GameCube 3D backend and its owned bridge/cache/raster collaborators.
    GameCubeRenderManager3D::GameCubeRenderManager3D()
        : RenderManager3D()
        , CapabilityProfile(new RendererBackendCapabilityProfile(true, false, false, false, 0, 0))
        , SceneRenderBridge(new GameCubeSceneRenderBridge())
        , MeshCache(new GameCubeMeshCache())
        , RasterRenderer(new GameCubeRasterRenderer(MeshCache))
        , OverlayRenderManager2D(nullptr)
        , HasRenderedSceneValue(false)
        , PresentedFrameWidth(0U)
        , PresentedFrameHeight(0U)
        , ExtractedFrameCount(0U) {
    }

    /// Releases owned GameCube renderer collaborators.
    GameCubeRenderManager3D::~GameCubeRenderManager3D() {
        delete RasterRenderer;
        delete MeshCache;
        delete SceneRenderBridge;
        delete CapabilityProfile;
    }

    /// Rebuilds one legacy raw material asset path through the cooked platform-owned GameCube material contract.
    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromRawAsset(ContentManager* assetContentManager, std::string materialAssetPath) {
        if (assetContentManager == nullptr) {
            throw new ArgumentNullException("assetContentManager");
        }

        if (materialAssetPath.empty()) {
            throw new ArgumentException("GameCube material asset path is required.", "materialAssetPath");
        }

        return BuildMaterialFromCooked(materialAssetPath, nullptr);
    }

    /// Builds the minimal runtime material required for the first cooked-material GameCube draw path.
    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        }

        GameCubeRuntimeMaterial* runtimeMaterial = new GameCubeRuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        runtimeMaterial->set_LightingModel(materialAsset->Lit
            ? RuntimeMaterialLightingModel::MetalRoughPbr
            : RuntimeMaterialLightingModel::Unlit);
        runtimeMaterial->SetBaseColor(float3(
            static_cast<float>(materialAsset->BaseColorR) / 255.0f,
            static_cast<float>(materialAsset->BaseColorG) / 255.0f,
            static_cast<float>(materialAsset->BaseColorB) / 255.0f));
        runtimeMaterial->SetTextureRelativePath(materialAsset->TextureRelativePath);
        if (materialAsset->DoubleSided) {
            runtimeMaterial->get_RenderState()->set_CullMode(MaterialCullMode::None);
        }

        return runtimeMaterial;
    }

    /// Builds the minimal runtime material required for the first cooked-material GameCube draw path from one serialized cooked asset path.
    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource) {
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked material path is required.", "cookedAssetPath");
        } else if (contentStreamSource == nullptr) {
            throw new ArgumentNullException("contentStreamSource");
        }

        ::Stream* stream = contentStreamSource->OpenRead(cookedAssetPath);
        try {
            ::Asset* asset = ::PackagedAssetBinarySerializer::Deserialize(stream);
            ::PlatformMaterialAsset* cookedMaterialAsset = he_cpp_try_cast<::PlatformMaterialAsset>(asset);
            if (cookedMaterialAsset == nullptr) {
                throw new ArgumentException("GameCube cooked material payload did not deserialize as PlatformMaterialAsset.", "cookedAssetPath");
            }

            stream->Dispose();
            GameCubeRuntimeMaterial* runtimeMaterial = static_cast<GameCubeRuntimeMaterial*>(BuildMaterialFromCooked(cookedMaterialAsset));
            AttachCookedDiffuseTexture(runtimeMaterial, cookedMaterialAsset, cookedAssetPath, contentStreamSource);
            delete cookedMaterialAsset;
            return runtimeMaterial;
        } catch (...) {
            if (stream != nullptr) {
                stream->Dispose();
            }

            throw;
        }
    }

    /// Builds a GameCube runtime model that keeps authored submesh and geometry arrays alive.
    RuntimeModel* GameCubeRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        ModelAssetIndexData* indexData = ModelAssetIndexData::Resolve(data);
        GameCubeRuntimeModel* runtimeModel = new GameCubeRuntimeModel();
        runtimeModel->set_Id(data->get_Id());
        runtimeModel->SetBounds(data->BoundsMin, data->BoundsMax);
        runtimeModel->SetSubmeshes(ModelSubmeshResolver::BuildRuntimeSubmeshes(data));
        runtimeModel->Positions = data->Positions;
        runtimeModel->Normals = data->Normals;
        runtimeModel->TexCoords = data->TexCoords;
        runtimeModel->Indices16 = indexData->get_Indices16();
        runtimeModel->Indices32 = indexData->get_Indices32();
        runtimeModel->Uses32BitIndices = indexData->get_Uses32BitIndices();
        delete indexData;
        return runtimeModel;
    }

    /// Builds a GameCube runtime model from one serialized cooked model asset path.
    RuntimeModel* GameCubeRenderManager3D::BuildModelFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource) {
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked model path is required.", "cookedAssetPath");
        } else if (contentStreamSource == nullptr) {
            throw new ArgumentNullException("contentStreamSource");
        }

        ::Stream* stream = contentStreamSource->OpenRead(cookedAssetPath);
        try {
            ::Asset* asset = ::PackagedAssetBinarySerializer::Deserialize(stream);
            ::ModelAsset* cookedModelAsset = he_cpp_try_cast<::ModelAsset>(asset);
            if (cookedModelAsset == nullptr) {
                throw new ArgumentException("GameCube cooked model payload did not deserialize as ModelAsset.", "cookedAssetPath");
            }

            stream->Dispose();
            GameCubeRuntimeModel* runtimeModel = static_cast<GameCubeRuntimeModel*>(BuildModelFromRaw(cookedModelAsset));
            runtimeModel->OwnedSourceModelAsset = cookedModelAsset;
            return runtimeModel;
        } catch (...) {
            if (stream != nullptr) {
                stream->Dispose();
            }

            throw;
        }
    }

    /// Releases one GameCube runtime material after the final scene reference is removed.
    void GameCubeRenderManager3D::ReleaseMaterial(RuntimeMaterial* material) {
        if (material == nullptr) {
            throw new ArgumentNullException("material");
        }

        ReleasedMaterials.push_back(material);
    }

    /// Releases deferred GameCube runtime materials and models after the scene manager reaches a safe transition boundary.
    void GameCubeRenderManager3D::FlushReleasedAssets() {
        for (RuntimeMaterial* material : ReleasedMaterials) {
            if (material == nullptr) {
                continue;
            }

            GameCubeRuntimeMaterial* runtimeMaterial = static_cast<GameCubeRuntimeMaterial*>(material);
            RuntimeTexture* ownedDiffuseTexture = runtimeMaterial->GetOwnedDiffuseTexture();
            if (ownedDiffuseTexture != nullptr) {
                Core* core = Core::get_Instance();
                if (core == nullptr || core->get_RenderManager2D() == nullptr) {
                    throw new InvalidOperationException("GameCube material release requires an active RenderManager2D to release material-owned diffuse textures.");
                }

                core->get_RenderManager2D()->ReleaseTexture(ownedDiffuseTexture);
                runtimeMaterial->SetOwnedDiffuseTexture(nullptr);
            }

            runtimeMaterial->Dispose();
            delete runtimeMaterial;
        }

        ReleasedMaterials.clear();

        for (RuntimeModel* model : ReleasedModels) {
            if (model == nullptr) {
                continue;
            }

            GameCubeRuntimeModel* runtimeModel = static_cast<GameCubeRuntimeModel*>(model);
            ReleaseOwnedSourceModelAsset(runtimeModel);
            Array<RuntimeSubmesh*>* submeshes = runtimeModel->get_Submeshes();
            if (submeshes != nullptr && submeshes != Array<RuntimeSubmesh*>::Empty()) {
                for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
                    delete (*submeshes)[submeshIndex];
                }

                delete submeshes;
            }

            if (runtimeModel->CachedMeshData != nullptr) {
                if (runtimeModel->CachedMeshData->PackedPositions != nullptr && runtimeModel->CachedMeshData->PackedPositions != Array<GameCubePackedPosition3>::Empty()) {
                    delete runtimeModel->CachedMeshData->PackedPositions;
                }

                if (runtimeModel->CachedMeshData->PackedNormals != nullptr && runtimeModel->CachedMeshData->PackedNormals != Array<GameCubePackedNormal3>::Empty()) {
                    delete runtimeModel->CachedMeshData->PackedNormals;
                }

                if (runtimeModel->CachedMeshData->PackedTexCoords != nullptr && runtimeModel->CachedMeshData->PackedTexCoords != Array<GameCubePackedTexCoord2>::Empty()) {
                    delete runtimeModel->CachedMeshData->PackedTexCoords;
                }

                if (runtimeModel->CachedMeshData->Indices16 != nullptr && runtimeModel->CachedMeshData->Indices16 != Array<uint16_t>::Empty()) {
                    delete runtimeModel->CachedMeshData->Indices16;
                }

                if (runtimeModel->CachedMeshData->SubmeshIndexStarts != nullptr && runtimeModel->CachedMeshData->SubmeshIndexStarts != Array<int32_t>::Empty()) {
                    delete runtimeModel->CachedMeshData->SubmeshIndexStarts;
                }

                if (runtimeModel->CachedMeshData->SubmeshIndexCounts != nullptr && runtimeModel->CachedMeshData->SubmeshIndexCounts != Array<int32_t>::Empty()) {
                    delete runtimeModel->CachedMeshData->SubmeshIndexCounts;
                }

                delete runtimeModel->CachedMeshData;
            }

            if (runtimeModel->Positions != nullptr && runtimeModel->Positions != Array<float3>::Empty()) {
                delete runtimeModel->Positions;
            }

            if (runtimeModel->Normals != nullptr && runtimeModel->Normals != Array<float3>::Empty()) {
                delete runtimeModel->Normals;
            }

            if (runtimeModel->TexCoords != nullptr && runtimeModel->TexCoords != Array<float2>::Empty()) {
                delete runtimeModel->TexCoords;
            }

            if (runtimeModel->Indices16 != nullptr && runtimeModel->Indices16 != Array<uint16_t>::Empty()) {
                delete runtimeModel->Indices16;
            }

            if (runtimeModel->Indices32 != nullptr && runtimeModel->Indices32 != Array<uint32_t>::Empty()) {
                delete runtimeModel->Indices32;
            }

            delete runtimeModel;
        }

        ReleasedModels.clear();
    }

    /// Releases one GameCube runtime model after the final scene reference is removed.
    void GameCubeRenderManager3D::ReleaseModel(RuntimeModel* model) {
        if (model == nullptr) {
            throw new ArgumentNullException("model");
        }

        ReleasedModels.push_back(model);
    }

    /// Extracts the current frame and renders it through GX.
    void GameCubeRenderManager3D::Draw() {
        if (OverlayRenderManager2D == nullptr) {
            throw new InvalidOperationException("GameCubeRenderManager3D requires an overlay GameCubeRenderManager2D before Draw().");
        } else if (PresentedFrameWidth == 0U) {
            throw new InvalidOperationException("GameCubeRenderManager3D requires one presented framebuffer width before Draw().");
        } else if (PresentedFrameHeight == 0U) {
            throw new InvalidOperationException("GameCubeRenderManager3D requires one presented framebuffer height before Draw().");
        }

        OverlayRenderManager2D->Draw();
        const bool has2DDrawables = OverlayRenderManager2D->HasCapturedDrawables();
        GameCubeFramePlan* framePlan = SceneRenderBridge->BuildFramePlan(CapabilityProfile, MainWindowSize.X, MainWindowSize.Y, PresentedFrameWidth, PresentedFrameHeight);
        if (framePlan == nullptr) {
            HasRenderedSceneValue = false;
            return;
        }

        const bool has3DDrawables = framePlan->DrawableSubmissions->get_Count() > 0;
        if (!has3DDrawables && !has2DDrawables) {
            HasRenderedSceneValue = false;
            delete framePlan;
            return;
        }

        if (has3DDrawables) {
            ExtractedFrameCount++;
            HasRenderedSceneValue = RasterRenderer->DrawFrame(framePlan);
        } else {
            HasRenderedSceneValue = false;
        }

        if (has2DDrawables) {
            RasterRenderer->Render2D(framePlan, *OverlayRenderManager2D, MainWindowSize.X, MainWindowSize.Y);
        }

        delete framePlan;
    }

    /// Registers the 2D overlay render manager that should be captured and rasterized inside the shared 3D draw call.
    void GameCubeRenderManager3D::SetOverlayRenderManager2D(GameCubeRenderManager2D* renderManager2D) {
        if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }

        OverlayRenderManager2D = renderManager2D;
    }

    /// Registers the physical presented framebuffer size used for GX viewport and scissor setup.
    void GameCubeRenderManager3D::SetPresentedFrameSize(uint16_t width, uint16_t height) {
        if (width == 0U) {
            throw new ArgumentOutOfRangeException("width");
        } else if (height == 0U) {
            throw new ArgumentOutOfRangeException("height");
        }

        PresentedFrameWidth = width;
        PresentedFrameHeight = height;
    }

    /// Returns the strict backend capability surface exposed by the first GameCube tier.
    RendererBackendCapabilityProfile* GameCubeRenderManager3D::GetCapabilityProfile() {
        return CapabilityProfile;
    }

    /// Reports whether this backend has already emitted at least one real GX scene frame.
    bool GameCubeRenderManager3D::HasRenderedScene() const {
        return HasRenderedSceneValue;
    }

    /// Resolves one packaged content-relative asset path against the absolute cooked material path that referenced it.
    std::string GameCubeRenderManager3D::ResolvePackagedContentAssetPath(const std::string& cookedMaterialAssetPath, const std::string& contentRelativePath) {
        if (cookedMaterialAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked material path is required.", "cookedMaterialAssetPath");
        } else if (contentRelativePath.empty()) {
            throw new ArgumentException("GameCube content-relative asset path is required.", "contentRelativePath");
        }

        std::string normalizedMaterialAssetPath = cookedMaterialAssetPath;
        std::replace(normalizedMaterialAssetPath.begin(), normalizedMaterialAssetPath.end(), '\\', '/');
        const std::size_t cookedMarkerIndex = normalizedMaterialAssetPath.find("cooked/");
        if (cookedMarkerIndex == std::string::npos) {
            throw new InvalidOperationException("GameCube cooked material path must contain the packaged 'cooked/' root segment.");
        }

        const std::string contentRootPath = cookedMaterialAssetPath.substr(0, cookedMarkerIndex);
        return Path::GetFullPath(Path::Combine(contentRootPath, contentRelativePath));
    }

    /// Loads and attaches one cooked diffuse texture when the path-based GameCube cooked-material contract references one.
    void GameCubeRenderManager3D::AttachCookedDiffuseTexture(GameCubeRuntimeMaterial* runtimeMaterial, PlatformMaterialAsset* materialAsset, const std::string& cookedMaterialAssetPath, IContentStreamSource* contentStreamSource) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        } else if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        } else if (cookedMaterialAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked material path is required.", "cookedMaterialAssetPath");
        } else if (contentStreamSource == nullptr) {
            throw new ArgumentNullException("contentStreamSource");
        }

        if (materialAsset->TextureRelativePath.empty()) {
            return;
        }

        const std::string cookedTextureAssetPath = ResolvePackagedContentAssetPath(cookedMaterialAssetPath, materialAsset->TextureRelativePath);
        ::Stream* textureStream = contentStreamSource->OpenRead(cookedTextureAssetPath);
        try {
            ::Asset* textureAssetPayload = ::PackagedAssetBinarySerializer::Deserialize(textureStream);
            ::TextureAsset* textureAsset = he_cpp_try_cast<::TextureAsset>(textureAssetPayload);
            if (textureAsset == nullptr) {
                throw new InvalidOperationException("GameCube cooked diffuse texture payload did not deserialize as TextureAsset.");
            }

            Core* core = Core::get_Instance();
            if (core == nullptr || core->get_RenderManager2D() == nullptr) {
                throw new InvalidOperationException("GameCube cooked diffuse texture attachment requires an active RenderManager2D.");
            }

            textureStream->Dispose();
            RuntimeTexture* runtimeTexture = core->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);
            runtimeMaterial->SetOwnedDiffuseTexture(runtimeTexture);
            if (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {
                delete textureAsset->Colors;
                textureAsset->Colors = Array<uint8_t>::Empty();
            }

            if (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {
                delete textureAsset->PaletteColors;
                textureAsset->PaletteColors = Array<uint8_t>::Empty();
            }

            delete textureAsset;
        } catch (...) {
            if (textureStream != nullptr) {
                textureStream->Dispose();
            }

            throw;
        }
    }

    /// Releases one owned deserialized cooked model payload attached to a GameCube runtime model.
    void GameCubeRenderManager3D::ReleaseOwnedSourceModelAsset(GameCubeRuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr || runtimeModel->OwnedSourceModelAsset == nullptr) {
            return;
        }

        ModelAsset* ownedSourceModelAsset = runtimeModel->OwnedSourceModelAsset;
        Array<ModelSubmeshAsset*>* submeshes = ownedSourceModelAsset->Submeshes;
        ownedSourceModelAsset->Positions = nullptr;
        ownedSourceModelAsset->Normals = nullptr;
        ownedSourceModelAsset->TexCoords = nullptr;
        ownedSourceModelAsset->Indices16 = nullptr;
        ownedSourceModelAsset->Indices32 = nullptr;
        ownedSourceModelAsset->Submeshes = nullptr;
        runtimeModel->OwnedSourceModelAsset = nullptr;

        if (submeshes != nullptr) {
            for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
                delete (*submeshes)[submeshIndex];
            }

            delete submeshes;
        }

        delete ownedSourceModelAsset;
    }
}
