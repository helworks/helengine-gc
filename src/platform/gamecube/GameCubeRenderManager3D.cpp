#include "platform/gamecube/GameCubeRenderManager3D.hpp"

#include <algorithm>

#include <ogc/system.h>

#include "Asset.hpp"
#include "Core.hpp"
#include "EditorAssetBinarySerializer.hpp"
#include "Entity.hpp"
#include "float2.hpp"
#include "IDrawable3D.hpp"
#include "MaterialLayout.hpp"
#include "MaterialLayoutBinding.hpp"
#include "MaterialCullMode.hpp"
#include "MaterialPropertyBlock.hpp"
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
#include "ShaderResourceType.hpp"
#include "StandardMaterialTextureBindingDefaults.hpp"
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
        , ExtractedFrameCount(0U) {
    }

    /// Releases owned GameCube renderer collaborators.
    GameCubeRenderManager3D::~GameCubeRenderManager3D() {
        delete RasterRenderer;
        delete MeshCache;
        delete SceneRenderBridge;
        delete CapabilityProfile;
    }

    /// Builds the minimal runtime material required for the first cooked-material GameCube draw path.
    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        }

        GameCubeRuntimeMaterial* runtimeMaterial = new GameCubeRuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        runtimeMaterial->SetLayout(CreateCookedMaterialLayout());
        StandardMaterialTextureBindingDefaults::Apply(runtimeMaterial);
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

        SYS_Report(
            "[GC] RM3D build cooked material id=%s texturePath=%s lit=%d ptr=%p\n",
            materialAsset->get_Id().c_str(),
            materialAsset->TextureRelativePath.c_str(),
            materialAsset->Lit ? 1 : 0,
            runtimeMaterial);
        return runtimeMaterial;
    }

    /// Builds the minimal runtime material required for the first cooked-material GameCube draw path from one serialized cooked asset path.
    RuntimeMaterial* GameCubeRenderManager3D::BuildMaterialFromCooked(std::string cookedAssetPath) {
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked material path is required.", "cookedAssetPath");
        }

        ::FileStream* stream = ::File::OpenRead(cookedAssetPath);
        try {
            ::Asset* asset = ::EditorAssetBinarySerializer::Deserialize(stream);
            ::PlatformMaterialAsset* cookedMaterialAsset = he_cpp_try_cast<::PlatformMaterialAsset>(asset);
            if (cookedMaterialAsset == nullptr) {
                throw new ArgumentException("GameCube cooked material payload did not deserialize as PlatformMaterialAsset.", "cookedAssetPath");
            }

            stream->Dispose();
            GameCubeRuntimeMaterial* runtimeMaterial = static_cast<GameCubeRuntimeMaterial*>(BuildMaterialFromCooked(cookedMaterialAsset));
            AttachCookedDiffuseTexture(runtimeMaterial, cookedMaterialAsset, cookedAssetPath);
            SYS_Report(
                "[GC] RM3D build cooked material path=%s id=%s ptr=%p\n",
                cookedAssetPath.c_str(),
                cookedMaterialAsset->get_Id().c_str(),
                runtimeMaterial);
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
        SYS_Report(
            "[GC] RM3D build raw model id=%s positions=%u submeshes=%u ptr=%p\n",
            data->get_Id().c_str(),
            static_cast<unsigned>(runtimeModel->Positions != nullptr ? runtimeModel->Positions->get_Length() : 0),
            static_cast<unsigned>(runtimeModel->get_Submeshes() != nullptr ? runtimeModel->get_Submeshes()->get_Length() : 0),
            runtimeModel);
        delete indexData;
        return runtimeModel;
    }

    /// Builds a GameCube runtime model from one serialized cooked model asset path.
    RuntimeModel* GameCubeRenderManager3D::BuildModelFromCooked(std::string cookedAssetPath) {
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked model path is required.", "cookedAssetPath");
        }

        ::FileStream* stream = ::File::OpenRead(cookedAssetPath);
        try {
            ::Asset* asset = ::EditorAssetBinarySerializer::Deserialize(stream);
            ::ModelAsset* cookedModelAsset = he_cpp_try_cast<::ModelAsset>(asset);
            if (cookedModelAsset == nullptr) {
                throw new ArgumentException("GameCube cooked model payload did not deserialize as ModelAsset.", "cookedAssetPath");
            }

            stream->Dispose();
            GameCubeRuntimeModel* runtimeModel = static_cast<GameCubeRuntimeModel*>(BuildModelFromRaw(cookedModelAsset));
            runtimeModel->OwnedSourceModelAsset = cookedModelAsset;
            SYS_Report(
                "[GC] RM3D build cooked model path=%s id=%s ptr=%p\n",
                cookedAssetPath.c_str(),
                cookedModelAsset->get_Id().c_str(),
                runtimeModel);
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

        SYS_Report(
            "[GC] RM3D release material queued id=%s ptr=%p\n",
            material->get_Id().c_str(),
            material);
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
            SYS_Report(
                "[GC] RM3D flush model begin id=%s ptr=%p ownedModel=%p positions=%p normals=%p texCoords=%p indices16=%p indices32=%p cached=%p submeshes=%p\n",
                runtimeModel->get_Id().c_str(),
                runtimeModel,
                runtimeModel->OwnedSourceModelAsset,
                runtimeModel->Positions,
                runtimeModel->Normals,
                runtimeModel->TexCoords,
                runtimeModel->Indices16,
                runtimeModel->Indices32,
                runtimeModel->CachedMeshData,
                runtimeModel->get_Submeshes());
            ReleaseOwnedSourceModelAsset(runtimeModel);
            Array<RuntimeSubmesh*>* submeshes = runtimeModel->get_Submeshes();
            if (submeshes != nullptr && submeshes != Array<RuntimeSubmesh*>::Empty()) {
                SYS_Report("[GC] RM3D flush model delete runtime submeshes array=%p count=%ld\n", submeshes, static_cast<long>(submeshes->get_Length()));
                for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
                    SYS_Report("[GC] RM3D flush model delete runtime submesh[%ld]=%p\n", static_cast<long>(submeshIndex), (*submeshes)[submeshIndex]);
                    delete (*submeshes)[submeshIndex];
                }

                delete submeshes;
            }

            if (runtimeModel->CachedMeshData != nullptr) {
                SYS_Report("[GC] RM3D flush model delete cached mesh=%p\n", runtimeModel->CachedMeshData);
                if (runtimeModel->CachedMeshData->PackedPositions != nullptr && runtimeModel->CachedMeshData->PackedPositions != Array<GameCubePackedPosition3>::Empty()) {
                    SYS_Report("[GC] RM3D flush model delete packed positions=%p\n", runtimeModel->CachedMeshData->PackedPositions);
                    delete runtimeModel->CachedMeshData->PackedPositions;
                }

                if (runtimeModel->CachedMeshData->PackedTexCoords != nullptr && runtimeModel->CachedMeshData->PackedTexCoords != Array<GameCubePackedTexCoord2>::Empty()) {
                    SYS_Report("[GC] RM3D flush model delete packed texcoords=%p\n", runtimeModel->CachedMeshData->PackedTexCoords);
                    delete runtimeModel->CachedMeshData->PackedTexCoords;
                }

                if (runtimeModel->CachedMeshData->Indices16 != nullptr && runtimeModel->CachedMeshData->Indices16 != Array<uint16_t>::Empty()) {
                    SYS_Report("[GC] RM3D flush model delete cached indices16=%p\n", runtimeModel->CachedMeshData->Indices16);
                    delete runtimeModel->CachedMeshData->Indices16;
                }

                if (runtimeModel->CachedMeshData->SubmeshIndexStarts != nullptr && runtimeModel->CachedMeshData->SubmeshIndexStarts != Array<int32_t>::Empty()) {
                    SYS_Report("[GC] RM3D flush model delete submesh index starts=%p\n", runtimeModel->CachedMeshData->SubmeshIndexStarts);
                    delete runtimeModel->CachedMeshData->SubmeshIndexStarts;
                }

                if (runtimeModel->CachedMeshData->SubmeshIndexCounts != nullptr && runtimeModel->CachedMeshData->SubmeshIndexCounts != Array<int32_t>::Empty()) {
                    SYS_Report("[GC] RM3D flush model delete submesh index counts=%p\n", runtimeModel->CachedMeshData->SubmeshIndexCounts);
                    delete runtimeModel->CachedMeshData->SubmeshIndexCounts;
                }

                delete runtimeModel->CachedMeshData;
            }

            if (runtimeModel->Positions != nullptr && runtimeModel->Positions != Array<float3>::Empty()) {
                SYS_Report("[GC] RM3D flush model delete positions=%p\n", runtimeModel->Positions);
                delete runtimeModel->Positions;
            }

            if (runtimeModel->Normals != nullptr && runtimeModel->Normals != Array<float3>::Empty()) {
                SYS_Report("[GC] RM3D flush model delete normals=%p\n", runtimeModel->Normals);
                delete runtimeModel->Normals;
            }

            if (runtimeModel->TexCoords != nullptr && runtimeModel->TexCoords != Array<float2>::Empty()) {
                SYS_Report("[GC] RM3D flush model delete texcoords=%p\n", runtimeModel->TexCoords);
                delete runtimeModel->TexCoords;
            }

            if (runtimeModel->Indices16 != nullptr && runtimeModel->Indices16 != Array<uint16_t>::Empty()) {
                SYS_Report("[GC] RM3D flush model delete indices16=%p\n", runtimeModel->Indices16);
                delete runtimeModel->Indices16;
            }

            if (runtimeModel->Indices32 != nullptr && runtimeModel->Indices32 != Array<uint32_t>::Empty()) {
                SYS_Report("[GC] RM3D flush model delete indices32=%p\n", runtimeModel->Indices32);
                delete runtimeModel->Indices32;
            }

            SYS_Report("[GC] RM3D flush model delete runtime model=%p\n", runtimeModel);
            delete runtimeModel;
        }

        ReleasedModels.clear();
    }

    /// Releases one GameCube runtime model after the final scene reference is removed.
    void GameCubeRenderManager3D::ReleaseModel(RuntimeModel* model) {
        if (model == nullptr) {
            throw new ArgumentNullException("model");
        }

        SYS_Report(
            "[GC] RM3D release model queued id=%s ptr=%p\n",
            model->get_Id().c_str(),
            model);
        ReleasedModels.push_back(model);
    }

    /// Extracts the current frame and renders it through GX.
    void GameCubeRenderManager3D::Draw() {
        if (OverlayRenderManager2D == nullptr) {
            throw new InvalidOperationException("GameCubeRenderManager3D requires an overlay GameCubeRenderManager2D before Draw().");
        }

        SYS_Report("[GC] RM3D integrated draw begin.\n");
        OverlayRenderManager2D->Draw();
        const bool has2DDrawables = OverlayRenderManager2D->HasCapturedDrawables();
        SYS_Report("[GC] RM3D integrated draw overlay captured has2D=%d\n", has2DDrawables ? 1 : 0);
        GameCubeFramePlan* framePlan = SceneRenderBridge->BuildFramePlan(CapabilityProfile, MainWindowSize.X, MainWindowSize.Y);
        if (framePlan == nullptr) {
            HasRenderedSceneValue = false;
            SYS_Report("[GC] RM3D integrated draw skipped: no active camera yet.\n");
            return;
        }

        const bool has3DDrawables = framePlan->DrawableSubmissions->get_Count() > 0;
        SYS_Report(
            "[GC] RM3D integrated draw frame plan has3D=%d has2D=%d camera=%p\n",
            has3DDrawables ? 1 : 0,
            has2DDrawables ? 1 : 0,
            framePlan->Camera);
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
            SYS_Report("[GC] RM3D integrated draw render2d begin.\n");
            RasterRenderer->Render2D(framePlan, *OverlayRenderManager2D, MainWindowSize.X, MainWindowSize.Y);
            SYS_Report("[GC] RM3D integrated draw render2d end.\n");
        }

        delete framePlan;
        SYS_Report("[GC] RM3D integrated draw end.\n");
    }

    /// Registers the 2D overlay render manager that should be captured and rasterized inside the shared 3D draw call.
    void GameCubeRenderManager3D::SetOverlayRenderManager2D(GameCubeRenderManager2D* renderManager2D) {
        if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }

        OverlayRenderManager2D = renderManager2D;
    }

    /// Returns the strict backend capability surface exposed by the first GameCube tier.
    RendererBackendCapabilityProfile* GameCubeRenderManager3D::GetCapabilityProfile() {
        return CapabilityProfile;
    }

    /// Reports whether this backend has already emitted at least one real GX scene frame.
    bool GameCubeRenderManager3D::HasRenderedScene() const {
        return HasRenderedSceneValue;
    }

    /// Builds the minimal shared-engine material layout needed for one diffuse texture binding.
    MaterialLayout* GameCubeRenderManager3D::CreateCookedMaterialLayout() {
        return new MaterialLayout(
            std::string(),
            std::string(),
            std::string(),
            std::string(),
            new MaterialRenderState(),
            new Array<MaterialLayoutBinding*>({
                new MaterialLayoutBinding(
                    StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName,
                    ShaderResourceType::Texture2D,
                    0,
                    0,
                    0)
            }),
            Array<MaterialLayoutBinding*>::Empty(),
            Array<MaterialLayoutBinding*>::Empty());
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
        const std::size_t cookedMarkerIndex = normalizedMaterialAssetPath.find("/cooked/");
        if (cookedMarkerIndex == std::string::npos) {
            throw new InvalidOperationException("GameCube cooked material path must contain the packaged '/cooked/' root segment.");
        }

        const std::string contentRootPath = cookedMaterialAssetPath.substr(0, cookedMarkerIndex);
        return Path::GetFullPath(Path::Combine(contentRootPath, contentRelativePath));
    }

    /// Loads and attaches one cooked diffuse texture when the path-based GameCube cooked-material contract references one.
    void GameCubeRenderManager3D::AttachCookedDiffuseTexture(GameCubeRuntimeMaterial* runtimeMaterial, PlatformMaterialAsset* materialAsset, const std::string& cookedMaterialAssetPath) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        } else if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        } else if (cookedMaterialAssetPath.empty()) {
            throw new ArgumentException("GameCube cooked material path is required.", "cookedMaterialAssetPath");
        }

        if (materialAsset->TextureRelativePath.empty()) {
            return;
        }

        const std::string cookedTextureAssetPath = ResolvePackagedContentAssetPath(cookedMaterialAssetPath, materialAsset->TextureRelativePath);
        ::FileStream* textureStream = ::File::OpenRead(cookedTextureAssetPath);
        try {
            ::Asset* textureAssetPayload = ::EditorAssetBinarySerializer::Deserialize(textureStream);
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
            runtimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);
            runtimeMaterial->SetOwnedDiffuseTexture(runtimeTexture);
            SYS_Report(
                "[GC] RM3D attach cooked diffuse texture material=%s texturePath=%s texturePtr=%p\n",
                runtimeMaterial->get_Id().c_str(),
                cookedTextureAssetPath.c_str(),
                runtimeTexture);
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
        SYS_Report(
            "[GC] RM3D release owned source model asset=%p submeshes=%p positions=%p normals=%p texCoords=%p indices16=%p indices32=%p\n",
            ownedSourceModelAsset,
            submeshes,
            ownedSourceModelAsset->Positions,
            ownedSourceModelAsset->Normals,
            ownedSourceModelAsset->TexCoords,
            ownedSourceModelAsset->Indices16,
            ownedSourceModelAsset->Indices32);
        ownedSourceModelAsset->Positions = nullptr;
        ownedSourceModelAsset->Normals = nullptr;
        ownedSourceModelAsset->TexCoords = nullptr;
        ownedSourceModelAsset->Indices16 = nullptr;
        ownedSourceModelAsset->Indices32 = nullptr;
        ownedSourceModelAsset->Submeshes = nullptr;
        runtimeModel->OwnedSourceModelAsset = nullptr;

        if (submeshes != nullptr) {
            for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
                SYS_Report("[GC] RM3D release owned source model delete submesh[%ld]=%p\n", static_cast<long>(submeshIndex), (*submeshes)[submeshIndex]);
                delete (*submeshes)[submeshIndex];
            }

            SYS_Report("[GC] RM3D release owned source model delete submesh array=%p\n", submeshes);
            delete submeshes;
        }

        SYS_Report("[GC] RM3D release owned source model delete model asset=%p\n", ownedSourceModelAsset);
        delete ownedSourceModelAsset;
    }
}
