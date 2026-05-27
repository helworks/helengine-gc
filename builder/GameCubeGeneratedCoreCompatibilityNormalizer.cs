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
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RenderManager3D.hpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "RenderManager3D.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "FPSComponent.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "Entity.hpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "Entity.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "PlatformMenuSceneResolver.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "DemoDiscReturnToMenuRuntimeComponent.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "TextureAssetColorFormat.hpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "EditorAssetBinarySerializer.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "FontAssetBinarySerializer.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "FontAsset.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "PointerInteractableHitResolver.cpp");
        NormalizeGeneratedCoreFile(generatedCoreRootPath, "StandardPlatformInputConfiguration.cpp");
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
        } else if (string.Equals(relativePath, "PlatformMenuSceneResolver.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizePlatformMenuSceneResolverSource(normalizedContents);
        } else if (string.Equals(relativePath, "DemoDiscReturnToMenuRuntimeComponent.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeDemoDiscReturnToMenuRuntimeComponentSource(normalizedContents);
        } else if (string.Equals(relativePath, "TextureAssetColorFormat.hpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeTextureAssetColorFormatHeaderSource(normalizedContents);
        } else if (string.Equals(relativePath, "EditorAssetBinarySerializer.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeTextureColorFormatReaderSource(normalizedContents);
        } else if (string.Equals(relativePath, "FontAssetBinarySerializer.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeFontAssetBinarySerializerSource(normalizedContents);
        } else if (string.Equals(relativePath, "FontAsset.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeFontAssetSource(normalizedContents);
        } else if (string.Equals(relativePath, "PointerInteractableHitResolver.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizePointerInteractableHitResolverSource(normalizedContents);
        } else if (string.Equals(relativePath, "StandardPlatformInputConfiguration.cpp", StringComparison.OrdinalIgnoreCase)) {
            return NormalizeStandardPlatformInputConfigurationSource(normalizedContents);
        }

        return normalizedContents;
    }

    /// <summary>
    /// Rewrites generated font disposal so GameCube runtime logs each owned pointer before deletion.
    /// </summary>
    /// <param name="contents">Generated font asset source.</param>
    /// <returns>Normalized generated font asset source.</returns>
    static string NormalizeFontAssetSource(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include <ogc/system.h>", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "#include \"FontAsset.hpp\"\n",
                "#include \"FontAsset.hpp\"\n#include <ogc/system.h>\n",
                "GameCube generated FontAsset.cpp should include SYS_Report support for font disposal diagnostics.");
        }

        if (!normalizedContents.Contains("[GC] FontAsset dispose begin", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "const bool sourceTexturePaletteColorsUsesSharedEmptyArray = (sourceTexturePaletteColors == Array<uint8_t>::Empty());\n",
                "const bool sourceTexturePaletteColorsUsesSharedEmptyArray = (sourceTexturePaletteColors == Array<uint8_t>::Empty());\nSYS_Report(\"[GC] FontAsset dispose begin font=%p characters=%p fontInfo=%p sourceTextureAsset=%p sourceColors=%p sourcePalette=%p texture=%p atlasPath=%s\\n\", this, characters, fontInfo, sourceTextureAsset, sourceTextureColors, sourceTexturePaletteColors, this->get_Texture(), this->get_CookedAtlasTextureRelativePath().c_str());\n",
                "GameCube generated FontAsset.cpp should log font disposal begin state.");
        }

        if (!normalizedContents.Contains("[GC] FontAsset delete characters", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "delete characters;\ndelete fontInfo;\n",
                "SYS_Report(\"[GC] FontAsset delete characters=%p\\n\", characters);\ndelete characters;\nSYS_Report(\"[GC] FontAsset delete fontInfo=%p\\n\", fontInfo);\ndelete fontInfo;\n",
                "GameCube generated FontAsset.cpp should log font character and font-info deletes.");
        }

        if (!normalizedContents.Contains("[GC] FontAsset delete source colors", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "    if (!sourceTextureColorsUsesSharedEmptyArray)\n    {\ndelete sourceTextureColors;\n    }\n    if (!sourceTexturePaletteColorsUsesSharedEmptyArray)\n    {\ndelete sourceTexturePaletteColors;\n    }\ndelete sourceTextureAsset;\n",
                "    if (!sourceTextureColorsUsesSharedEmptyArray)\n    {\nSYS_Report(\"[GC] FontAsset delete source colors=%p\\n\", sourceTextureColors);\ndelete sourceTextureColors;\n    }\n    if (!sourceTexturePaletteColorsUsesSharedEmptyArray)\n    {\nSYS_Report(\"[GC] FontAsset delete source palette=%p\\n\", sourceTexturePaletteColors);\ndelete sourceTexturePaletteColors;\n    }\nSYS_Report(\"[GC] FontAsset delete source texture asset=%p\\n\", sourceTextureAsset);\ndelete sourceTextureAsset;\n",
                "GameCube generated FontAsset.cpp should log font source texture deletes.");
        }

        return normalizedContents;
    }

    /// <summary>
    /// Rewrites one generated null-conditional parent access that the current C++ generator still emits into the pointer interactable hit resolver.
    /// </summary>
    /// <param name="contents">Generated pointer interactable hit resolver source.</param>
    /// <returns>Normalized pointer interactable hit resolver source.</returns>
    static string NormalizePointerInteractableHitResolverSource(string contents) {
        return contents.Replace(
            "if (interactable?.Parent == nullptr || camera == nullptr)",
            "if (interactable == nullptr || interactable->get_Parent() == nullptr || camera == nullptr)",
            StringComparison.Ordinal);
    }

    /// <summary>
    /// Rewrites generated standard platform input configuration binding copies back to a native list so the GameCube generated core compiles against the current binding field type.
    /// </summary>
    /// <param name="contents">Generated standard platform input configuration source.</param>
    /// <returns>Normalized standard platform input configuration source.</returns>
    static string NormalizeStandardPlatformInputConfigurationSource(string contents) {
        return contents.Replace(
            "Array<::StandardPlatformActionBinding*> *copiedBindings = new Array<StandardPlatformActionBinding*>(bindings->get_Count());\nfor (int32_t index = 0; index < bindings->get_Count(); index++) {\n::StandardPlatformActionBinding *binding = (*bindings)[index];\n(*copiedBindings)[index] = (binding != nullptr ? binding : throw new InvalidOperationException(\"Standard platform action bindings cannot contain null entries.\"));\n}\nthis->Bindings = copiedBindings;",
            "List<::StandardPlatformActionBinding*> *copiedBindings = new List<StandardPlatformActionBinding*>(bindings->get_Count());\nfor (int32_t index = 0; index < bindings->get_Count(); index++) {\n::StandardPlatformActionBinding *binding = (*bindings)[index];\ncopiedBindings->Add(binding != nullptr ? binding : throw new InvalidOperationException(\"Standard platform action bindings cannot contain null entries.\"));\n}\nthis->Bindings = copiedBindings;",
            StringComparison.Ordinal);
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
        if (UsesPointerBasedCookedPlatformMaterialContract(contents) || UsesPathBasedCookedPlatformMaterialContract(contents)) {
            return NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(contents);
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
            return NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(normalizedContents);
        }

        if (!normalizedContents.Contains("#include \"PlatformMaterialAsset.hpp\"", StringComparison.Ordinal)) {
            normalizedContents = normalizedContents.Replace(
                "#include \"MaterialAsset.hpp\"\n",
                "#include \"MaterialAsset.hpp\"\n#include \"PlatformMaterialAsset.hpp\"\n",
                StringComparison.Ordinal);
        }

        const string cookedMaterialBlockWithoutTextureBinding = "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nreturn runtimeMaterial;}";
        if (normalizedContents.Contains(cookedMaterialBlockWithoutTextureBinding, StringComparison.Ordinal)) {
            return NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(normalizedContents.Replace(
                cookedMaterialBlockWithoutTextureBinding,
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
                StringComparison.Ordinal));
        }

        string modernRawMaterialBlock = "::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nthis->ApplyMaterialDiffuseTexture(runtimeMaterial, materialAsset, fullPath);\nreturn runtimeMaterial;}";
        if (normalizedContents.Contains(modernRawMaterialBlock, StringComparison.Ordinal)) {
            return NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(normalizedContents.Replace(
                modernRawMaterialBlock,
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);\nif (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {\n::TextureAsset *textureAsset = this->AssetContentManager->Load<TextureAsset*>(materialAsset->TextureRelativePath, RuntimeContentProcessorIds::TextureAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(textureAsset);\nif (textureAsset->Colors != nullptr && textureAsset->Colors != Array<uint8_t>::Empty()) {\ndelete textureAsset->Colors;\ntextureAsset->Colors = Array<uint8_t>::Empty();\n}\nif (textureAsset->PaletteColors != nullptr && textureAsset->PaletteColors != Array<uint8_t>::Empty()) {\ndelete textureAsset->PaletteColors;\ntextureAsset->PaletteColors = Array<uint8_t>::Empty();\n}\ndelete textureAsset;\nthis->TrackOwnedTexture(runtimeTexture);\nruntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);\n}\nreturn runtimeMaterial;}",
                StringComparison.Ordinal));
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
            if (UsesCookedPlatformMaterialContract(fallbackNormalizedContents)) {
                return NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(fallbackNormalizedContents);
            }

            throw new InvalidOperationException(
                "GameCube generated runtime scene asset resolver should resolve materials through the cooked platform material contract. "
                + $"PointerBased={UsesPointerBasedCookedPlatformMaterialContract(fallbackNormalizedContents)}; "
                + $"PathBased={UsesPathBasedCookedPlatformMaterialContract(fallbackNormalizedContents)}.");
        }

        return NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(fallbackNormalizedContents);
    }

    /// <summary>
    /// Rewrites generated runtime scene asset resolver paths so GameCube builds log runtime asset resolution, cache hits, and owned-asset tracking summaries.
    /// </summary>
    /// <param name="contents">Generated runtime scene resolver source.</param>
    /// <returns>Generated runtime scene resolver source with GameCube resolver diagnostics injected.</returns>
    static string NormalizeRuntimeSceneAssetReferenceResolverLoggingSource(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include <ogc/system.h>", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "#include \"RuntimeSceneAssetReferenceResolver.hpp\"\n",
                "#include \"RuntimeSceneAssetReferenceResolver.hpp\"\n#include <ogc/system.h>\n",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should include SYS_Report support for runtime asset diagnostics.");
        }

        if (!normalizedContents.Contains("[GC] Resolver begin owned asset tracking", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->ActiveOwnedMaterials = new List<::RuntimeMaterial*>();\n}",
                "this->ActiveOwnedMaterials = new List<::RuntimeMaterial*>();\nSYS_Report(\"[GC] Resolver begin owned asset tracking\\n\");\n}",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log owned-asset tracking start.");
        }

        if (!normalizedContents.Contains("[GC] Resolver complete owned asset tracking", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "delete resolvedFontsByPath;\nreturn new ::RuntimeSceneOwnedAssetSet(ownedTextures, ownedFonts, ownedModels, ownedMaterials);}",
                "delete resolvedFontsByPath;\nSYS_Report(\"[GC] Resolver complete owned asset tracking textures=%u fonts=%u models=%u materials=%u\\n\", static_cast<unsigned>(ownedTextures != nullptr ? ownedTextures->get_Count() : 0), static_cast<unsigned>(ownedFonts != nullptr ? ownedFonts->get_Count() : 0), static_cast<unsigned>(ownedModels != nullptr ? ownedModels->get_Count() : 0), static_cast<unsigned>(ownedMaterials != nullptr ? ownedMaterials->get_Count() : 0));\nreturn new ::RuntimeSceneOwnedAssetSet(ownedTextures, ownedFonts, ownedModels, ownedMaterials);}",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log owned-asset tracking completion.");
        }

        if (!normalizedContents.Contains("[GC] Resolver font cache-hit path=%s ptr=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "return cachedFontAsset;    }",
                "SYS_Report(\"[GC] Resolver font cache-hit path=%s ptr=%p\\n\", fullPath.c_str(), cachedFontAsset);\nreturn cachedFontAsset;    }",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log runtime font cache hits.");
        }

        if (!normalizedContents.Contains("[GC] Resolver font load path=%s ptr=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedFont(fontAsset);\nreturn fontAsset;}",
                "this->TrackOwnedFont(fontAsset);\nSYS_Report(\"[GC] Resolver font load path=%s ptr=%p\\n\", fullPath.c_str(), fontAsset);\nreturn fontAsset;}",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log runtime font loads.");
        }

        if (!normalizedContents.Contains("[GC] Resolver font atlas attach begin path=%s font=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "const std::string atlasFullPath = this->ResolvePackagedContentPath(fontAsset->get_CookedAtlasTextureRelativePath());\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromCooked(atlasFullPath);\nfontAsset->AttachCookedRuntimeTexture(runtimeTexture);",
                "const std::string atlasFullPath = this->ResolvePackagedContentPath(fontAsset->get_CookedAtlasTextureRelativePath());\nSYS_Report(\"[GC] Resolver font atlas attach begin path=%s font=%p\\n\", atlasFullPath.c_str(), fontAsset);\n::RuntimeTexture *runtimeTexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromCooked(atlasFullPath);\nfontAsset->AttachCookedRuntimeTexture(runtimeTexture);\nSYS_Report(\"[GC] Resolver font atlas attach complete path=%s font=%p texture=%p\\n\", atlasFullPath.c_str(), fontAsset, runtimeTexture);",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log external cooked font atlas attachment.");
        }

        if (!normalizedContents.Contains("[GC] Resolver material generated cache-hit key=%s ptr=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedMaterial(generatedRuntimeMaterial);\nreturn generatedRuntimeMaterial;    }",
                "this->TrackOwnedMaterial(generatedRuntimeMaterial);\nSYS_Report(\"[GC] Resolver material generated cache-hit key=%s ptr=%p\\n\", generatedAssetKey.c_str(), generatedRuntimeMaterial);\nreturn generatedRuntimeMaterial;    }",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log generated material cache hits.");
        }

        if (!normalizedContents.Contains("[GC] Resolver material load path=%s ptr=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedMaterial(generatedCookedRuntimeMaterial);\nreturn generatedCookedRuntimeMaterial;    }",
                "this->TrackOwnedMaterial(generatedCookedRuntimeMaterial);\nSYS_Report(\"[GC] Resolver material load path=%s ptr=%p\\n\", generatedFullPath.c_str(), generatedCookedRuntimeMaterial);\nreturn generatedCookedRuntimeMaterial;    }",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log generated material loads.");
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedMaterial(runtimeMaterial);\nreturn runtimeMaterial;}",
                "this->TrackOwnedMaterial(runtimeMaterial);\nSYS_Report(\"[GC] Resolver material load path=%s ptr=%p\\n\", fullPath.c_str(), runtimeMaterial);\nreturn runtimeMaterial;}",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log file-backed material loads.");
        }

        if (!normalizedContents.Contains("[GC] Resolver model generated cache-hit key=%s ptr=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedModel(generatedRuntimeModel);\nreturn generatedRuntimeModel;    }",
                "this->TrackOwnedModel(generatedRuntimeModel);\nSYS_Report(\"[GC] Resolver model generated cache-hit key=%s ptr=%p\\n\", generatedAssetKey.c_str(), generatedRuntimeModel);\nreturn generatedRuntimeModel;    }",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log generated model cache hits.");
        }

        if (!normalizedContents.Contains("[GC] Resolver model load path=%s ptr=%p", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedModel(generatedModel);\nreturn generatedModel;    }",
                "this->TrackOwnedModel(generatedModel);\nSYS_Report(\"[GC] Resolver model load path=%s ptr=%p\\n\", generatedFullPath.c_str(), generatedModel);\nreturn generatedModel;    }",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log generated model loads.");
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "this->TrackOwnedModel(runtimeModel);\nreturn runtimeModel;}",
                "this->TrackOwnedModel(runtimeModel);\nSYS_Report(\"[GC] Resolver model load path=%s ptr=%p\\n\", fullPath.c_str(), runtimeModel);\nreturn runtimeModel;}",
                "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log file-backed model loads.");
        }

        if (!normalizedContents.Contains("[GC] Resolver texture build path=%s ptr=%p", StringComparison.Ordinal)) {
            const string trackedTextureBlockWithStage = "this->TrackOwnedTexture(runtimeTexture);\nthis->set_LastTextureLoadStage(\"ResolveTextureTracked\");\nreturn runtimeTexture;}";
            const string trackedTextureBlockWithoutStage = "this->TrackOwnedTexture(runtimeTexture);\nreturn runtimeTexture;}";
            if (normalizedContents.Contains(trackedTextureBlockWithStage, StringComparison.Ordinal)) {
                normalizedContents = ReplaceRequired(
                    normalizedContents,
                    trackedTextureBlockWithStage,
                    "this->TrackOwnedTexture(runtimeTexture);\nSYS_Report(\"[GC] Resolver texture build path=%s ptr=%p\\n\", fullPath.c_str(), runtimeTexture);\nthis->set_LastTextureLoadStage(\"ResolveTextureTracked\");\nreturn runtimeTexture;}",
                    "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log file-backed texture builds.");
            } else {
                normalizedContents = ReplaceRequired(
                    normalizedContents,
                    trackedTextureBlockWithoutStage,
                    "this->TrackOwnedTexture(runtimeTexture);\nSYS_Report(\"[GC] Resolver texture build path=%s ptr=%p\\n\", fullPath.c_str(), runtimeTexture);\nreturn runtimeTexture;}",
                    "GameCube generated RuntimeSceneAssetReferenceResolver.cpp should log file-backed texture builds.");
            }
        }

        return normalizedContents;
    }

    /// <summary>
    /// Determines whether the generated runtime scene resolver already uses the cooked platform material contract expected by the GameCube packaged runtime.
    /// </summary>
    /// <param name="contents">Generated runtime scene resolver source.</param>
    /// <returns>True when the resolver already loads cooked platform materials and binds packaged textures directly.</returns>
    static bool UsesCookedPlatformMaterialContract(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        return UsesPointerBasedCookedPlatformMaterialContract(contents) || UsesPathBasedCookedPlatformMaterialContract(contents);
    }

    /// <summary>
    /// Determines whether the generated runtime scene resolver already uses the legacy pointer-based cooked platform material contract.
    /// </summary>
    /// <param name="contents">Generated runtime scene resolver source.</param>
    /// <returns>True when the resolver loads <c>PlatformMaterialAsset</c> instances directly and binds textures in the resolver.</returns>
    static bool UsesPointerBasedCookedPlatformMaterialContract(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        return contents.Contains("::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);", StringComparison.Ordinal)
            && contents.Contains("::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);", StringComparison.Ordinal)
            && contents.Contains("if (!String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath)) {", StringComparison.Ordinal)
            && contents.Contains("runtimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);", StringComparison.Ordinal);
    }

    /// <summary>
    /// Determines whether the generated runtime scene resolver already uses the current path-based cooked platform material contract.
    /// </summary>
    /// <param name="contents">Generated runtime scene resolver source.</param>
    /// <returns>True when the resolver delegates cooked material creation directly to the renderer using packaged material paths.</returns>
    static bool UsesPathBasedCookedPlatformMaterialContract(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        return contents.Contains("::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedFullPath);", StringComparison.Ordinal)
            && contents.Contains("::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(fullPath);", StringComparison.Ordinal);
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
    /// Rewrites the generated packaged font reader so GameCube accepts the current shared font payload version and layout.
    /// </summary>
    /// <param name="contents">Generated packaged font reader source.</param>
    /// <returns>Normalized packaged font reader source.</returns>
    static string NormalizeFontAssetBinarySerializerSource(string contents) {
        string normalizedContents = NormalizeTextureColorFormatReaderSource(contents);
        normalizedContents = global::System.Text.RegularExpressions.Regex.Replace(
            normalizedContents,
            """uint8_t FontAssetBinarySerializer::CurrentVersion = \d+;""",
            "uint8_t FontAssetBinarySerializer::CurrentVersion = 5;",
            global::System.Text.RegularExpressions.RegexOptions.Multiline);
        normalizedContents = global::System.Text.RegularExpressions.Regex.Replace(
            normalizedContents,
            """uint8_t FontAssetBinarySerializer::RuntimeTextureIdVersion = \d+;""",
            "uint8_t FontAssetBinarySerializer::RuntimeTextureIdVersion = 2;",
            global::System.Text.RegularExpressions.RegexOptions.Multiline);
        normalizedContents = global::System.Text.RegularExpressions.Regex.Replace(
            normalizedContents,
            """uint8_t FontAssetBinarySerializer::TextureColorFormatVersion = \d+;""",
            "uint8_t FontAssetBinarySerializer::TextureColorFormatVersion = 3;",
            global::System.Text.RegularExpressions.RegexOptions.Multiline);
        normalizedContents = global::System.Text.RegularExpressions.Regex.Replace(
            normalizedContents,
            """uint8_t FontAssetBinarySerializer::PaletteTextureMetadataVersion = \d+;""",
            "uint8_t FontAssetBinarySerializer::PaletteTextureMetadataVersion = 4;",
            global::System.Text.RegularExpressions.RegexOptions.Multiline);

        if (!normalizedContents.Contains("uint8_t FontAssetBinarySerializer::ExternalCookedAtlasPathVersion = 5;", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "uint8_t FontAssetBinarySerializer::TextureColorFormatVersion = 3;\n\n::TextureAssetAlphaPrecision FontAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(::TextureAssetColorFormat colorFormat)\n",
                "uint8_t FontAssetBinarySerializer::TextureColorFormatVersion = 3;\n\nuint8_t FontAssetBinarySerializer::ExternalCookedAtlasPathVersion = 5;\n\n::TextureAssetAlphaPrecision FontAssetBinarySerializer::GetDefaultTextureAssetAlphaPrecision(::TextureAssetColorFormat colorFormat)\n",
                "GameCube generated FontAssetBinarySerializer.cpp should expose the external cooked atlas font version constant.");
        }

        if (UsesCurrentCookedAtlasFontDeserializer(normalizedContents)) {
            return NormalizeCurrentCookedAtlasFontDeserializer(normalizedContents);
        }

        if (normalizedContents.Contains("uint8_t FontAssetBinarySerializer::CurrentVersion = 5;", StringComparison.Ordinal)
            && normalizedContents.Contains("uint8_t FontAssetBinarySerializer::ExternalCookedAtlasPathVersion = 5;", StringComparison.Ordinal)
            && normalizedContents.Contains("if (header->get_Version() >= ExternalCookedAtlasPathVersion)", StringComparison.Ordinal)
            && normalizedContents.Contains("cookedAtlasTextureRelativePath = reader->ReadString();", StringComparison.Ordinal)) {
            return normalizedContents;
        }

        const string fontDeserializerStartMarker = "FontAssetBinarySerializer::set_LastDeserializeStage(\"ReadFontInfo\");";
        const string fontDeserializerEndMarker = "FontAssetBinarySerializer::set_LastDeserializeStage(\"ReadCharacterCount\");";
        int fontDeserializerStartIndex = normalizedContents.IndexOf(fontDeserializerStartMarker, StringComparison.Ordinal);
        int fontDeserializerEndIndex = normalizedContents.IndexOf(fontDeserializerEndMarker, StringComparison.Ordinal);
        if (fontDeserializerStartIndex < 0 || fontDeserializerEndIndex <= fontDeserializerStartIndex) {
            throw new InvalidOperationException("GameCube generated FontAssetBinarySerializer.cpp should deserialize version-5 font payloads using the shared cooked-atlas layout.");
        }
        normalizedContents =
            normalizedContents[..fontDeserializerStartIndex]
            + "::TextureAsset *sourceTexture = new ::TextureAsset();\n::FontInfo *fontInfo = nullptr;\nfloat lineHeight = 0.0f;\nint32_t atlasWidth = 0;\nint32_t atlasHeight = 0;\nif (header->get_Version() >= ExternalCookedAtlasPathVersion)\n{\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadCookedAtlasTexturePath\");\nreader->ReadString();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadSourceTextureHeader\");\nsourceTexture->set_RuntimeAssetId(header->get_Version() >= RuntimeTextureIdVersion ? static_cast<uint64_t>(reader->ReadInt64()) : 0ul);\nsourceTexture->Width = reader->ReadUInt16();\nsourceTexture->Height = reader->ReadUInt16();\nsourceTexture->ColorFormat = header->get_Version() >= TextureColorFormatVersion ? ReadTextureAssetColorFormat(reader) : TextureAssetColorFormat::Rgba32;\nsourceTexture->AlphaPrecision = header->get_Version() >= PaletteTextureMetadataVersion ? ReadTextureAssetAlphaPrecision(reader) : GetDefaultTextureAssetAlphaPrecision(sourceTexture->ColorFormat);\nsourceTexture->PaletteColors = header->get_Version() >= PaletteTextureMetadataVersion ? reader->ReadByteArray() : Array<uint8_t>::Empty();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadSourceTextureColors\");\nsourceTexture->Colors = reader->ReadByteArray();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadFontInfo\");\nfontInfo = ([&]() {\nauto __ctor_arg_000000BE = reader->ReadString();\nauto __ctor_arg_000000BF = reader->ReadInt32();\nauto __ctor_arg_000000C0 = reader->ReadSingle();\nreturn new ::FontInfo(__ctor_arg_000000BE, __ctor_arg_000000BF, __ctor_arg_000000C0);\n})();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadAtlasMetrics\");\nlineHeight = reader->ReadSingle();\natlasWidth = reader->ReadInt32();\natlasHeight = reader->ReadInt32();\n}\nelse {\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadFontInfo\");\nfontInfo = ([&]() {\nauto __ctor_arg_000000BE = reader->ReadString();\nauto __ctor_arg_000000BF = reader->ReadInt32();\nauto __ctor_arg_000000C0 = reader->ReadSingle();\nreturn new ::FontInfo(__ctor_arg_000000BE, __ctor_arg_000000BF, __ctor_arg_000000C0);\n})();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadAtlasMetrics\");\nlineHeight = reader->ReadSingle();\natlasWidth = reader->ReadInt32();\natlasHeight = reader->ReadInt32();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadSourceTextureHeader\");\nsourceTexture->set_RuntimeAssetId(header->get_Version() >= RuntimeTextureIdVersion ? static_cast<uint64_t>(reader->ReadInt64()) : 0ul);\nsourceTexture->Width = reader->ReadUInt16();\nsourceTexture->Height = reader->ReadUInt16();\nsourceTexture->ColorFormat = header->get_Version() >= TextureColorFormatVersion ? ReadTextureAssetColorFormat(reader) : TextureAssetColorFormat::Rgba32;\nsourceTexture->AlphaPrecision = header->get_Version() >= PaletteTextureMetadataVersion ? ReadTextureAssetAlphaPrecision(reader) : GetDefaultTextureAssetAlphaPrecision(sourceTexture->ColorFormat);\nsourceTexture->PaletteColors = header->get_Version() >= PaletteTextureMetadataVersion ? reader->ReadByteArray() : Array<uint8_t>::Empty();\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ReadSourceTextureColors\");\nsourceTexture->Colors = reader->ReadByteArray();\n}\n"
            + normalizedContents[fontDeserializerEndIndex..];

        const string fontTextureBuildStartMarker = "FontAssetBinarySerializer::set_LastDeserializeStage(\"BuildRuntimeTexture\");";
        const string fontTextureBuildEndMarker = "FontAssetBinarySerializer::set_LastDeserializeStage(\"Complete\");";
        int fontTextureBuildStartIndex = normalizedContents.IndexOf(fontTextureBuildStartMarker, StringComparison.Ordinal);
        int fontTextureBuildEndIndex = normalizedContents.IndexOf(fontTextureBuildEndMarker, StringComparison.Ordinal);
        if (fontTextureBuildStartIndex < 0 || fontTextureBuildEndIndex <= fontTextureBuildStartIndex) {
            throw new InvalidOperationException("GameCube generated FontAssetBinarySerializer.cpp should only build runtime font textures when embedded atlas bytes are present.");
        }
        normalizedContents =
            normalizedContents[..fontTextureBuildStartIndex]
            + "FontAssetBinarySerializer::set_LastDeserializeStage(\"BuildRuntimeTexture\");\n::RuntimeTexture *texture = nullptr;\n::TextureAsset *storedSourceTextureAsset = nullptr;\nif (sourceTexture->Width > 0 && sourceTexture->Height > 0 && sourceTexture->Colors != nullptr && sourceTexture->Colors->get_Length() > 0)\n{\ntexture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(sourceTexture);\nstoredSourceTextureAsset = sourceTexture;\n}\nFontAssetBinarySerializer::set_LastDeserializeStage(\"ConstructFontAsset\");\n::FontAsset *asset = ([&]() {\nauto __object_000000C6 = new ::FontAsset(fontInfo, texture, characters, lineHeight, atlasWidth, atlasHeight);\n__object_000000C6->set_SourceTextureAsset(storedSourceTextureAsset);\nreturn __object_000000C6;\n})();\n"
            + normalizedContents[fontTextureBuildEndIndex..];

        return normalizedContents;
    }

    /// <summary>
    /// Returns true when the shared generated font reader already uses the current cooked-atlas payload layout.
    /// </summary>
    /// <param name="contents">Generated packaged font reader source.</param>
    /// <returns>True when the source already matches the current shared cooked-atlas font layout.</returns>
    static bool UsesCurrentCookedAtlasFontDeserializer(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        return contents.Contains("std::string cookedAtlasTextureRelativePath = String::Empty;", StringComparison.Ordinal)
            && contents.Contains("if (header->get_Version() >= ExternalCookedAtlasPathVersion)", StringComparison.Ordinal)
            && contents.Contains("cookedAtlasTextureRelativePath = reader->ReadString();", StringComparison.Ordinal)
            && contents.Contains("set_CookedAtlasTextureRelativePath(cookedAtlasTextureRelativePath);", StringComparison.Ordinal);
    }

    /// <summary>
    /// Normalizes the current shared cooked-atlas font reader shape for GameCube by avoiding runtime texture creation when the cooked payload omits embedded atlas bytes.
    /// </summary>
    /// <param name="contents">Generated packaged font reader source that already matches the current cooked-atlas layout.</param>
    /// <returns>Normalized packaged font reader source.</returns>
    static string NormalizeCurrentCookedAtlasFontDeserializer(string contents) {
        if (contents == null) {
            throw new ArgumentNullException(nameof(contents));
        }

        if (contents.Contains("::TextureAsset *storedSourceTextureAsset = nullptr;", StringComparison.Ordinal)
            && contents.Contains("if (sourceTexture->Width > 0 && sourceTexture->Height > 0 && sourceTexture->Colors != nullptr && sourceTexture->Colors->get_Length() > 0)", StringComparison.Ordinal)) {
            return contents;
        }

        global::System.Text.RegularExpressions.Match textureBuildMatch = global::System.Text.RegularExpressions.Regex.Match(
            contents,
            """::RuntimeTexture \*texture = Core::get_Instance\(\)->get_RenderManager2D\(\)->BuildTextureFromRaw\(sourceTexture\);\n::FontAsset \*asset = \(\[&\]\(\) \{\nauto (?<assetVariable>__object_[A-F0-9]+) = new ::FontAsset\(fontInfo, texture, characters, lineHeight, atlasWidth, atlasHeight\);\n\k<assetVariable>->set_SourceTextureAsset\(sourceTexture\);\n\k<assetVariable>->set_CookedAtlasTextureRelativePath\(cookedAtlasTextureRelativePath\);\nreturn \k<assetVariable>;\n\}\)\(\);""",
            global::System.Text.RegularExpressions.RegexOptions.Multiline);
        if (!textureBuildMatch.Success) {
            throw new InvalidOperationException("GameCube generated FontAssetBinarySerializer.cpp should build cooked-atlas font assets through the current shared texture-construction block.");
        }

        string assetVariableName = textureBuildMatch.Groups["assetVariable"].Value;
        string replacementBlock =
            "::RuntimeTexture *texture = nullptr;\n"
            + "::TextureAsset *storedSourceTextureAsset = nullptr;\n"
            + "if (sourceTexture->Width > 0 && sourceTexture->Height > 0 && sourceTexture->Colors != nullptr && sourceTexture->Colors->get_Length() > 0)\n"
            + "{\n"
            + "texture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(sourceTexture);\n"
            + "storedSourceTextureAsset = sourceTexture;\n"
            + "}\n"
            + "::FontAsset *asset = ([&]() {\n"
            + $"auto {assetVariableName} = new ::FontAsset(fontInfo, texture, characters, lineHeight, atlasWidth, atlasHeight);\n"
            + $"{assetVariableName}->set_SourceTextureAsset(storedSourceTextureAsset);\n"
            + $"{assetVariableName}->set_CookedAtlasTextureRelativePath(cookedAtlasTextureRelativePath);\n"
            + $"return {assetVariableName};\n"
            + "})();";
        return contents[..textureBuildMatch.Index]
            + replacementBlock
            + contents[(textureBuildMatch.Index + textureBuildMatch.Length)..];
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
        if (contents.Contains("this->OverlayHost->Dispose();", StringComparison.Ordinal)
            || contents.Contains("overlayHost->Dispose();", StringComparison.Ordinal)) {
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
    /// Rewrites the generated menu-scene resolver so packaged GameCube builds return to the authored packaged main-menu scene id.
    /// </summary>
    /// <param name="contents">Generated menu-scene resolver source.</param>
    /// <returns>Normalized generated menu-scene resolver source.</returns>
    static string NormalizePlatformMenuSceneResolverSource(string contents) {
        if (contents.Contains("std::string PlatformMenuSceneResolver::DesktopMainMenuSceneId = \"Scenes/DemoDiscMainMenu.helen\";", StringComparison.Ordinal)) {
            return contents;
        }

        return ReplaceRequired(
            contents,
            "std::string PlatformMenuSceneResolver::DesktopMainMenuSceneId = \"DemoDiscMainMenu\";\n",
            "std::string PlatformMenuSceneResolver::DesktopMainMenuSceneId = \"Scenes/DemoDiscMainMenu.helen\";\n",
            "GameCube generated PlatformMenuSceneResolver.cpp should route return-to-menu requests to the packaged main-menu scene id.");
    }

    /// <summary>
    /// Rewrites the generated return-to-menu runtime component so GameCube builds can log whether the component updates and sees the B-button edge.
    /// </summary>
    /// <param name="contents">Generated return-to-menu runtime component source.</param>
    /// <returns>Normalized generated return-to-menu runtime component source.</returns>
    static string NormalizeDemoDiscReturnToMenuRuntimeComponentSource(string contents) {
        string normalizedContents = contents;
        if (!normalizedContents.Contains("#include <ogc/system.h>", StringComparison.Ordinal)) {
            normalizedContents = ReplaceRequired(
                normalizedContents,
                "#include \"system/diagnostics/stopwatch.hpp\"\n",
                "#include \"system/diagnostics/stopwatch.hpp\"\n#include <ogc/system.h>\n",
                "GameCube generated DemoDiscReturnToMenuRuntimeComponent.cpp should include SYS_Report support for input diagnostics.");
        }

        if (normalizedContents.Contains("[GC] ReturnToMenu update alive", StringComparison.Ordinal)) {
            return normalizedContents;
        }

        return ReplaceRequired(
            normalizedContents,
            "void DemoDiscReturnToMenuRuntimeComponent::Update()\n{\n::InputSystem *inputSystem = Core::get_Instance()->get_Input();\nconst bool wasReturnPressed = this->WasKeyboardReturnPressed(inputSystem) || this->WasGamepadReturnPressed(inputSystem);\n    if (wasReturnPressed)\n    {\nCore::get_Instance()->get_SceneManager()->LoadScene(PlatformMenuSceneResolver::ResolveMainMenuSceneId(), SceneLoadMode::Single);\n    }\n}\n",
            "void DemoDiscReturnToMenuRuntimeComponent::Update()\n{\nstatic bool hasLoggedAlive = false;\n::InputSystem *inputSystem = Core::get_Instance()->get_Input();\n::InputGamepadState currentState = inputSystem->GetGamepadState(0);\n::InputGamepadState previousState = inputSystem->GetPreviousGamepadState(0);\nconst bool eastDown = currentState.IsButtonDown(InputGamepadButton::East);\nconst bool eastWasDown = previousState.IsButtonDown(InputGamepadButton::East);\nconst bool northDown = currentState.IsButtonDown(InputGamepadButton::North);\nconst bool northWasDown = previousState.IsButtonDown(InputGamepadButton::North);\nconst bool selectDown = currentState.IsButtonDown(InputGamepadButton::Select);\nconst bool selectWasDown = previousState.IsButtonDown(InputGamepadButton::Select);\nif (!hasLoggedAlive) {\nSYS_Report(\"[GC] ReturnToMenu update alive connected=%d eastDown=%d eastWasDown=%d\\n\", currentState.get_Connected() ? 1 : 0, eastDown ? 1 : 0, eastWasDown ? 1 : 0);\nhasLoggedAlive = true;\n}\nconst bool wasReturnPressed = this->WasKeyboardReturnPressed(inputSystem) || this->WasGamepadReturnPressed(inputSystem);\n    if (wasReturnPressed)\n    {\nconst std::string targetSceneId = PlatformMenuSceneResolver::ResolveMainMenuSceneId();\nSYS_Report(\"[GC] ReturnToMenu pressed east=%d/%d north=%d/%d select=%d/%d target=%s\\n\", eastDown ? 1 : 0, eastWasDown ? 1 : 0, northDown ? 1 : 0, northWasDown ? 1 : 0, selectDown ? 1 : 0, selectWasDown ? 1 : 0, targetSceneId.c_str());\nCore::get_Instance()->get_SceneManager()->LoadScene(targetSceneId, SceneLoadMode::Single);\n    }\n}\n",
            "GameCube generated DemoDiscReturnToMenuRuntimeComponent.cpp should log whether the runtime component updates and sees the return-to-menu input edge.");
    }

    /// <summary>
    /// Rewrites the generated texture color-format enum so GameCube builds retain the GxRgb5A3 platform format value.
    /// </summary>
    /// <param name="contents">Generated texture color-format enum header source.</param>
    /// <returns>Normalized generated texture color-format enum header source.</returns>
    static string NormalizeTextureAssetColorFormatHeaderSource(string contents) {
        if (contents.Contains("GxRgb5A3 = 4", StringComparison.Ordinal)) {
            return contents;
        }

        return ReplaceRequired(
            contents,
            "    Indexed8 = 3\n};\n",
            "    Indexed8 = 3,\n    GxRgb5A3 = 4\n};\n",
            "GameCube generated TextureAssetColorFormat.hpp should include the GxRgb5A3 platform texture format.");
    }

    /// <summary>
    /// Rewrites generated texture color-format readers so GameCube builds can deserialize GxRgb5A3 texture payloads.
    /// </summary>
    /// <param name="contents">Generated texture color-format reader source.</param>
    /// <returns>Normalized generated texture color-format reader source.</returns>
    static string NormalizeTextureColorFormatReaderSource(string contents) {
        if (contents.Contains("TextureAssetColorFormat::GxRgb5A3", StringComparison.Ordinal)) {
            return contents;
        }

        string normalizedContents = contents
            .Replace(
                "else     if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Indexed8))\n    {\nreturn TextureAssetColorFormat::Indexed8;    }\nthrow new InvalidOperationException(std::string(\"Unsupported serialized texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                "else     if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Indexed8))\n    {\nreturn TextureAssetColorFormat::Indexed8;    }\nelse     if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::GxRgb5A3))\n    {\nreturn TextureAssetColorFormat::GxRgb5A3;    }\nthrow new InvalidOperationException(std::string(\"Unsupported serialized texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                StringComparison.Ordinal)
            .Replace(
                "else     if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Indexed8))\n    {\nreturn TextureAssetColorFormat::Indexed8;    }\nthrow new InvalidOperationException(std::string(\"Unsupported texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                "else     if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::Indexed8))\n    {\nreturn TextureAssetColorFormat::Indexed8;    }\nelse     if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::GxRgb5A3))\n    {\nreturn TextureAssetColorFormat::GxRgb5A3;    }\nthrow new InvalidOperationException(std::string(\"Unsupported texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                StringComparison.Ordinal)
            .Replace(
                "return TextureAssetColorFormat::Indexed8;    }\n}\n}\n}\nthrow new InvalidOperationException(std::string(\"Unsupported serialized texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                "return TextureAssetColorFormat::Indexed8;    }\nelse {\n    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::GxRgb5A3))\n    {\nreturn TextureAssetColorFormat::GxRgb5A3;    }\n}\n}\n}\n}\nthrow new InvalidOperationException(std::string(\"Unsupported serialized texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                StringComparison.Ordinal)
            .Replace(
                "return TextureAssetColorFormat::Indexed8;    }\n}\n}\n}\nthrow new InvalidOperationException(std::string(\"Unsupported texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                "return TextureAssetColorFormat::Indexed8;    }\nelse {\n    if (serializedValue == static_cast<uint8_t>(TextureAssetColorFormat::GxRgb5A3))\n    {\nreturn TextureAssetColorFormat::GxRgb5A3;    }\n}\n}\n}\n}\nthrow new InvalidOperationException(std::string(\"Unsupported texture color format '\") + std::to_string(serializedValue) + std::string(\"'.\"));\n}",
                StringComparison.Ordinal);
        if (normalizedContents.Contains("TextureAssetColorFormat::GxRgb5A3", StringComparison.Ordinal)) {
            return normalizedContents;
        }

        throw new InvalidOperationException("GameCube generated texture color-format readers should recognize the GxRgb5A3 serialized value.");
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
