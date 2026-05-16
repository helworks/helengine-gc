namespace helengine.gamecube.builder;

/// <summary>
/// Rewrites regenerated GameCube core sources so packaged builds use cooked platform-owned materials instead of raw shader packages.
/// </summary>
public sealed class GameCubeGeneratedCoreCompatibilityNormalizer {
    /// <summary>
    /// Normalizes the generated-core files required by the GameCube packaged runtime material contract.
    /// </summary>
    /// <param name="generatedCoreRootPath">Generated-core root consumed by the Docker build.</param>
    public void Normalize(string generatedCoreRootPath) {
        if (string.IsNullOrWhiteSpace(generatedCoreRootPath)) {
            throw new ArgumentException("Generated core root path must be provided.", nameof(generatedCoreRootPath));
        }

        if (!Directory.Exists(generatedCoreRootPath)) {
            throw new InvalidOperationException("Generated core root path was not found.");
        }

        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RuntimeContentManagerConfiguration.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RuntimeSceneAssetReferenceResolver.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RenderManager3D.hpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RenderManager3D.cpp");
    }

    /// <summary>
    /// Applies one GameCube-specific normalization to one generated-core source file.
    /// </summary>
    /// <param name="generatedCoreRootPath">Generated-core root consumed by the Docker build.</param>
    /// <param name="relativePath">Relative generated-core file path.</param>
    static void NormalizeGeneratedCoreFile(string generatedCoreRootPath, string relativePath) {
        string sourcePath = Path.Combine(generatedCoreRootPath, relativePath);
        if (!File.Exists(sourcePath)) {
            return;
        }

        string originalContents = File.ReadAllText(sourcePath);
        string normalizedContents = NormalizeGeneratedCoreSource(relativePath, originalContents);
        if (!string.Equals(originalContents, normalizedContents, StringComparison.Ordinal)) {
            File.WriteAllText(sourcePath, normalizedContents);
        }
    }

