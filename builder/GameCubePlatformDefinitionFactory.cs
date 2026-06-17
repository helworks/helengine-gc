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
    /// Generic native numeric type remaps required by C++ platforms that cannot emit System.Numerics runtime types directly.
    /// </summary>
    const string NativeNumericTypeRemaps = "System.Numerics.Vector2=helengine.float2;System.Numerics.Vector3=helengine.float3;System.Numerics.Vector4=helengine.float4;System.Numerics.Quaternion=helengine.float4";

    /// <summary>
    /// Generic generated-math-convention value that instructs the shared C++ generator to emit native column-vector math helpers.
    /// </summary>
    const string NativeColumnVectorMathConvention = "native-column-vector";

    /// <summary>
    /// Generic pointer-size contract forwarded to the shared C++ generator for GameCube-native output.
    /// </summary>
    const string GameCubePointerSizeInBytes = "4";

    /// <summary>
    /// Generic preprocessor symbol that forces generated-core regeneration to include the GameCube 3D physics runtime even when the selected build scenes do not surface physics feature symbols directly.
    /// </summary>
    const string GameCubeIncludePhysics3DRuntimeSymbol = "HELENGINE_GAMECUBE_INCLUDE_PHYSICS3D_RUNTIME";

    /// <summary>
    /// Generic codegen hook that identifies the builder-owned file-system adapter header for disc-backed GameCube runtime paths.
    /// </summary>
    const string NativeFileSystemHeader = "\"platform/gamecube/GameCubeDiscFileSystem.hpp\"";

    /// <summary>
    /// Generic codegen hook that identifies the builder-owned file-system adapter type for disc-backed GameCube runtime paths.
    /// </summary>
    const string NativeFileSystemType = "helengine::gamecube::GameCubeDiscFileSystem";

    /// <summary>
    /// Creates the serialized default GameCube texture settings contract used when assets do not provide an explicit GameCube override.
    /// </summary>
    /// <returns>Serialized default GameCube texture settings.</returns>
    static string CreateDefaultSerializedTextureCookSettings() {
        return SerializeTextureCookSettings(new TextureAssetProcessorSettings {
            MaxResolution = 0,
            ColorFormatId = "GxRgb5A3",
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
            ColorFormatId = "GxRgb5A3",
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
            ["colorFormat"] = processorSettings.ColorFormatId,
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
                "GxRgb5A3"
            ],
            [
                TextureAssetAlphaPrecision.A8
            ],
            [
                new PlatformTextureFormatCombinationDefinition("GxRgb5A3", TextureAssetAlphaPrecision.A8)
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
                    [
                        new PlatformSettingDefinition(
                            "write-conversion-report",
                            "Write Conversion Report",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "include-project-defined-preprocessor-symbols",
                            "Include Project Symbols",
                            PlatformSettingKind.Boolean,
                            "false",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "additional-preprocessor-symbols",
                            "Additional Preprocessor Symbols",
                            PlatformSettingKind.Text,
                            GameCubeIncludePhysics3DRuntimeSymbol,
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "load-native-runtime-metadata",
                            "Load Native Runtime Metadata",
                            PlatformSettingKind.Boolean,
                            "true",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "generated-math-convention",
                            "Generated Math Convention",
                            PlatformSettingKind.Text,
                            NativeColumnVectorMathConvention,
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "pointer-size-bytes",
                            "Pointer Size (Bytes)",
                            PlatformSettingKind.Text,
                            GameCubePointerSizeInBytes,
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "type-remaps",
                            "Type Remaps",
                            PlatformSettingKind.Text,
                            NativeNumericTypeRemaps,
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "native-file-system-header",
                            "Native File System Header",
                            PlatformSettingKind.Text,
                            NativeFileSystemHeader,
                            true,
                            []),
                        new PlatformSettingDefinition(
                            "native-file-system-type",
                            "Native File System Type",
                            PlatformSettingKind.Text,
                            NativeFileSystemType,
                            true,
                            [])
                    ])
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
