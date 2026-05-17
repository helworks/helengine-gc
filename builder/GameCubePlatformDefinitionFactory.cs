using helengine.baseplatform.Definitions;
using helengine.baseplatform.Profiles;

namespace helengine.gamecube.builder;

/// <summary>
/// Creates the typed GameCube builder metadata consumed by the editor.
/// </summary>
public static class GameCubePlatformDefinitionFactory {
    /// <summary>
    /// Serialized default texture cook settings used when GameCube-owned assets do not author an explicit platform override yet.
    /// </summary>
    const string DefaultGameCubeTextureSettings = "{\"maxResolution\":0,\"colorFormat\":\"GxRgb5A3\",\"alphaPrecision\":\"A8\"}";

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
                    DefaultGameCubeTextureSettings),
                new PlatformAssetCookCapabilityDefinition(
                    "font-atlas-texture",
                    "runtime-font-atlas-texture",
                    PlatformAssetCookOwnershipKind.BuilderOwned,
                    "gamecube-texture",
                    DefaultGameCubeTextureSettings)
            ]);
    }
}
