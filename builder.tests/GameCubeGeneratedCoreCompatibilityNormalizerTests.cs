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

    /// <summary>
    /// Ensures the normalizer rewrites generated file I/O so disc-backed GameCube paths flow through the platform file system.
    /// </summary>
    [Fact]
    public void Normalize_WhenGeneratedFileIoUsesHostFileAccess_RewritesSource() {
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "gamecube-generated-core-normalizer", Guid.NewGuid().ToString("N"));
        string systemIoRootPath = Path.Combine(generatedCoreRootPath, "system", "io");
        Directory.CreateDirectory(systemIoRootPath);

        try {
            string sourcePath = Path.Combine(systemIoRootPath, "file.cpp");
            string originalSource = """
                #include "file.hpp"

                bool File::Exists(const char* fileName) {
                	if (!fileName)
                	{
                		return false;
                	}

                	std::ifstream file(fileName);
                	return file.good();
                }

                FileStream* File::OpenRead(const char* filePath)
                {
                	return new FileStream(filePath, FileMode::Open, FileAccess::Read, FileShare::Read);
                }
                """;
            File.WriteAllText(sourcePath, originalSource);

            GameCubeGeneratedCoreCompatibilityNormalizer normalizer = new();

            normalizer.Normalize(generatedCoreRootPath);

            string normalizedSource = File.ReadAllText(sourcePath);
            Assert.Contains("#include \"platform/gamecube/GameCubeDiscFileSystem.hpp\"", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("GameCubeDiscFileSystem::CanHandlePath(fileName)", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("GameCubeDiscFileSystem::Exists(fileName)", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("GameCubeDiscFileSystem::OpenRead(filePath)", normalizedSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }

    /// <summary>
    /// Ensures the normalizer rewrites generated float4x4 math from the generic row-vector template to the GameCube matrix convention.
    /// </summary>
    [Fact]
    public void Normalize_WhenGeneratedFloat4x4UsesGenericTemplate_RewritesSource() {
        string generatedCoreRootPath = Path.Combine(Path.GetTempPath(), "gamecube-generated-core-normalizer", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(generatedCoreRootPath);

        try {
            string sourcePath = Path.Combine(generatedCoreRootPath, "float4x4.cpp");
            string originalSource = """
                void float4x4::CreateFromQuaternion(::float4& quaternion, ::float4x4& result)
                {
                const float num9 = quaternion.X * quaternion.X;
                const float num8 = quaternion.Y * quaternion.Y;
                const float num7 = quaternion.Z * quaternion.Z;
                const float num6 = quaternion.X * quaternion.Y;
                const float num5 = quaternion.Z * quaternion.W;
                const float num4 = quaternion.Z * quaternion.X;
                const float num3 = quaternion.Y * quaternion.W;
                const float num2 = quaternion.Y * quaternion.Z;
                const float num = quaternion.X * quaternion.W;
                result.M11 = 1.0f - (2.0f * (num8 + num7));
                result.M12 = 2.0f * (num6 + num5);
                result.M13 = 2.0f * (num4 - num3);
                result.M14 = 0.0f;
                result.M21 = 2.0f * (num6 - num5);
                result.M22 = 1.0f - (2.0f * (num7 + num9));
                result.M23 = 2.0f * (num2 + num);
                result.M24 = 0.0f;
                result.M31 = 2.0f * (num4 + num3);
                result.M32 = 2.0f * (num2 - num);
                result.M33 = 1.0f - (2.0f * (num8 + num9));
                result.M34 = 0.0f;
                result.M41 = 0.0f;
                result.M42 = 0.0f;
                result.M43 = 0.0f;
                result.M44 = 1.0f;
                }
                void float4x4::CreateLookAt(::float3& cameraPosition, ::float3& cameraTarget, ::float3& cameraUpVector, ::float4x4& result)
                {
                ::float3 vector = float3::Normalize(cameraPosition - cameraTarget);
                ::float3 vector2 = float3::Normalize(float3::Cross(cameraUpVector, vector));
                ::float3 vector3 = float3::Cross(vector, vector2);
                result.M11 = vector2.X;
                result.M12 = vector3.X;
                result.M13 = vector.X;
                result.M14 = 0.0f;
                result.M21 = vector2.Y;
                result.M22 = vector3.Y;
                result.M23 = vector.Y;
                result.M24 = 0.0f;
                result.M31 = vector2.Z;
                result.M32 = vector3.Z;
                result.M33 = vector.Z;
                result.M34 = 0.0f;
                result.M41 = -float3::Dot(vector2, cameraPosition);
                result.M42 = -float3::Dot(vector3, cameraPosition);
                result.M43 = -float3::Dot(vector, cameraPosition);
                result.M44 = 1.0f;
                }
                void float4x4::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane, ::float4x4& result)
                {
                result.M11 = static_cast<float>((2.0 / (static_cast<double>(right) - static_cast<double>(left))));
                result.M12 = 0.0f;
                result.M13 = 0.0f;
                result.M14 = 0.0f;
                result.M21 = 0.0f;
                result.M22 = static_cast<float>((2.0 / (static_cast<double>(top) - static_cast<double>(bottom))));
                result.M23 = 0.0f;
                result.M24 = 0.0f;
                result.M31 = 0.0f;
                result.M32 = 0.0f;
                result.M33 = static_cast<float>((1.0 / (static_cast<double>(zNearPlane) - static_cast<double>(zFarPlane))));
                result.M34 = 0.0f;
                result.M41 = static_cast<float>(((static_cast<double>(left) + static_cast<double>(right)) / (static_cast<double>(left) - static_cast<double>(right))));
                result.M42 = static_cast<float>(((static_cast<double>(top) + static_cast<double>(bottom)) / (static_cast<double>(bottom) - static_cast<double>(top))));
                result.M43 = static_cast<float>((static_cast<double>(zNearPlane) / (static_cast<double>(zNearPlane) - static_cast<double>(zFarPlane))));
                result.M44 = 1.0f;
                }
                void float4x4::CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance, ::float4x4& result)
                {
                    if ((fieldOfView <= 0.0f) || (fieldOfView >= 3.141593f))
                    {
                throw new ArgumentException("fieldOfView <= 0 or >= PI");
                    }
                    if (nearPlaneDistance <= 0.0f)
                    {
                throw new ArgumentException("nearPlaneDistance <= 0");
                    }
                    if (farPlaneDistance <= 0.0f)
                    {
                throw new ArgumentException("farPlaneDistance <= 0");
                    }
                    if (nearPlaneDistance >= farPlaneDistance)
                    {
                throw new ArgumentException("nearPlaneDistance >= farPlaneDistance");
                    }
                float yScale = 1.0f / static_cast<float>(Math::Tan(static_cast<double>(fieldOfView) * 0.5f));
                float xScale = yScale / aspectRatio;
                float negFarRange = Number::IsPositiveInfinity(farPlaneDistance) ? -1.0f : farPlaneDistance / (nearPlaneDistance - farPlaneDistance);
                result.M11 = xScale;
                result.M12 = result.M13 = result.M14 = 0.0f;
                result.M22 = yScale;
                result.M21 = result.M23 = result.M24 = 0.0f;
                result.M31 = result.M32 = 0.0f;
                result.M33 = negFarRange;
                result.M34 = -1.0f;
                result.M41 = result.M42 = result.M44 = 0.0f;
                result.M43 = nearPlaneDistance * negFarRange;
                }
                void float4x4::CreateTranslation(float x, float y, float z, ::float4x4& result)
                {
                result.M11 = 1;
                result.M12 = 0;
                result.M13 = 0;
                result.M14 = 0;
                result.M21 = 0;
                result.M22 = 1;
                result.M23 = 0;
                result.M24 = 0;
                result.M31 = 0;
                result.M32 = 0;
                result.M33 = 1;
                result.M34 = 0;
                result.M41 = x;
                result.M42 = y;
                result.M43 = z;
                result.M44 = 1;
                }
                void float4x4::CreateTranslation(::float3& position, ::float4x4& result)
                {
                result.M11 = 1;
                result.M12 = 0;
                result.M13 = 0;
                result.M14 = 0;
                result.M21 = 0;
                result.M22 = 1;
                result.M23 = 0;
                result.M24 = 0;
                result.M31 = 0;
                result.M32 = 0;
                result.M33 = 1;
                result.M34 = 0;
                result.M41 = position.X;
                result.M42 = position.Y;
                result.M43 = position.Z;
                result.M44 = 1;
                }
                void float4x4::Multiply(::float4x4& matrix1, ::float4x4& matrix2, ::float4x4& result)
                {
                float m11 = (((matrix1.M11 * matrix2.M11) + (matrix1.M12 * matrix2.M21)) + (matrix1.M13 * matrix2.M31)) + (matrix1.M14 * matrix2.M41);
                float m12 = (((matrix1.M11 * matrix2.M12) + (matrix1.M12 * matrix2.M22)) + (matrix1.M13 * matrix2.M32)) + (matrix1.M14 * matrix2.M42);
                float m13 = (((matrix1.M11 * matrix2.M13) + (matrix1.M12 * matrix2.M23)) + (matrix1.M13 * matrix2.M33)) + (matrix1.M14 * matrix2.M43);
                float m14 = (((matrix1.M11 * matrix2.M14) + (matrix1.M12 * matrix2.M24)) + (matrix1.M13 * matrix2.M34)) + (matrix1.M14 * matrix2.M44);
                float m21 = (((matrix1.M21 * matrix2.M11) + (matrix1.M22 * matrix2.M21)) + (matrix1.M23 * matrix2.M31)) + (matrix1.M24 * matrix2.M41);
                float m22 = (((matrix1.M21 * matrix2.M12) + (matrix1.M22 * matrix2.M22)) + (matrix1.M23 * matrix2.M32)) + (matrix1.M24 * matrix2.M42);
                float m23 = (((matrix1.M21 * matrix2.M13) + (matrix1.M22 * matrix2.M23)) + (matrix1.M23 * matrix2.M33)) + (matrix1.M24 * matrix2.M43);
                float m24 = (((matrix1.M21 * matrix2.M14) + (matrix1.M22 * matrix2.M24)) + (matrix1.M23 * matrix2.M34)) + (matrix1.M24 * matrix2.M44);
                float m31 = (((matrix1.M31 * matrix2.M11) + (matrix1.M32 * matrix2.M21)) + (matrix1.M33 * matrix2.M31)) + (matrix1.M34 * matrix2.M41);
                float m32 = (((matrix1.M31 * matrix2.M12) + (matrix1.M32 * matrix2.M22)) + (matrix1.M33 * matrix2.M32)) + (matrix1.M34 * matrix2.M42);
                float m33 = (((matrix1.M31 * matrix2.M13) + (matrix1.M32 * matrix2.M23)) + (matrix1.M33 * matrix2.M33)) + (matrix1.M34 * matrix2.M43);
                float m34 = (((matrix1.M31 * matrix2.M14) + (matrix1.M32 * matrix2.M24)) + (matrix1.M33 * matrix2.M34)) + (matrix1.M34 * matrix2.M44);
                float m41 = (((matrix1.M41 * matrix2.M11) + (matrix1.M42 * matrix2.M21)) + (matrix1.M43 * matrix2.M31)) + (matrix1.M44 * matrix2.M41);
                float m42 = (((matrix1.M41 * matrix2.M12) + (matrix1.M42 * matrix2.M22)) + (matrix1.M43 * matrix2.M32)) + (matrix1.M44 * matrix2.M42);
                float m43 = (((matrix1.M41 * matrix2.M13) + (matrix1.M42 * matrix2.M23)) + (matrix1.M43 * matrix2.M33)) + (matrix1.M44 * matrix2.M43);
                float m44 = (((matrix1.M41 * matrix2.M14) + (matrix1.M42 * matrix2.M24)) + (matrix1.M43 * matrix2.M34)) + (matrix1.M44 * matrix2.M44);
                result.M11 = m11;
                result.M12 = m12;
                result.M13 = m13;
                result.M14 = m14;
                result.M21 = m21;
                result.M22 = m22;
                result.M23 = m23;
                result.M24 = m24;
                result.M31 = m31;
                result.M32 = m32;
                result.M33 = m33;
                result.M34 = m34;
                result.M41 = m41;
                result.M42 = m42;
                result.M43 = m43;
                result.M44 = m44;
                }
                """;
            File.WriteAllText(sourcePath, originalSource);

            GameCubeGeneratedCoreCompatibilityNormalizer normalizer = new();

            normalizer.Normalize(generatedCoreRootPath);

            string normalizedSource = File.ReadAllText(sourcePath);
            Assert.Contains("result.M12 = 2.0f * (num6 - num5);", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M14 = -float3::Dot(vector2, cameraPosition);", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M34 = static_cast<float>((static_cast<double>(zNearPlane) / (static_cast<double>(zNearPlane) - static_cast<double>(zFarPlane))));", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M33 = Number::IsPositiveInfinity(farPlaneDistance) ? 0.0f : nearPlaneDistance / (nearPlaneDistance - farPlaneDistance);", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M34 = nearPlaneDistance * (farPlaneDistance / (nearPlaneDistance - farPlaneDistance));", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M43 = -1.0f;", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M14 = x;", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M24 = y;", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("result.M34 = z;", normalizedSource, StringComparison.Ordinal);
            Assert.Contains("float m11 = (((matrix2.M11 * matrix1.M11) + (matrix2.M12 * matrix1.M21)) + (matrix2.M13 * matrix1.M31)) + (matrix2.M14 * matrix1.M41);", normalizedSource, StringComparison.Ordinal);
            Assert.DoesNotContain("float m11 = (((matrix1.M11 * matrix2.M11) + (matrix1.M12 * matrix2.M21)) + (matrix1.M13 * matrix2.M31)) + (matrix1.M14 * matrix2.M41);", normalizedSource, StringComparison.Ordinal);
            Assert.DoesNotContain("result.M41 = x;", normalizedSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(generatedCoreRootPath)) {
                Directory.Delete(generatedCoreRootPath, true);
            }
        }
    }
}
