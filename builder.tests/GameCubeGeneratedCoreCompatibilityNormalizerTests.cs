namespace helengine.gamecube.builder.tests;

/// <summary>
/// Verifies the generated-core compatibility normalizer accepts current cooked-material resolver output.
/// </summary>
public sealed class GameCubeGeneratedCoreCompatibilityNormalizerTests {
    /// <summary>
    /// Ensures the normalizer preserves runtime scene asset resolver output that already uses the current path-based cooked platform material contract.
    /// </summary>
    [Fact]
    public void Normalize_WhenRuntimeSceneResolverAlreadyUsesPathBasedCookedPlatformMaterials_PreservesSource() {
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "gamecube-generated-core-normalizer", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(generatedCoreRootPath);

        try {
            string sourcePath = Path.Combine(generatedCoreRootPath, "RuntimeSceneAssetReferenceResolver.cpp");
            string originalSource = """
                #include "MaterialAsset.hpp"
                #include "PlatformMaterialAsset.hpp"
                #include "ShaderAsset.hpp"
                #include "RuntimeMaterial.hpp"
                #include "TextureAsset.hpp"
                #include "RuntimeTexture.hpp"
                #include "RenderManager2D.hpp"

                ::RuntimeMaterial *RuntimeSceneAssetReferenceResolver::ResolveRuntimeMaterial(::SceneAssetReference* reference)
                {
                if (reference->get_SourceKind() == SceneAssetReferenceSourceKind::Generated)
                {
                const std::string generatedAssetKey = this->BuildGeneratedAssetCacheKey(reference);
                ::RuntimeMaterial* generatedRuntimeMaterial;
                if (this->ActiveGeneratedMaterialsByKey->TryGetValue(generatedAssetKey, generatedRuntimeMaterial))
                {
                this->TrackOwnedMaterial(generatedRuntimeMaterial);
                return generatedRuntimeMaterial;    }
                const std::string generatedFullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedFullPath);
                this->ActiveGeneratedMaterialsByKey->Add(generatedAssetKey, generatedCookedRuntimeMaterial);
                this->TrackOwnedMaterial(generatedCookedRuntimeMaterial);
                return generatedCookedRuntimeMaterial;    }
                const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(fullPath);
                this->TrackOwnedMaterial(runtimeMaterial);
                return runtimeMaterial;}
                """;
            File.WriteAllText(sourcePath, originalSource);

            GameCubeGeneratedCoreCompatibilityNormalizer normalizer = new();

            normalizer.Normalize(generatedCoreRootPath);

            string normalizedSource = File.ReadAllText(sourcePath);
            Assert.Equal(originalSource, normalizedSource);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }

    /// <summary>
    /// Ensures the normalizer injects GameCube resolver logging into the current path-based cooked-platform material resolver output.
    /// </summary>
    [Fact]
    public void Normalize_WhenRuntimeSceneResolverUsesPathBasedCookedPlatformMaterials_AddsResolverLogging() {
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "gamecube-generated-core-normalizer", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(generatedCoreRootPath);

        try {
            string sourcePath = Path.Combine(generatedCoreRootPath, "RuntimeSceneAssetReferenceResolver.cpp");
            string originalSource = """
                #include "RuntimeSceneAssetReferenceResolver.hpp"

                void RuntimeSceneAssetReferenceResolver::BeginOwnedAssetTracking()
                {
                this->ResetGeneratedRuntimeAssetCaches();
                this->ActiveOwnedTextures = new List<::RuntimeTexture*>();
                this->ActiveOwnedFonts = new List<::FontAsset*>();
                this->ActiveResolvedFontsByPath = new Dictionary<std::string, ::FontAsset*>(StringComparer::get_OrdinalIgnoreCase());
                this->ActiveOwnedModels = new List<::RuntimeModel*>();
                this->ActiveOwnedMaterials = new List<::RuntimeMaterial*>();
                }

                ::RuntimeSceneOwnedAssetSet* RuntimeSceneAssetReferenceResolver::CompleteOwnedAssetTracking()
                {
                List<::RuntimeTexture*> *ownedTextures = this->ActiveOwnedTextures;
                List<::FontAsset*> *ownedFonts = this->ActiveOwnedFonts;
                List<::RuntimeModel*> *ownedModels = this->ActiveOwnedModels;
                List<::RuntimeMaterial*> *ownedMaterials = this->ActiveOwnedMaterials;
                Dictionary<std::string, ::FontAsset*> *resolvedFontsByPath = this->ActiveResolvedFontsByPath;
                this->ActiveOwnedTextures = nullptr;
                this->ActiveOwnedFonts = nullptr;
                this->ActiveResolvedFontsByPath = nullptr;
                this->ActiveOwnedModels = nullptr;
                this->ActiveOwnedMaterials = nullptr;
                this->ResetGeneratedRuntimeAssetCaches();
                delete resolvedFontsByPath;
                return new ::RuntimeSceneOwnedAssetSet(ownedTextures, ownedFonts, ownedModels, ownedMaterials);}

                ::FontAsset* RuntimeSceneAssetReferenceResolver::ResolveFont(::SceneAssetReference* reference)
                {
                const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
                ::FontAsset* cachedFontAsset;
                if (this->ActiveResolvedFontsByPath->TryGetValue(fullPath, cachedFontAsset))
                {
                return cachedFontAsset;    }
                ::FontAsset *fontAsset = this->AssetContentManager->Load<FontAsset*>(fullPath, RuntimeContentProcessorIds::FontAsset);
                this->TrackOwnedFont(fontAsset);
                return fontAsset;}

                ::RuntimeMaterial* RuntimeSceneAssetReferenceResolver::ResolveMaterial(::SceneAssetReference* reference)
                {
                if (reference->get_SourceKind() == SceneAssetReferenceSourceKind::Generated)
                {
                const std::string generatedAssetKey = this->BuildGeneratedAssetCacheKey(reference);
                ::RuntimeMaterial* generatedRuntimeMaterial;
                if (this->ActiveGeneratedMaterialsByKey->TryGetValue(generatedAssetKey, generatedRuntimeMaterial))
                {
                this->TrackOwnedMaterial(generatedRuntimeMaterial);
                return generatedRuntimeMaterial;    }
                const std::string generatedFullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedFullPath);
                this->ActiveGeneratedMaterialsByKey->Add(generatedAssetKey, generatedCookedRuntimeMaterial);
                this->TrackOwnedMaterial(generatedCookedRuntimeMaterial);
                return generatedCookedRuntimeMaterial;    }
                const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(fullPath);
                this->TrackOwnedMaterial(runtimeMaterial);
                return runtimeMaterial;}

                ::RuntimeModel* RuntimeSceneAssetReferenceResolver::ResolveModel(::SceneAssetReference* reference)
                {
                if (reference->get_SourceKind() == SceneAssetReferenceSourceKind::Generated)
                {
                const std::string generatedAssetKey = this->BuildGeneratedAssetCacheKey(reference);
                ::RuntimeModel* generatedRuntimeModel;
                if (this->ActiveGeneratedModelsByKey->TryGetValue(generatedAssetKey, generatedRuntimeModel))
                {
                this->TrackOwnedModel(generatedRuntimeModel);
                return generatedRuntimeModel;    }
                const std::string generatedFullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeModel *generatedModel = Core::get_Instance()->get_RenderManager3D()->BuildModelFromCooked(generatedFullPath);
                this->ActiveGeneratedModelsByKey->Add(generatedAssetKey, generatedModel);
                this->TrackOwnedModel(generatedModel);
                return generatedModel;    }
                const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeModel *runtimeModel = Core::get_Instance()->get_RenderManager3D()->BuildModelFromCooked(fullPath);
                this->TrackOwnedModel(runtimeModel);
                return runtimeModel;}

                ::RuntimeTexture* RuntimeSceneAssetReferenceResolver::ResolveTexture(::SceneAssetReference* reference)
                {
                const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
                ::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromCooked(fullPath);
                this->TrackOwnedTexture(runtimeTexture);
                return runtimeTexture;}
                """;
            File.WriteAllText(sourcePath, originalSource);

            GameCubeGeneratedCoreCompatibilityNormalizer normalizer = new();

            normalizer.Normalize(generatedCoreRootPath);

            string normalizedSource = File.ReadAllText(sourcePath);
            Assert.Contains("#include <ogc/system.h>", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("[GC] Resolver begin owned asset tracking", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("[GC] Resolver font load path=%s ptr=%p", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("[GC] Resolver material generated cache-hit key=%s ptr=%p", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("[GC] Resolver model load path=%s ptr=%p", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("[GC] Resolver texture build path=%s ptr=%p", normalizedSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }

    /// <summary>
    /// Ensures the normalizer rewrites the current generated null-conditional parent access that devkitPPC cannot compile.
    /// </summary>
    [Fact]
    public void Normalize_WhenPointerInteractableResolverUsesNullConditionalParent_RewritesSource() {
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "gamecube-generated-core-normalizer", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(generatedCoreRootPath);

        try {
            string sourcePath = Path.Combine(generatedCoreRootPath, "PointerInteractableHitResolver.cpp");
            string originalSource = """
                void PointerInteractableHitResolver::ResolvePointerInInteractableSpace(::IInteractable2D* interactable, ::ICamera* camera, int32_t pointerX, int32_t pointerY, int32_t& resolvedPointerX, int32_t& resolvedPointerY)
                {
                resolvedPointerX = pointerX;
                resolvedPointerY = pointerY;
                    if (interactable?.Parent == nullptr || camera == nullptr)
                    {
                return;    }
                }
                """;
            File.WriteAllText(sourcePath, originalSource);

            GameCubeGeneratedCoreCompatibilityNormalizer normalizer = new();

            normalizer.Normalize(generatedCoreRootPath);

            string normalizedSource = File.ReadAllText(sourcePath);
            Assert.Contains("if (interactable == nullptr || interactable->get_Parent() == nullptr || camera == nullptr)", normalizedSource, StringComparison.Ordinal);
            Assert.DoesNotContain("interactable?.Parent", normalizedSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }

}