    /// <summary>
    /// Applies one GameCube-specific normalization to one generated-core source buffer.
    /// </summary>
    /// <param name="relativePath">Relative generated-core file path.</param>
    /// <param name="contents">Generated-core source contents.</param>
    /// <returns>Normalized generated-core source contents.</returns>
    static string NormalizeGeneratedCoreSource(string relativePath, string contents) {
        if (string.IsNullOrWhiteSpace(relativePath)) {
            throw new ArgumentException("Generated-core relative path must be provided.", nameof(relativePath));
        } else if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        string normalizedContents = contents.Replace("\r\n", "\n", StringComparison.Ordinal);

        if (string.Equals(relativePath, "RuntimeContentManagerConfiguration.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeRuntimeContentManagerConfigurationSource(normalizedContents);
        } else if (string.Equals(relativePath, "RuntimeSceneAssetReferenceResolver.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeRuntimeSceneAssetReferenceResolverSource(normalizedContents);
        } else if (string.Equals(relativePath, "RenderManager3D.hpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeRenderManager3DHeaderSource(normalizedContents);
        } else if (string.Equals(relativePath, "RenderManager3D.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeRenderManager3DSource(normalizedContents);
        }

        return normalizedContents;
    }

    /// <summary>
    /// Rewrites generated runtime content-manager registration to load cooked platform-owned material assets.
    /// </summary>
    /// <param name="contents">Generated runtime content-manager source.</param>
    /// <returns>Normalized runtime content-manager source.</returns>
    static string NormalizeRuntimeContentManagerConfigurationSource(string contents) {
        if (contents.Contains("RegisterProcessorIfMissing<PlatformMaterialAsset*>", StringComparison.Ordinal)) {
            return contents;
        }

        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include \"PlatformMaterialAsset.hpp\"", StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                "#include \"MaterialAsset.hpp\"\n",
                "#include \"MaterialAsset.hpp\"\n#include \"PlatformMaterialAsset.hpp\"\n",
                StringComparison.Ordinal);
        }

        return ReplaceRequired(
            normalizedContents,
            "RegisterProcessorIfMissing<MaterialAsset*>(contentManager, RuntimeContentProcessorIds::MaterialAsset, new ::AssetContentProcessor_1<::MaterialAsset*>(), new Array<std::string>({ MaterialAssetExtension }));",
            "RegisterProcessorIfMissing<PlatformMaterialAsset*>(contentManager, RuntimeContentProcessorIds::MaterialAsset, new ::AssetContentProcessor_1<::PlatformMaterialAsset*>(), new Array<std::string>({ MaterialAssetExtension }));",
            "GameCube generated runtime content manager should register cooked platform materials through PlatformMaterialAsset.");
    }

    /// <summary>
    /// Rewrites generated scene material resolution from raw material-plus-shader loading to cooked platform-owned material loading.
    /// </summary>
    /// <param name="contents">Generated runtime scene resolver source.</param>
    /// <returns>Normalized runtime scene resolver source.</returns>
    static string NormalizeRuntimeSceneAssetReferenceResolverSource(string contents) {
        if (contents.Contains("BuildMaterialFromCooked(materialAsset)", StringComparison.Ordinal)
            && contents.Contains("PlatformMaterialAsset *materialAsset", StringComparison.Ordinal)
            && contents.Contains("materialAsset->TextureRelativePath", StringComparison.Ordinal)) {
            return contents;
        }

        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include \"PlatformMaterialAsset.hpp\"", StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                "#include \"MaterialAsset.hpp\"\n",
                "#include \"MaterialAsset.hpp\"\n#include \"PlatformMaterialAsset.hpp\"\n",
                StringComparison.Ordinal);
        }

        const string cookedMaterialBlockWithoutTextureBinding = "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nreturn runtimeMaterial;}";
        if (normalizedContents.Contains(cookedMaterialBlockWithoutTextureBinding, StringComparison.Ordinal)) {
            return normalizedContents.Replace(
                cookedMaterialBlockWithoutTextureBinding,
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
                StringComparison.Ordinal);
        }

        string modernRawMaterialBlock = "::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nthis->ApplyMaterialDiffuseTexture(runtimeMaterial, materialAsset, fullPath);\nreturn runtimeMaterial;}";
        if (normalizedContents.Contains(modernRawMaterialBlock, StringComparison.Ordinal)) {
            return normalizedContents.Replace(
                modernRawMaterialBlock,
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
                StringComparison.Ordinal);
        }

        string fallbackNormalizedContents = normalizedContents
            .Replace(
                "::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);",
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);",
                StringComparison.Ordinal)
            .Replace(
                "::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);\n",
                string.Empty,
                StringComparison.Ordinal)
            .Replace(
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);",
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);",
                StringComparison.Ordinal)
            .Replace(
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, nullptr);",
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);",
                StringComparison.Ordinal)
            .Replace(
                "this->ApplyMaterialDiffuseTexture(runtimeMaterial, materialAsset, fullPath);\n",
                "if (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\n",
                StringComparison.Ordinal);
        if (string.Equals(fallbackNormalizedContents, contents, StringComparison.Ordinal)) {
            throw new InvalidOperationException("GameCube generated runtime scene asset resolver should resolve materials through the cooked platform material contract.");
        }

        return fallbackNormalizedContents;
    }

    /// <summary>
    /// Rewrites the generated render-manager header so the cooked platform-owned material contract exists for GameCube.
    /// </summary>
    /// <param name="contents">Generated render-manager header source.</param>
    /// <returns>Normalized generated render-manager header source.</returns>
    static string NormalizeRenderManager3DHeaderSource(string contents) {
        if (contents.Contains("virtual ::RuntimeMaterial* BuildMaterialFromCooked(::PlatformMaterialAsset* materialAsset);", StringComparison.Ordinal)) {
            return contents;
        }

        string normalizedContents = contents;
        if (!normalizedContents.Contains("class PlatformMaterialAsset;", StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                "class MaterialAsset;\n",
                "class MaterialAsset;\nclass PlatformMaterialAsset;\n",
                StringComparison.Ordinal);
        }

        return ReplaceRequired(
            normalizedContents,
            "    virtual ::RuntimeMaterial* BuildMaterialFromRaw(::MaterialAsset* materialAsset, ::ShaderAsset* shaderAsset);\n",
            "    virtual ::RuntimeMaterial* BuildMaterialFromCooked(::PlatformMaterialAsset* materialAsset);\n\n    virtual ::RuntimeMaterial* BuildMaterialFromRaw(::MaterialAsset* materialAsset, ::ShaderAsset* shaderAsset);\n",
            "GameCube generated RenderManager3D.hpp should expose the cooked platform material contract.");
    }

    /// <summary>
    /// Rewrites the generated render-manager source so the cooked platform-owned material contract has a default implementation.
    /// </summary>
    /// <param name="contents">Generated render-manager source.</param>
    /// <returns>Normalized generated render-manager source.</returns>
    static string NormalizeRenderManager3DSource(string contents) {
        if (contents.Contains("BuildMaterialFromCooked(::PlatformMaterialAsset* materialAsset)", StringComparison.Ordinal)) {
            return contents;
        }

        const string cookedImplementation = """
::RuntimeMaterial* RenderManager3D::BuildMaterialFromCooked(::PlatformMaterialAsset* materialAsset)
{
throw new NotSupportedException("This renderer does not support platform-owned cooked material creation.");
}

""";

        return ReplaceRequired(
            contents,
            "::RuntimeMaterial* RenderManager3D::BuildMaterialFromRaw(::MaterialAsset* materialAsset, ::ShaderAsset* shaderAsset)\n",
            cookedImplementation + "::RuntimeMaterial* RenderManager3D::BuildMaterialFromRaw(::MaterialAsset* materialAsset, ::ShaderAsset* shaderAsset)\n",
            "GameCube generated RenderManager3D.cpp should expose the cooked platform material default implementation.");
    }

    /// <summary>
    /// Replaces one required generated-source fragment and throws when the expected source shape is absent.
    /// </summary>
    /// <param name="contents">Generated source contents to rewrite.</param>
    /// <param name="oldValue">Exact generated fragment that must exist.</param>
    /// <param name="newValue">Replacement fragment written when the expected fragment exists.</param>
    /// <param name="failureMessage">Detailed failure message describing the missing generated-source contract.</param>
    /// <returns>Rewritten generated source.</returns>
    static string ReplaceRequired(string contents, string oldValue, string newValue, string failureMessage) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        } else if (oldValue == null) {
            throw new ArgumentNullException(nameof(oldValue));
        } else if (newValue == null) {
            throw new ArgumentNullException(nameof(newValue));
        } else if (string.IsNullOrWhiteSpace(failureMessage)) {
            throw new ArgumentException("Failure message must be provided.", nameof(failureMessage));
        } else if (!contents.Contains(oldValue, StringComparison.Ordinal)) {
            throw new InvalidOperationException(failureMessage);
        }

        return contents.Replace(oldValue, newValue, StringComparison.Ordinal);
    }
}
