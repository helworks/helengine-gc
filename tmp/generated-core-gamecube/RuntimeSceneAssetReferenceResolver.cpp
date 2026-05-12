#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "ModelAsset.hpp"
#include "ContentManager.hpp"
#include "RuntimeModel.hpp"
#include "MaterialAsset.hpp"
#include "ShaderAsset.hpp"
#include "RuntimeMaterial.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "TextureAsset.hpp"
#include "RuntimeTexture.hpp"
#include "RenderManager2D.hpp"
#include "MaterialPropertyBlock.hpp"
#include "FontAsset.hpp"
#include "system/io/path.hpp"
#include "system/io/file.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "StandardMaterialTextureBindingDefaults.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "ShaderTargetNames.hpp"
#include "system/io/file.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/io/path.hpp"

::FontAsset* RuntimeSceneAssetReferenceResolver::ResolveFont(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
return this->AssetContentManager->Load<FontAsset*>(fullPath, RuntimeContentProcessorIds::FontAsset);}

::RuntimeMaterial* RuntimeSceneAssetReferenceResolver::ResolveMaterial(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);
::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);
::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);
this->ApplyMaterialDiffuseTexture(runtimeMaterial, materialAsset, fullPath);
return runtimeMaterial;}

::RuntimeModel* RuntimeSceneAssetReferenceResolver::ResolveModel(::SceneAssetReference* reference)
{
    if (reference == nullptr)
    {
throw new ArgumentNullException("reference");
    }
const std::string fullPath = this->ResolveFileBackedAssetPath(reference);
::ModelAsset *modelAsset = this->AssetContentManager->Load<ModelAsset*>(fullPath, RuntimeContentProcessorIds::ModelAsset);
return Core::get_Instance()->get_RenderManager3D()->BuildModelFromRaw(modelAsset);}

RuntimeSceneAssetReferenceResolver::RuntimeSceneAssetReferenceResolver(::ContentManager* assetContentManager, std::string contentRootPath, ::ShaderCompileTarget shaderTarget) : AssetContentManager(), ContentRootPath(), ShaderTarget()
{
    if (assetContentManager == nullptr)
    {
throw new ArgumentNullException("assetContentManager");
    }
    if (String::IsNullOrWhiteSpace(contentRootPath))
    {
throw ([&]() {
auto __ctor_arg_0000012B = "Content root path must be provided.";
auto __ctor_arg_0000012C = "contentRootPath";
return new ArgumentException(__ctor_arg_0000012B, __ctor_arg_0000012C);
})();
    }
this->ContentRootPath = Path::GetFullPath(contentRootPath);
this->AssetContentManager = assetContentManager;
this->ShaderTarget = shaderTarget;
}

std::string RuntimeSceneAssetReferenceResolver::FontDirectoryName = "fonts";

std::string RuntimeSceneAssetReferenceResolver::ImportedTextureDirectoryName = "cooked/imported";

std::string RuntimeSceneAssetReferenceResolver::ShaderDirectoryName = "cooked/shaders";

std::string RuntimeSceneAssetReferenceResolver::ShaderPackageExtension = ".hasset";

void RuntimeSceneAssetReferenceResolver::ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath)
{
    if (runtimeMaterial == nullptr)
    {
throw new ArgumentNullException("runtimeMaterial");
    }
    if (materialAsset == nullptr)
    {
throw new ArgumentNullException("materialAsset");
    }
    if (String::IsNullOrWhiteSpace(materialPath))
    {
throw ([&]() {
auto __ctor_arg_0000012D = "Material path must be provided.";
auto __ctor_arg_0000012E = "materialPath";
return new ArgumentException(__ctor_arg_0000012D, __ctor_arg_0000012E);
})();
    }
    if (String::IsNullOrWhiteSpace(materialAsset->DiffuseTextureAssetId))
    {
return;    }
std::string diffuseTexturePath;
    if (this->TryResolveSourceTexturePath(materialPath, materialAsset->DiffuseTextureAssetId, diffuseTexturePath))
    {
::TextureAsset *sourceTextureAsset = this->AssetContentManager->Load<TextureAsset*>(diffuseTexturePath, RuntimeContentProcessorIds::TextureAsset);
::RuntimeTexture *sourceRuntimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(sourceTextureAsset);
runtimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, sourceRuntimeTexture);
return;    }
diffuseTexturePath = this->ResolveImportedTexturePackagePath(materialAsset->DiffuseTextureAssetId);
::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(diffuseTexturePath, RuntimeContentProcessorIds::TextureAsset);
::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);
runtimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);
}

