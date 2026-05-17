using helengine.baseplatform.Definitions;
using helengine.baseplatform.Profiles;
using helengine.editor;
using System.Text.Json;

namespace helengine.gamecube.builder;

/// <summary>
/// Creates the typed GameCube builder metadata consumed by the editor.
/// </summary>
public static class GameCubePlatformDefinitionFactory {
    /// <summary>
    /// Creates the serialized default GameCube texture settings contract used when assets do not provide an explicit GameCube override.
    /// </summary>
    /// <returns>Serialized default GameCube texture settings.</returns>
    static string CreateDefaultSerializedTextureCookSettings() {
        return SerializeTextureCookSettings(new TextureAssetProcessorSettings {
            MaxResolution = 0,
            ColorFormat = TextureAssetColorFormat.GxRgb5A3,
            AlphaPrecision = TextureAssetAlphaPrecision.A8
        });
    }

    /// <summary>
    /// Creates the serialized default GameCube font-atlas texture settings contract used when fonts do not provide an explicit GameCube override.
    /// </summary>
    /// <returns>Serialized default GameCube font-atlas texture settings.</returns>
    static string CreateDefaultSerializedFontAtlasTextureCookSettings() {
        return SerializeTextureCookSettings(new TextureAssetProcessorSettings {
            MaxResolution = 0,
            ColorFormat = TextureAssetColorFormat.GxRgb5A3,
            AlphaPrecision = TextureAssetAlphaPrecision.A8
        });
    }

    /// <summary>
    /// Serializes the generic texture cook settings contract published to the editor build graph.
    /// </summary>
    /// <param name="processorSettings">Resolved texture processor settings to serialize.</param>
    /// <returns>Serialized generic texture cook settings payload.</returns>
    static string SerializeTextureCookSettings(TextureAssetProcessorSettings processorSettings) {
        if (processorSettings == null) {
            throw new ArgumentNullException(nameof(processorSettings));
        }

        return JsonSerializer.Serialize(new Dictionary<string, object> {
            ["maxResolution"] = processorSettings.MaxResolution,
            ["colorFormat"] = processorSettings.ColorFormat.ToString(),
            ["alphaPrecision"] = processorSettings.AlphaPrecision.ToString()
        });
    }

    /// <summary>
    /// Creates the generic texture format capability metadata supported by the GameCube texture cooker.
    /// </summary>
    /// <returns>Texture capability metadata for GameCube builder-owned texture cook contracts.</returns>
    static PlatformTextureFormatCapabilityDefinition CreateTextureFormatCapabilities() {
        return new PlatformTextureFormatCapabilityDefinition(
            [
                TextureAssetColorFormat.GxRgb5A3
            ],
            [
                TextureAssetAlphaPrecision.A8
            ],
            [
                new PlatformTextureFormatCombinationDefinition(TextureAssetColorFormat.GxRgb5A3, TextureAssetAlphaPrecision.A8)
            ]);
    }

