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
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "ContentManager.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "SceneManager.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RenderManager3D.hpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RenderManager3D.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "FPSComponent.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "Entity.hpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "Entity.cpp");
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
        } else if (string.Equals(relativePath, "ContentManager.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeContentManagerSource(normalizedContents);
        } else if (string.Equals(relativePath, "SceneManager.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeSceneManagerSource(normalizedContents);
        } else if (string.Equals(relativePath, "RenderManager3D.hpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeRenderManager3DHeaderSource(normalizedContents);
        } else if (string.Equals(relativePath, "RenderManager3D.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeRenderManager3DSource(normalizedContents);
        } else if (string.Equals(relativePath, "FPSComponent.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeFpsComponentSource(normalizedContents);
        } else if (string.Equals(relativePath, "Entity.hpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeEntityHeaderSource(normalizedContents);
        } else if (string.Equals(relativePath, "Entity.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeEntitySource(normalizedContents);
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
            && contents.Contains("materialAsset->TextureRelativePath", StringComparison.Ordinal)
            && contents.Contains("delete textureAsset;", StringComparison.Ordinal)) {
            return contents;
        }

        string normalizedContents = contents;
        string cookedGeneratedMaterialBlock = "::PlatformMaterialAsset *generatedPlatformMaterialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(generatedFullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedPlatformMaterialAsset);\nthis->ActiveGeneratedMaterialsByKey->Add(generatedAssetKey, generatedCookedRuntimeMaterial);\nreturn generatedCookedRuntimeMaterial;    }";
        if (normalizedContents.Contains(cookedGeneratedMaterialBlock, StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                cookedGeneratedMaterialBlock,
                "::PlatformMaterialAsset *generatedPlatformMaterialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(generatedFullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedPlatformMaterialAsset);\nif (!String::IsNullOrWhiteSpace(generatedPlatformMaterialAsset->TextureRelativePath)) {\n::TextureAsset *generatedTextureAsset = this->AssetContentManager->Load<TextureAsset*>(generatedPlatformMaterialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *generatedRuntimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(generatedTextureAsset);\nif (generatedTextureAsset->Colors != nullptr && generatedTextureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete generatedTextureAsset->Colors;\ngeneratedTextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (generatedTextureAsset->PaletteColors != nullptr && generatedTextureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete generatedTextureAsset->PaletteColors;\ngeneratedTextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete generatedTextureAsset;\nthis->TrackOwnedTexture(generatedRuntimeTexture);\ngeneratedCookedRuntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, generatedRuntimeTexture);\n}\nthis->ActiveGeneratedMaterialsByKey->Add(generatedAssetKey, generatedCookedRuntimeMaterial);\nreturn generatedCookedRuntimeMaterial;    }",
                StringComparison.Ordinal);
        }

        string cookedMaterialReturnBlock = "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\nreturn Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);}";
        if (normalizedContents.Contains(cookedMaterialReturnBlock, StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                cookedMaterialReturnBlock,
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
                StringComparison.Ordinal);
        }

        if (normalizedContents.Contains("materialAsset->TextureRelativePath", StringComparison.Ordinal)
            && normalizedContents.Contains("delete textureAsset;", StringComparison.Ordinal)
            && normalizedContents.Contains("generatedPlatformMaterialAsset->TextureRelativePath", StringComparison.Ordinal)) {
            return normalizedContents;
        }

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
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
                StringComparison.Ordinal);
        }

        string modernRawMaterialBlock = "::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nthis->ApplyMaterialDiffuseTexture(runtimeMaterial, materialAsset, fullPath);\nreturn runtimeMaterial;}";
        if (normalizedContents.Contains(modernRawMaterialBlock, StringComparison.Ordinal)) {
            return normalizedContents.Replace(
                modernRawMaterialBlock,
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
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
                "if (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\n",
                StringComparison.Ordinal);
        fallbackNormalizedContents = fallbackNormalizedContents
            .Replace(
                "::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(fullPath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nthis->TrackOwnedTexture(runtimeTexture);\nreturn runtimeTexture;}",
                "::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(fullPath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nreturn runtimeTexture;}",
                StringComparison.Ordinal)
            .Replace(
                "::TextureAsset *sourceTextureAsset = this->AssetContentManager->Load<TextureAsset*>(diffuseTexturePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *sourceRuntimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(sourceTextureAsset);\nthis->TrackOwnedTexture(sourceRuntimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, sourceRuntimeTexture);\nreturn;    }",
                "::TextureAsset *sourceTextureAsset = this->AssetContentManager->Load<TextureAsset*>(diffuseTexturePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *sourceRuntimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(sourceTextureAsset);\nif (sourceTextureAsset->Colors != nullptr && sourceTextureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete sourceTextureAsset->Colors;\nsourceTextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (sourceTextureAsset->PaletteColors != nullptr && sourceTextureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete sourceTextureAsset->PaletteColors;\nsourceTextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete sourceTextureAsset;\nthis->TrackOwnedTexture(sourceRuntimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, sourceRuntimeTexture);\nreturn;    }",
                StringComparison.Ordinal)
            .Replace(
                "::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(diffuseTexturePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}",
                "::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(diffuseTexturePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}",
                StringComparison.Ordinal);
        if (string.Equals(fallbackNormalizedContents, contents, StringComparison.Ordinal)) {
            throw new InvalidOperationException("GameCube generated runtime scene asset resolver should resolve materials through the cooked platform material contract.");
        }

        return fallbackNormalizedContents;
    }

    /// <summary>
    /// Rewrites generated content-manager loads so GameCube packaged builds log each resolved asset path.
    /// </summary>
    /// <param name="contents">Generated content-manager source.</param>
    /// <returns>Normalized generated content-manager source.</returns>
    static string NormalizeContentManagerSource(string contents) {
        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include <ogc/system.h>", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "#include \"system/string_comparer.hpp\"\n",
                "#include \"system/string_comparer.hpp\"\n#include <ogc/system.h>\n",
                "GameCube generated ContentManager.cpp should include SYS_Report support for load diagnostics.");
        }

        if (normalizedContents.Contains("SYS_Report(\"[GC] ContentManager opening asset: %s\\n\", fullPath.c_str());", StringComparison.Ordinal)) {
            return normalizedContents;
        }

        return ReplaceRequired(
            normalizedContents,
            "{\n::FileStream *stream = File::OpenRead(fullPath);\n",
            "{\nSYS_Report(\"[GC] ContentManager opening asset: %s\\n\", fullPath.c_str());\n::FileStream *stream = File::OpenRead(fullPath);\n",
            "GameCube generated ContentManager.cpp should log resolved asset paths before opening packaged content.");
    }

    /// <summary>
    /// Rewrites generated scene-manager loads so GameCube packaged builds record scene-load start times for first-draw timing logs.
    /// </summary>
    /// <param name="contents">Generated scene-manager source.</param>
    /// <returns>Normalized generated scene-manager source.</returns>
    static string NormalizeSceneManagerSource(string contents) {
        string normalizedContents = contents;
        if (!normalizedContents.Contains("extern \"C\" void GameCubeRecordSceneLoadRequest(const char* sceneId);", StringComparison.Ordinal)) {
            if (normalizedContents.Contains("#include \"SceneManager.hpp\"\n", StringComparison.Ordinal)) {
                normalizedContents = normalizedContents.Replace(
                    "#include \"SceneManager.hpp\"\n",
                    "#include \"SceneManager.hpp\"\nextern \"C\" void GameCubeRecordSceneLoadRequest(const char* sceneId);\n",
                    StringComparison.Ordinal);
            } else if (normalizedContents.Contains("#include <ogc/system.h>\n", StringComparison.Ordinal)) {
                normalizedContents = ReplaceRequired(
                    normalizedContents,
                    "#include <ogc/system.h>\n",
                    "#include <ogc/system.h>\nextern \"C\" void GameCubeRecordSceneLoadRequest(const char* sceneId);\n",
                    "GameCube generated SceneManager.cpp should declare the native scene-load timing hook.");
            } else {
                throw new InvalidOperationException("GameCube generated SceneManager.cpp should declare the native scene-load timing hook.");
            }
        }

        if (normalizedContents.Contains("GameCubeRecordSceneLoadRequest(sceneId.c_str());", StringComparison.Ordinal)) {
            return normalizedContents;
        }

        if (normalizedContents.Contains("this->RecordTraceState(\"LoadSceneImmediateBegin\", sceneId);\n", StringComparison.Ordinal)) {
            return normalizedContents.Replace(
                "this->RecordTraceState(\"LoadSceneImmediateBegin\", sceneId);\n",
                "this->RecordTraceState(\"LoadSceneImmediateBegin\", sceneId);\nGameCubeRecordSceneLoadRequest(sceneId.c_str());\n",
                StringComparison.Ordinal);
        }

        return ReplaceRequired(
            normalizedContents,
            "        ? static_cast<long>(this->SceneCatalog->get_Entries()->get_Length())\n        : -1L);\n",
            "        ? static_cast<long>(this->SceneCatalog->get_Entries()->get_Length())\n        : -1L);\nGameCubeRecordSceneLoadRequest(sceneId.c_str());\n",
            "GameCube generated SceneManager.cpp should record scene-load start times immediately after scene-load requests are logged.");
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
    /// Rewrites the generated FPS overlay teardown so packaged scene transitions dispose the overlay subtree instead of only detaching it.
    /// </summary>
    /// <param name="contents">Generated FPS component source.</param>
    /// <returns>Normalized generated FPS component source.</returns>
    static string NormalizeFpsComponentSource(string contents) {
        if (contents.Contains("this->OverlayHost->Dispose();", StringComparison.Ordinal)) {
            return contents;
        }

        return ReplaceRequired(
            contents,
            "    if (Parent != nullptr && this->OverlayHost != nullptr && this->OverlayHost->get_Parent() == Parent)\n    {\nParent->RemoveChild(this->OverlayHost);\n    }\n",
            "    if (this->OverlayHost != nullptr)\n    {\nthis->OverlayHost->Dispose();\n    }\n",
            "GameCube generated FPSComponent.cpp should dispose the overlay subtree during teardown.");
    }

    /// <summary>
    /// Rewrites the generated entity header so disposal-aware detach suppression state exists on GameCube builds.
    /// </summary>
    /// <param name="contents">Generated entity header source.</param>
    /// <returns>Normalized generated entity header source.</returns>
    static string NormalizeEntityHeaderSource(string contents) {
        string normalizedContents = contents;

        if (!normalizedContents.Contains("bool isDisposing;", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "private:\n    bool isEnabled;\n",
                "private:\n    bool isDisposing;\n\n    bool isEnabled;\n",
                "GameCube generated Entity.hpp should store the disposal guard state.");
        }

        if (!normalizedContents.Contains("bool ShouldSuppressRegistrationRefreshForDetachment(::Entity* entity);", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "    void RefreshRegistrationsAfterParentChange();\n",
                "    bool ShouldSuppressRegistrationRefreshForDetachment(::Entity* entity);\n\n    void RefreshRegistrationsAfterParentChange();\n",
                "GameCube generated Entity.hpp should declare the disposal-aware detach suppression helper.");
        }

        return normalizedContents;
    }

    /// <summary>
    /// Rewrites the generated entity teardown so disposal-time detach does not refresh registrations against stale scene state.
    /// </summary>
    /// <param name="contents">Generated entity source.</param>
    /// <returns>Normalized generated entity source.</returns>
    static string NormalizeEntitySource(string contents) {
        string normalizedContents = contents;

        if (normalizedContents.Contains("Entity::Entity() : Children(), Components(), LayerMask(), Parent(), isEnabled(), isInitialized(), isStatic(), orientation(), position(), scale()", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "Entity::Entity() : Children(), Components(), LayerMask(), Parent(), isEnabled(), isInitialized(), isStatic(), orientation(), position(), scale()\n",
                "Entity::Entity() : Children(), Components(), LayerMask(), Parent(), isDisposing(), isEnabled(), isInitialized(), isStatic(), orientation(), position(), scale()\n",
                "GameCube generated Entity.cpp constructor should initialize the disposal guard state.");
        }

        if (!normalizedContents.Contains("bool Entity::ShouldSuppressRegistrationRefreshForDetachment(::Entity* entity)", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "void Entity::RefreshRegistrationsAfterParentChange()\n{\n    if (!this->get_IsHierarchyEnabled() || Core::get_Instance() == nullptr || Core::get_Instance()->get_ObjectManager() == nullptr)\n    {\nreturn;    }\nthis->RefreshRegistrationsAfterParentChangeRecursive(this);\n}\n\n",
                "bool Entity::ShouldSuppressRegistrationRefreshForDetachment(::Entity* entity)\n{\n    if (entity == nullptr)\n    {\nthrow new ArgumentNullException(\"entity\");\n    }\nreturn this->isDisposing || entity->isDisposing;\n}\n\nvoid Entity::RefreshRegistrationsAfterParentChange()\n{\n    if (!this->get_IsHierarchyEnabled() || Core::get_Instance() == nullptr || Core::get_Instance()->get_ObjectManager() == nullptr)\n    {\nreturn;    }\nthis->RefreshRegistrationsAfterParentChangeRecursive(this);\n}\n\n",
                "GameCube generated Entity.cpp should expose the disposal-aware detach suppression helper.");
        }

        if (normalizedContents.Contains("if (wasHierarchyEnabled && entity->get_IsHierarchyEnabled())", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "    if (wasHierarchyEnabled && entity->get_IsHierarchyEnabled())\n    {\nentity->RefreshRegistrationsAfterParentChange();\n    }\n",
                "    if (!this->ShouldSuppressRegistrationRefreshForDetachment(entity) && wasHierarchyEnabled && entity->get_IsHierarchyEnabled())\n    {\nentity->RefreshRegistrationsAfterParentChange();\n    }\n",
                "GameCube generated Entity.cpp should suppress registration refresh while entities are being disposed.");
        }

        if (!normalizedContents.Contains("if (this->isDisposing)", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "void Entity::Dispose()\n{\n    if (this->Children != nullptr)\n",
                "void Entity::Dispose()\n{\n    if (this->isDisposing)\n    {\nreturn;    }\nthis->isDisposing = true;\n    if (this->Children != nullptr)\n",
                "GameCube generated Entity.cpp should guard disposal re-entry before tearing down children.");
        }

        return normalizedContents;
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