std::string RuntimeSceneAssetReferenceResolver::EnsureTrailingDirectorySeparator(std::string path)
{
    if (String::EndsWith(path, Path::DirectorySeparatorChar) || String::EndsWith(path, Path::AltDirectorySeparatorChar))
    {
return path;    }
return String::Concat(path, Path::DirectorySeparatorChar);}

std::string RuntimeSceneAssetReferenceResolver::ResolveFileBackedAssetPath(::SceneAssetReference* reference)
{
    if (reference->get_SourceKind() != SceneAssetReferenceSourceKind::FileSystem)
    {
throw new InvalidOperationException("Player builds currently require file-backed packaged scene references.");
    }
    if (String::IsNullOrWhiteSpace(reference->get_RelativePath()))
    {
throw new InvalidOperationException("Packaged scene asset references must include a relative path.");
    }
const std::string fullPath = Path::GetFullPath(Path::Combine(this->ContentRootPath, reference->get_RelativePath()));
const std::string contentRootPrefix = this->EnsureTrailingDirectorySeparator(this->ContentRootPath);
    if (!String::StartsWith(fullPath, contentRootPrefix, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException("Packaged scene asset reference path must stay inside the content root.");
    }
return fullPath;}

std::string RuntimeSceneAssetReferenceResolver::ResolveImportedTexturePackagePath(std::string assetId)
{
    if (String::IsNullOrWhiteSpace(assetId))
    {
throw new InvalidOperationException("Packaged material assets must include a diffuse texture asset id before resolving imported textures.");
    }
return Path::Combine(this->ContentRootPath, ImportedTextureDirectoryName, assetId);}

std::string RuntimeSceneAssetReferenceResolver::ResolveShaderPackagePath(std::string shaderAssetId)
{
    if (String::IsNullOrWhiteSpace(shaderAssetId))
    {
throw new InvalidOperationException("Packaged material assets must include a shader asset id.");
    }
const std::string fileName = String::Concat(shaderAssetId, ".", ShaderTargetNames::GetTargetName(this->ShaderTarget), ShaderPackageExtension);
return Path::Combine(this->ContentRootPath, ShaderDirectoryName, fileName);}

bool RuntimeSceneAssetReferenceResolver::TryResolveSourceTexturePath(std::string materialPath, std::string assetId, std::string& texturePath)
{
    if (String::IsNullOrWhiteSpace(materialPath))
    {
throw ([&]() {
auto __ctor_arg_0000012F = "Material path must be provided.";
auto __ctor_arg_00000130 = "materialPath";
return new ArgumentException(__ctor_arg_0000012F, __ctor_arg_00000130);
})();
    }
    if (String::IsNullOrWhiteSpace(assetId))
    {
texturePath = String::Empty;
return false;    }
const std::string materialDirectoryPath = Path::GetDirectoryName(Path::GetFullPath(materialPath));
    if (String::IsNullOrWhiteSpace(materialDirectoryPath))
    {
texturePath = String::Empty;
return false;    }
const std::string candidateTexturePath = Path::IsPathRooted(assetId) ? Path::GetFullPath(assetId) : Path::GetFullPath(Path::Combine(materialDirectoryPath, assetId));
    if (File::Exists(candidateTexturePath))
    {
texturePath = candidateTexturePath;
return true;    }
texturePath = String::Empty;
return false;}

