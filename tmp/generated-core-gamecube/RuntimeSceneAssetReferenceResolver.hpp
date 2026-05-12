#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ModelAsset;
class ContentManager;
class RuntimeModel;
class MaterialAsset;
class ShaderAsset;
class RuntimeMaterial;
class Core;
class RenderManager3D;
class TextureAsset;
class RuntimeTexture;
class RenderManager2D;
class MaterialPropertyBlock;
class FontAsset;
class SceneAssetReference;

#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "ModelAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeModel.hpp"
#include "MaterialAsset.hpp"
#include "ShaderAsset.hpp"
#include "RuntimeMaterial.hpp"
#include "Core.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "RuntimeMaterial.hpp"
#include "runtime/native_exceptions.hpp"
#include "TextureAsset.hpp"
#include "RuntimeTexture.hpp"
#include "RenderManager2D.hpp"
#include "MaterialPropertyBlock.hpp"
#include "FontAsset.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/path.hpp"
#include "system/io/path.hpp"
#include "system/io/file.hpp"
#include "system/io/file.hpp"
#include "ContentManager.hpp"
#include "ShaderCompileTarget.hpp"
#include "FontAsset.hpp"
#include "SceneAssetReference.hpp"
#include "RuntimeModel.hpp"

class RuntimeSceneAssetReferenceResolver
{
public:
    virtual ~RuntimeSceneAssetReferenceResolver() = default;

    ::FontAsset* ResolveFont(::SceneAssetReference* reference);

    ::RuntimeMaterial* ResolveMaterial(::SceneAssetReference* reference);

    ::RuntimeModel* ResolveModel(::SceneAssetReference* reference);

    RuntimeSceneAssetReferenceResolver(::ContentManager* assetContentManager, std::string contentRootPath, ::ShaderCompileTarget shaderTarget);
private:
    static std::string FontDirectoryName;

    static std::string ImportedTextureDirectoryName;

    static std::string ShaderDirectoryName;

    static std::string ShaderPackageExtension;

    ::ContentManager* AssetContentManager;

    std::string ContentRootPath;

    ::ShaderCompileTarget ShaderTarget;

    void ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath);

    std::string EnsureTrailingDirectorySeparator(std::string path);

    std::string ResolveFileBackedAssetPath(::SceneAssetReference* reference);

    std::string ResolveImportedTexturePackagePath(std::string assetId);

    std::string ResolveShaderPackagePath(std::string shaderAssetId);

    bool TryResolveSourceTexturePath(std::string materialPath, std::string assetId, std::string& texturePath);
};