    /// <summary>
    /// Creates the current GameCube platform definition.
    /// </summary>
    /// <returns>The GameCube platform definition.</returns>
    public static PlatformDefinition Create() {
        return new PlatformDefinition(
            "gamecube",
            "Nintendo GameCube",
            [
                new PlatformBuildProfileDefinition(
                    "gamecube-default",
                    "GameCube Default",
                    "Standard GameCube player build",
                    "gx",
                    "default",
                    [
                        new PlatformSettingDefinition(
                            "texture-scale-percent",
                            "Texture Scale Percent",
                            PlatformSettingKind.Text,
                            "100",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "shader-variant-pruning",
                            "Shader Variant Pruning",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            [])
                    ])
            ],
            [
                new PlatformGraphicsProfileDefinition(
                    "gx",
                    "GX",
                    "Current GameCube rendering backend",
                    [
                        new PlatformSettingDefinition(
                            "default-width",
                            "Default Width",
                            PlatformSettingKind.Text,
                            "640",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "default-height",
                            "Default Height",
                            PlatformSettingKind.Text,
                            "480",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "vsync-enabled",
                            "VSync Enabled",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "fullscreen-enabled",
                            "Fullscreen Enabled",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            [])
                    ])
            ],
            [
                new PlatformAssetRequirementDefinition(
                    "scene",
                    "Scene",
                    true,
                    ["helen"]),
                new PlatformAssetRequirementDefinition(
                    "texture",
                    "Texture",
                    true,
                    ["png", "tga", "jpg"]),
                new PlatformAssetRequirementDefinition(
                    "font",
                    "Font",
                    false,
                    ["font.asset", "ttf", "otf"])
            ],
            [
                new PlatformMaterialSchemaDefinition(
                    GameCubeMaterialSchemaIds.StandardTexturedSchemaId,
                    "GameCube Standard Textured",
                    ["gx"],
                    [
                        new PlatformMaterialFieldDefinition(
                            GameCubeMaterialSchemaIds.TextureRelativePathFieldId,
                            "Texture",
                            PlatformMaterialFieldKind.Text,
                            string.Empty,
                            false,
                            []),
                        new PlatformMaterialFieldDefinition(
                            GameCubeMaterialSchemaIds.DoubleSidedFieldId,
                            "Double Sided",
                            PlatformMaterialFieldKind.Boolean,
                            "false",
                            true,
                            []),
                        new PlatformMaterialFieldDefinition(
                            GameCubeMaterialSchemaIds.VertexColorModeFieldId,
                            "Vertex Color",
                            PlatformMaterialFieldKind.Choice,
                            "multiply",
                            true,
                            ["multiply", "ignore"]),
                        new PlatformMaterialFieldDefinition(
                            GameCubeMaterialSchemaIds.BaseColorFieldId,
                            "Base Color",
                            PlatformMaterialFieldKind.Color,
                            "#FFFFFFFF",
                            true,
                            []),
                        new PlatformMaterialFieldDefinition(
                            GameCubeMaterialSchemaIds.LightingModeFieldId,
                            "Lighting",
                            PlatformMaterialFieldKind.Choice,
                            "lit",
                            true,
                            ["lit", "unlit"])
                    ])
            ],
            [
                new PlatformComponentSupportRule(
                    "helengine.meshcomponent",
                    PlatformComponentSupportKind.Transform,
                    "Mesh components are normalized during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.cameracomponent",
                    PlatformComponentSupportKind.Transform,
                    "Camera components are normalized during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.fpscomponent",
                    PlatformComponentSupportKind.PassThrough,
                    "FPS overlay payload is canonical across platforms.",
                    string.Empty)
            ],
            [
                new PlatformCodegenProfileDefinition(
                    "default",
                    "Default",
                    "GameCube C# to C++ codegen profile",
                    PlatformCodegenLanguage.Cpp,
                    PlatformSerializationEndianness.BigEndian,
                    [])
            ],
            [
                new PlatformStorageProfileDefinition(
                    "disc-layout",
                    "Disc Layout",
                    PlatformStorageProfileKind.DiscLayout,
                    "gamecube-disc-layout",
                    allowContainerSegmentation: true)
            ],
            [
                new PlatformMediaProfileDefinition(
                    "gamecube-install-tree",
                    "GameCube Install Tree",
                    PlatformMediaLayoutKind.InstallTree,
                    allowPhysicalDuplication: true,
                    preferLocalityOverDeduplication: true)
            ],
            new RuntimeGenerationContract(
                RuntimeMaterialResolutionMode.CookedPlatformOwned,
                true,
                PackagedPathPolicy.ContentRelativeOnly),
            assetCookCapabilities: [
                new PlatformAssetCookCapabilityDefinition(
                    "texture",
                    "runtime-texture",
                    PlatformAssetCookOwnershipKind.BuilderOwned,
                    "gamecube-texture",
                    CreateDefaultSerializedTextureCookSettings(),
                    CreateTextureFormatCapabilities()),
                new PlatformAssetCookCapabilityDefinition(
                    "font-atlas-texture",
                    "runtime-font-atlas-texture",
                    PlatformAssetCookOwnershipKind.BuilderOwned,
                    "gamecube-texture",
                    CreateDefaultSerializedFontAtlasTextureCookSettings(),
                    CreateTextureFormatCapabilities())
            ]);
    }
}
